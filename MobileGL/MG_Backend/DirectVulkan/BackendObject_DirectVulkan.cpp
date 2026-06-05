// MobileGL - MobileGL/MG_Backend/DirectVulkan/BackendObject_DirectVulkan.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "BackendObject_DirectVulkan.h"
#include "MG_Backend/BackendObject.h"
#include "DirectVulkan.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    namespace {
        Bool IsReleaseCurrentRequest(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
            return dpy == EGL_NO_DISPLAY && draw == EGL_NO_SURFACE && read == EGL_NO_SURFACE && ctx == EGL_NO_CONTEXT;
        }
    } // namespace

    BackendObject_DirectVulkan::~BackendObject_DirectVulkan() = default;

    Bool BackendObject_DirectVulkan::InitWindowSurface() {
        if (!m_windowHandle.Handle) {
            MGLOG_E("Cannot initialize DirectVulkan window surface: native window handle is null");
            return false;
        }

        auto nativeWindow = reinterpret_cast<NativeWindowType>(m_windowHandle.Handle);

        pVulkanRenderer = MakeUnique<MG_Backend::DirectVulkan::VulkanRenderer>(nativeWindow);
        MOBILEGL_ASSERT(pVulkanRenderer != nullptr, "InitWindowSurface: VulkanRenderer creation failed");
        pVulkanRenderer->Initialize();
        return true;
    }

    void BackendObject_DirectVulkan::Initialize() {
        m_initialized = true;
    }

    Bool BackendObject_DirectVulkan::InitCapabilities() {
        if (!m_initialized) {
            MGLOG_E("Cannot initialize capabilities before backend is initialized");
            return false;
        }
        if (!pVulkanRenderer) {
            MGLOG_E("Cannot initialize capabilities: Vulkan renderer has not been created");
            return false;
        }

        MG_Util::BackendLoader::FillInVulkanCapabilities(m_vulkanCaps, pVulkanRenderer->GetPhysicalDevice().properties);
        UpdateDynamicBackendParameters();
        return true;
    }

    Bool BackendObject_DirectVulkan::InitializeEGLDisplay(EGLDisplay dpy, EGLint* major, EGLint* minor) {
        if (!m_initialized) {
            MGLOG_E("DirectVulkan backend not initialized");
            return false;
        }
        return BackendObject::InitializeEGLDisplay(dpy, major, minor);
    }

    Bool BackendObject_DirectVulkan::CreateEGLWindowSurface(const WindowHandle& handle) {
        const std::lock_guard<std::recursive_mutex> lock(m_eglStateMutex);
        if (!m_initialized) {
            MGLOG_E("DirectVulkan backend not initialized");
            return false;
        }
        if (handle.Backend != WindowBackend::Android || !handle.Handle) {
            MGLOG_E("DirectVulkan backend only supports Android native windows");
            return false;
        }

        const Bool sameHandle =
            m_eglWindowSurfaceInitialized && m_windowHandle.Backend == handle.Backend && m_windowHandle.Handle == handle.Handle;
        if (sameHandle) {
            return true;
        }

        if (m_eglWindowSurfaceInitialized || pVulkanRenderer) {
            pVulkanRenderer.reset();
            ResetEGLRuntimeState();
        }

        return BackendObject::CreateEGLWindowSurface(handle);
    }

    Bool BackendObject_DirectVulkan::MakeEGLCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
        const std::lock_guard<std::recursive_mutex> lock(m_eglStateMutex);
        if (IsReleaseCurrentRequest(dpy, draw, read, ctx)) {
            return BackendObject::MakeEGLCurrent(dpy, draw, read, ctx);
        }
        if (!pVulkanRenderer) {
            MGLOG_E("DirectVulkan renderer is not initialized");
            return false;
        }
        return BackendObject::MakeEGLCurrent(dpy, draw, read, ctx);
    }

    Bool BackendObject_DirectVulkan::SwapEGLBuffers(EGLDisplay dpy, EGLSurface draw) {
        const std::lock_guard<std::recursive_mutex> lock(m_eglStateMutex);
        if (!pVulkanRenderer) {
            MGLOG_E("DirectVulkan renderer is not initialized");
            return false;
        }
        return BackendObject::SwapEGLBuffers(dpy, draw);
    }

    const RendererInfo& BackendObject_DirectVulkan::GetRendererInfo() const {
        static RendererInfo RendererInfo = {
            .RendererName = "Magma",          // Renderer Name
            .BackendName = "Direct (Vulkan)", // Backend Name
            .ExtraVendor = Nullopt,           // Extra vendor
            .RendererGLInfo =
                {
                    .TargetGLVersion = {3, 3, 0},                      // Target OpenGL Version
                    .TargetGLSLVersion = {4, 6, 0},                    // Target Shading Language Version
                    .Extensions = {V_OpenGL30, V_OpenGL31, V_OpenGL32, // OpenGL Extensions
                                   V_OpenGL33, E_GL_ARB_draw_buffers_blend,
                                   E_GL_ARB_compute_shader,
                                   E_GL_ARB_shader_storage_buffer_object,
                                   E_GL_ARB_shader_image_load_store,
                                   E_GL_ARB_program_interface_query},
                    .IsCompatibilityProfile = false // Is Compatibility Profile
                },
            .StaticBackendCapability = {.AllowVSOnlyPrograms = false} // Backend Capability
        };
        return RendererInfo;
    }

    String BackendObject_DirectVulkan::GetBackendAPIVersionString() const {
        if (!m_initialized) {
            return "<uninitialized DirectVulkan backend>";
        }
        // Format:
        // <GPU Name>, Vulkan <Vulkan Version>, Driver <Driver Version>
        String str = m_vulkanCaps.DeviceName + ", Vulkan " + m_vulkanCaps.VulkanAPIVersion.toString() + ", Driver " +
                     m_vulkanCaps.DriverVersionString;
        return str;
    }

    BackendType BackendObject_DirectVulkan::GetBackendType() const {
        return BackendType::DirectVulkan;
    }

    const GlobalBackendFunctionsTable& BackendObject_DirectVulkan::GetBackendFunctions() const {
        static GlobalBackendFunctionsTable funcsTable;
        static Bool funcsTableInitialized = false;
        if (!funcsTableInitialized) {
            funcsTable.Present = Present;
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
            funcsTableInitialized = true;
        }
        return funcsTable;
    }

    const DynamicBackendParameters& BackendObject_DirectVulkan::GetDynamicParameters() const {
        return m_dynamicParameters;
    }

    void BackendObject_DirectVulkan::UpdateDynamicBackendParameters() {
        m_dynamicParameters.UniformBufferOffsetAlignment = m_vulkanCaps.UniformBufferOffsetAlignment;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
