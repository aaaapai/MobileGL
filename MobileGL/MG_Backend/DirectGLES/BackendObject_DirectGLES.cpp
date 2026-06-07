// MobileGL - MobileGL/MG_Backend/DirectGLES/BackendObject_DirectGLES.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "BackendObject_DirectGLES.h"
#include "MG_Backend/BackendObject.h"
#include <MG_Backend/DirectGLES/DirectGLES.h>
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>
#include <format>

namespace MobileGL::MG_Backend::DirectGLES {
    namespace {
        Bool IsReleaseCurrentRequest(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
            return dpy == EGL_NO_DISPLAY && draw == EGL_NO_SURFACE && read == EGL_NO_SURFACE && ctx == EGL_NO_CONTEXT;
        }
    } // namespace

    BackendObject_DirectGLES::~BackendObject_DirectGLES() {
        DestroyEGLContext();
    }

    Bool BackendObject_DirectGLES::InitWindowSurface() {
        // Only use EGL for now
        auto nativeWindow = reinterpret_cast<NativeWindowType>(m_windowHandle.Handle);
        if (!DirectGLES::InitWindowSurface(nativeWindow)) {
            MGLOG_E("Failed to initialize window surface for DirectGLES backend");
            return false;
        }
        return true;
    }

    void BackendObject_DirectGLES::Initialize() {
        if (!MG_Util::BackendLoader::AcquireEGLFunctions(m_EGLFunctions)) {
            MGLOG_E("Failed to acquire EGL functions for DirectGLES backend");
            return;
        }
        if (!MG_Util::BackendLoader::AcquireGLESFunctions(m_GLESFunctions, m_EGLFunctions.eglGetProcAddress)) {
            MGLOG_E("Failed to acquire GLES functions for DirectGLES backend");
            return;
        }
        m_initialized = true;

        DirectGLES::SetEGLFuncsTable(m_EGLFunctions);
        DirectGLES::SetGLESFuncsTable(m_GLESFunctions);
    }

    Bool BackendObject_DirectGLES::InitCapabilities() {
        if (!m_initialized) {
            MGLOG_E("DirectGLES backend not initialized");
            return false;
        }

        if (!MG_Util::BackendLoader::FillInGLESCapabilities(m_GLESCapabilities, m_GLESFunctions)) {
            MGLOG_E("Failed to fill in GLES capabilities for DirectGLES backend");
            return false;
        }
        DirectGLES::SetGLESCapabilities(m_GLESCapabilities);
        UpdateDynamicBackendParameters();
        return true;
    }

    Bool BackendObject_DirectGLES::InitializeEGLDisplay(EGLDisplay dpy, EGLint* major, EGLint* minor) {
        if (!m_initialized) {
            MGLOG_E("DirectGLES backend not initialized");
            return false;
        }
        return BackendObject::InitializeEGLDisplay(dpy, major, minor);
    }

    Bool BackendObject_DirectGLES::CreateEGLWindowSurface(const WindowHandle& handle) {
        const std::lock_guard<std::recursive_mutex> lock(m_eglStateMutex);
        if (!m_initialized) {
            MGLOG_E("DirectGLES backend not initialized");
            return false;
        }

        if ((handle.Backend != WindowBackend::Android && handle.Backend != WindowBackend::X11) || !handle.Handle) {
            MGLOG_E("DirectGLES backend only supports Android and X11 native windows");
            return false;
        }

        const Bool sameHandle = m_eglSurfaceInitialized && m_eglSurfaceKind == SurfaceKind::Window &&
                                m_windowHandle.Backend == handle.Backend && m_windowHandle.Handle == handle.Handle;
        if (sameHandle) {
            return true;
        }

        if (m_eglSurfaceInitialized) {
            DestroyEGLContext();
            ResetEGLRuntimeState();
        }

        return BackendObject::CreateEGLWindowSurface(handle);
    }

    Bool BackendObject_DirectGLES::InitPbufferSurface(EGLint width, EGLint height) {
        return DirectGLES::InitPbufferSurface(width, height);
    }

    Bool BackendObject_DirectGLES::MakeEGLCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
        if (IsReleaseCurrentRequest(dpy, draw, read, ctx)) {
            return BackendObject::MakeEGLCurrent(dpy, draw, read, ctx);
        }

        return BackendObject::MakeEGLCurrent(dpy, draw, read, ctx);
    }

    Bool BackendObject_DirectGLES::SwapEGLBuffers(EGLDisplay dpy, EGLSurface draw) {
        return BackendObject::SwapEGLBuffers(dpy, draw);
    }

    const RendererInfo& BackendObject_DirectGLES::GetRendererInfo() const {
        static RendererInfo RendererInfo = {
            .RendererName = "Espryt",            // Renderer Name
            .BackendName = "Direct (OpenGL ES)", // Backend Name
            .ExtraVendor = Nullopt,              // Extra vendor
            .RendererGLInfo =
                {
                    .TargetGLVersion = {3, 3, 0},                      // Target OpenGL Version
                    .TargetGLSLVersion = {4, 6, 0},                    // Target Shading Language Version
                    .Extensions = {V_OpenGL30, V_OpenGL31, V_OpenGL32, // OpenGL Extensions
                                   V_OpenGL33, E_GL_ARB_draw_buffers_blend, E_GL_ARB_compute_shader,
                                   E_GL_ARB_shader_storage_buffer_object, E_GL_ARB_shader_image_load_store,
                                   E_GL_ARB_program_interface_query, E_GL_ARB_framebuffer_object,
                                   E_GL_EXT_framebuffer_object, E_GL_ARB_depth_texture},
                    .IsCompatibilityProfile = false // Is Compatibility Profile
                },
            .StaticBackendCapability = {.AllowVSOnlyPrograms = false} // Backend Capability
        };
        return RendererInfo;
    }

    String BackendObject_DirectGLES::GetBackendAPIVersionString() const {
        if (!m_initialized) {
            return "<uninitialized DirectGLES backend>";
        }
        // Format:
        // <OpenGL ES Renderer>, OpenGL ES <OpenGL ES Version>
        String versionString = std::format("{}, OpenGL ES {}.{}", m_GLESCapabilities.GLESRendererString,
                                           m_GLESCapabilities.GLESVersion.Major, m_GLESCapabilities.GLESVersion.Minor);
        return versionString;
    }

    BackendType BackendObject_DirectGLES::GetBackendType() const {
        return BackendType::DirectGLES;
    }

    const GlobalBackendFunctionsTable& BackendObject_DirectGLES::GetBackendFunctions() const {
        static GlobalBackendFunctionsTable funcsTable;
        static Bool funcsTableInitialized = false;
        if (!funcsTableInitialized) {
            funcsTable.Present = DirectGLES::Present;
            funcsTable.GL.DrawArrays = DrawArrays;
            funcsTable.GL.DrawElements = DrawElements;
            funcsTable.GL.DrawElementsBaseVertex = DrawElementsBaseVertex;
            funcsTable.GL.MultiDrawElements = MultiDrawElements;
            funcsTable.GL.MultiDrawElementsBaseVertex = MultiDrawElementsBaseVertex;
            funcsTable.GL.MultiDrawElementsIndirect = MultiDrawElementsIndirect;
            funcsTable.GL.MultiDrawArraysIndirect = MultiDrawArraysIndirect;
            funcsTable.GL.DrawRangeElementsBaseVertex = DrawRangeElementsBaseVertex;
            funcsTable.GL.DrawRangeElements = DrawRangeElements;
            funcsTable.GL.DrawElementsInstancedBaseVertexBaseInstance = DrawElementsInstancedBaseVertexBaseInstance;
            funcsTable.GL.DrawElementsInstancedBaseVertex = DrawElementsInstancedBaseVertex;
            funcsTable.GL.DrawElementsInstancedBaseInstance = DrawElementsInstancedBaseInstance;
            funcsTable.GL.DrawElementsInstanced = DrawElementsInstanced;
            funcsTable.GL.DrawArraysInstancedBaseInstance = DrawArraysInstancedBaseInstance;
            funcsTable.GL.DrawArraysInstanced = DrawArraysInstanced;
            funcsTable.GL.DrawElementsIndirect = DrawElementsIndirect;
            funcsTable.GL.DrawArraysIndirect = DrawArraysIndirect;
            funcsTable.GL.DispatchCompute = DispatchCompute;
            funcsTable.GL.DispatchComputeIndirect = DispatchComputeIndirect;
            funcsTable.GL.MemoryBarrier = MemoryBarrier;
            funcsTable.GL.MemoryBarrierByRegion = MemoryBarrierByRegion;
            funcsTable.GL.BindImageTexture = BindImageTexture;
            funcsTable.GL.GetIntegeri_v = GetIntegeri_v;
            funcsTable.GL.GetInteger64i_v = GetInteger64i_v;
            funcsTable.GL.GetProgramiv = GetProgramiv;
            funcsTable.GL.GetProgramInterfaceiv = GetProgramInterfaceiv;
            funcsTable.GL.GetProgramResourceIndex = GetProgramResourceIndex;
            funcsTable.GL.GetProgramResourceName = GetProgramResourceName;
            funcsTable.GL.GetProgramResourceiv = GetProgramResourceiv;
            funcsTable.GL.GetProgramResourceLocation = GetProgramResourceLocation;
            funcsTable.GL.GetProgramResourceLocationIndex = GetProgramResourceLocationIndex;
            funcsTable.GL.ShaderStorageBlockBinding = ShaderStorageBlockBinding;
            funcsTable.GL.Clear = Clear;
            funcsTable.GL.ClearBufferfi = ClearBufferfi;
            funcsTable.GL.ClearBufferfv = ClearBufferfv;
            funcsTable.GL.ClearBufferuiv = ClearBufferuiv;
            funcsTable.GL.ClearBufferiv = ClearBufferiv;
            funcsTable.GL.BlitFramebuffer = BlitFramebuffer;
            funcsTable.GL.CopyTexImage2D = CopyTexImage2D;
            funcsTable.GL.CopyTexSubImage2D = CopyTexSubImage2D;
            funcsTable.GL.GenerateMipmap = GenerateMipmap;
            funcsTable.GL.ReadPixels = ReadPixels;
            funcsTable.GL.GetTexImage = GetTexImage;
            funcsTableInitialized = true;
        }
        return funcsTable;
    }

    const DynamicBackendParameters& BackendObject_DirectGLES::GetDynamicParameters() const {
        return m_dynamicParameters;
    }

    void BackendObject_DirectGLES::UpdateDynamicBackendParameters() {
        m_dynamicParameters.UniformBufferOffsetAlignment = m_GLESCapabilities.UniformBufferOffsetAlignment;
    }

    const MG_External::GLESFunctionsTable& BackendObject_DirectGLES::GetGLESFunctions() const {
        return m_GLESFunctions;
    }

    const MG_External::EGLFunctionsTable& BackendObject_DirectGLES::GetEGLFunctions() const {
        return m_EGLFunctions;
    }
} // namespace MobileGL::MG_Backend::DirectGLES
