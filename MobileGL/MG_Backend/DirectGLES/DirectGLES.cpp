// MobileGL - MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "DirectGLES.h"
#include "EGL/egl.h"
#include "MG_Util/Types.h"
#include "Utils.h"
#include "Managers.h"
#include <MG_Util/Converters/GLToMG/TextureEnumConverter.h>
#include <MG_Util/Classifiers/TextureEnumClassifier.h>
#include <MG_Util/Metrics/TextureMetrics.h>
#include <MG_State/GLState/Core.h>
#include <MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.h>
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>
#include <MG_Util/Converters/GLToStr/GLEnumConverter.h>
#include <MG_Util/Converters/MGToGL/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToStr/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToGL/RenderStateEnumConverter.h>
#include <MG_Util/Metrics/BufferMetrics.h>
#include <MG_Util/Texture/PixelStoreProcessor.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#if defined(__linux__) && !defined(__ANDROID__) && __has_include(<X11/Xlib.h>)
#pragma push_macro("Bool")
#pragma push_macro("None")
#include <X11/Xlib.h>
#pragma pop_macro("None")
#pragma pop_macro("Bool")
#endif

namespace MobileGL::MG_Backend::DirectGLES {
    MG_External::EGLFunctionsTable g_EGLFuncs;
    MG_External::GLESFunctionsTable g_GLESFuncs;
    MG_External::GLESCapabilities g_GLESCapabilities;

    static Bool QueryCurrentSurfaceSize(Int& outWidth, Int& outHeight);

    enum class DrawSyncBit : Uint32 {
        None = 0,
        IndexBuffer = 1 << 0,
        IndirectBuffer = 1 << 1,
        Instancing = 1 << 2
    };

    inline DrawSyncBit operator|(DrawSyncBit a, DrawSyncBit b) {
        return static_cast<DrawSyncBit>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    inline DrawSyncBit& operator|=(DrawSyncBit& a, DrawSyncBit b) {
        a = a | b;
        return a;
    }

    struct DrawElementsIndirectCommand {
        Uint32 count = 0;
        Uint32 instanceCount = 0;
        Uint32 firstIndex = 0;
        Int32 baseVertex = 0;
        Uint32 baseInstance = 0;
    };

    namespace DebugImpl {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG
        void ErrorLopper::Loop(const std::function<void(GLenum)>& func) {
            GLenum err = g_GLESFuncs.glGetError();
            while (err != GL_NO_ERROR) {
                func(err);
                err = g_GLESFuncs.glGetError();
            }
        }

        void ErrorLopper::Clear() {
            GLenum err = g_GLESFuncs.glGetError();
            while (err != GL_NO_ERROR) {
                MGLOG_D("Stray GL Error cleared: %s", MG_Util::ConvertGLEnumToString(err).c_str());
                err = g_GLESFuncs.glGetError();
            }
        }

        ErrorLopper::ErrorLopper() {
            Clear();
        }
        ErrorLopper::~ErrorLopper() {
            Clear();
        }
#else
        void ErrorLopper::Loop(const std::function<void(GLenum)>& func) {}
        void ErrorLopper::Clear() {}
        ErrorLopper::ErrorLopper() = default;
        ErrorLopper::~ErrorLopper() = default;
#endif

#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG
        OpenGLScopeMarker::OpenGLScopeMarker(const String& scopeName) {
            g_GLESFuncs.glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, scopeName.c_str());
        }

        OpenGLScopeMarker::~OpenGLScopeMarker() {
            g_GLESFuncs.glPopDebugGroup();
        }
#else
        OpenGLScopeMarker::OpenGLScopeMarker(const String& scopeName) {}

        OpenGLScopeMarker::~OpenGLScopeMarker() {}
#endif
    } // namespace DebugImpl

    // TODO: deletion for deleted objects

    namespace BufferImpl {
        void CreateAndSyncBufferObject(const SharedPtr<MG_State::GLState::BufferObject>& bufferObject) {
            bufferObject->MarkPersistentMappedRangeDirty();
            if (!(bufferObject->GetChangeBits() & BufferChangeBits::DirtyBit)) return;

            const auto& backendBufferIt = g_backendBufferObjects.find(bufferObject.get());
            Bool exist = (backendBufferIt != g_backendBufferObjects.end());
            auto& backendObj = exist ? backendBufferIt->second : g_backendBufferObjects.GetOrCreate(bufferObject);
            if (!exist) {
                backendObj = MakeShared<BackendBufferObject>();
            }
            backendObj->SyncToBackend(bufferObject);
        }

        void SyncBufferBindingPoints(BufferTarget target, GLenum glTarget) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            auto bindingPointCnt = MG_State::pGLContext->GetBufferBindingPointCount(target);
            for (SizeT i = 0; i < bindingPointCnt; ++i) {
                auto& point = MG_State::pGLContext->GetBufferBindingPoint(target, i);
                auto& obj = point.GetBoundObject();
                if (!obj) {
                    g_GLESFuncs.glBindBufferBase(glTarget, static_cast<GLuint>(i), 0);
                    continue;
                }

                CreateAndSyncBufferObject(obj);
                const auto& backendBufferIt = g_backendBufferObjects.find(obj.get());
                if (backendBufferIt == g_backendBufferObjects.end()) {
                    MGLOG_E("No backend buffer found for %s binding point %zu.",
                            MG_Util::ConvertGLEnumToString(glTarget).c_str(), i);
                    continue;
                }

                const auto& range = point.GetRange();
                auto backendBufferId = backendBufferIt->second->GetBackendBufferId();
                if (range.start == 0 && range.end >= obj->GetSize()) {
                    g_GLESFuncs.glBindBufferBase(glTarget, static_cast<GLuint>(i), backendBufferId);
                } else {
                    const auto start = std::min(range.start, obj->GetSize());
                    const auto end = std::min(range.end, obj->GetSize());
                    g_GLESFuncs.glBindBufferRange(glTarget, static_cast<GLuint>(i), backendBufferId,
                                                  static_cast<GLintptr>(start), static_cast<GLsizeiptr>(end - start));
                }
            }
        }

        void SyncBoundBuffer(BufferTarget target, GLenum glTarget) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            auto& bufferObject = MG_State::pGLContext->GetBufferBindingSlot(target).GetBoundObject();
            if (!bufferObject) {
                g_GLESFuncs.glBindBuffer(glTarget, 0);
                return;
            }

            CreateAndSyncBufferObject(bufferObject);
            const auto& backendBufferIt = g_backendBufferObjects.find(bufferObject.get());
            if (backendBufferIt == g_backendBufferObjects.end()) {
                MGLOG_E("No backend buffer found for %s.", MG_Util::ConvertGLEnumToString(glTarget).c_str());
                return;
            }
            backendBufferIt->second->Bind(glTarget);
        }

        void SyncNeccessaryBuffers(Bool includeIBO = false, Bool includeIndirectBuffer = false) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            g_backendBufferObjects.CollectGarbageIfNeeded();

            // All buffers we need are:
            //   1.VBO 2.IBO (if needed) 3.UBO 4.IndirectBuffer (if needed)
            // PBO is not needed since it should be handled in frontend

            const auto& currentVAOObject = MG_State::pGLContext->GetBoundVertexArray();
            if (!currentVAOObject) {
                MGLOG_E("No VAO is currently bound, cannot sync necessary buffers.");
                return;
            }

            // VBO
            for (const auto& attrib : currentVAOObject->GetAllAttributes()) {
                if (!attrib.Enabled) continue;
                auto& bufferObject = attrib.Buffer;
                if (bufferObject) {
                    CreateAndSyncBufferObject(bufferObject);
                }
            }

            // IBO
            if (includeIBO) {
                auto& possibleIBO = currentVAOObject->GetIndexBufferBindingSlot().GetBoundObject();
                if (possibleIBO) {
                    CreateAndSyncBufferObject(possibleIBO);
                }
            }

            // Indirect Buffer Object
            if (includeIndirectBuffer) {
                auto& possibleIndirectBuffer =
                    MG_State::pGLContext->GetBufferBindingSlot(BufferTarget::DrawIndirect).GetBoundObject();
                if (possibleIndirectBuffer) {
                    CreateAndSyncBufferObject(possibleIndirectBuffer);
                }
            }

            SyncBufferBindingPoints(BufferTarget::Uniform, GL_UNIFORM_BUFFER);
        }

        void SyncComputeBuffers(Bool includeDispatchIndirectBuffer) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            g_backendBufferObjects.CollectGarbageIfNeeded();
            SyncBufferBindingPoints(BufferTarget::Uniform, GL_UNIFORM_BUFFER);
            SyncBufferBindingPoints(BufferTarget::ShaderStorage, GL_SHADER_STORAGE_BUFFER);
            if (includeDispatchIndirectBuffer) {
                SyncBoundBuffer(BufferTarget::DispatchIndirect, GL_DISPATCH_INDIRECT_BUFFER);
            }
        }
    } // namespace BufferImpl

    namespace VertexArrayImpl {
        void SyncCurrentVAO() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            g_backendVertexArrayObjects.CollectGarbageIfNeeded();

            auto& currentVAOObject = MG_State::pGLContext->GetBoundVertexArray();
            if (!currentVAOObject) {
                MGLOG_E("No VAO is currently bound, cannot sync current VAO.");
                return;
            }

            const auto& backendVAOIt = g_backendVertexArrayObjects.find(currentVAOObject.get());
            Bool exist = (backendVAOIt != g_backendVertexArrayObjects.end());
            auto& backendObj = exist ? backendVAOIt->second : g_backendVertexArrayObjects.GetOrCreate(currentVAOObject);
            if (!exist) {
                backendObj = MakeShared<VertexArrayImpl::BackendVertexArrayObject>();
            }
            backendObj->SyncToBackend(currentVAOObject);
        }
    } // namespace VertexArrayImpl

    namespace TextureImpl {
        SharedPtr<BackendTextureObject>& SyncTextureObjectToBackend(
            const SharedPtr<MG_State::GLState::ITextureObject>& textureObject) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            const auto& backendTextureIt = g_backendTextureObjects.find(textureObject.get());
            Bool exist = (backendTextureIt != g_backendTextureObjects.end());
            auto& backendObj = exist ? backendTextureIt->second : g_backendTextureObjects.GetOrCreate(textureObject);
            if (!exist) {
                backendObj = MakeShared<BackendTextureObject>();
            }
            backendObj->SyncTextureParamsToBackend(textureObject);
            backendObj->SyncBuiltinSamplerToBackend(textureObject);
            backendObj->SyncMipmapsToBackend(textureObject);

            return backendObj;
        }

        void SyncNeccessaryTextures() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            g_backendTextureObjects.CollectGarbageIfNeeded();

            // All textures we need are:
            //   1. textures bound to texture units (TODO: only sync ones that are used in current program)
            //   2. textures used in current FBO
            //   3. textures bound to image units (TODO)

            for (int index = 0; index < MG_State::GLState::TextureState::MAX_TEXTURE_IMAGE_UNITS; ++index) {
                auto& unit = MG_State::pGLContext->GetTextureUnitObject(index);
                for (const auto& bindingSlot : unit.GetAllBindingSlots()) {
                    auto& textureObject = bindingSlot.GetBoundObject();
                    if (textureObject) {
                        SyncTextureObjectToBackend(textureObject);
                    }
                }
            }

            const auto& currentFBO =
                MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw).GetBoundObject();
            if (currentFBO) {
                for (const auto& attachment : currentFBO->GetAllAttachmentObjects()) {
                    if (!attachment.IsTexture()) continue;
                    auto& textureObject = attachment.GetTexture();
                    if (textureObject) {
                        SyncTextureObjectToBackend(textureObject);
                    }
                }
            }
        }

        void SyncImageTextureBinding(Uint unit) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            auto& imageBinding = MG_State::pGLContext->GetImageTextureBinding(static_cast<Int>(unit));
            if (!imageBinding.Texture) {
                g_GLESFuncs.glBindImageTexture(unit, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
                return;
            }

            auto& backendTexture = SyncTextureObjectToBackend(imageBinding.Texture);
            g_GLESFuncs.glBindImageTexture(unit, backendTexture->GetBackendTextureId(), imageBinding.Level,
                                           imageBinding.Layered, imageBinding.Layer, imageBinding.Access,
                                           imageBinding.Format);
        }

        void SyncImageTextureBindings() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            for (Uint unit = 0; unit < MG_State::GLState::TextureState::MAX_TEXTURE_IMAGE_UNITS; ++unit) {
                SyncImageTextureBinding(unit);
            }
        }
    } // namespace TextureImpl

    namespace FramebufferImpl {
        void SyncCurrentFBO() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            g_backendFramebufferObjects.CollectGarbageIfNeeded();
            TextureImpl::g_backendTextureObjects.CollectGarbageIfNeeded();
            RenderbufferImpl::g_backendRenderbufferObjects.CollectGarbageIfNeeded();

            const FramebufferTarget fboTargets[] = {FramebufferTarget::Draw, FramebufferTarget::Read};

            MG_State::GLState::FramebufferObject* lastUpdatedFBO = nullptr;

            for (auto& target : fboTargets) {
                auto& slot = MG_State::pGLContext->GetFramebufferBindingSlot(target);
                auto version = slot.GetVersion();
                if (version == g_fboBindVersions[SizeT(target)]) continue;

                auto& currentFBO = slot.GetBoundObject();

                if (!currentFBO) {
                    MGLOG_E("No FBO is currently bound, cannot sync current FBO.");
                    continue;
                }

                if (currentFBO == MG_Impl::GLImpl::FramebufferImpl::pDefaultFramebufferInfo->defaultFBO) {
                    // Default FBO, nothing to sync
                    continue;
                }

                if (currentFBO.get() == lastUpdatedFBO) {
                    MGLOG_D("Draw FBO and read FBO are the same, skipping sync.");
                    continue;
                }

                const auto& backendFBOIt = g_backendFramebufferObjects.find(currentFBO.get());
                Bool exist = (backendFBOIt != g_backendFramebufferObjects.end());
                auto& backendObj = exist ? backendFBOIt->second : g_backendFramebufferObjects.GetOrCreate(currentFBO);
                if (!exist) {
                    backendObj = MakeShared<BackendFramebufferObject>();
                }
                backendObj->SyncToBackend(currentFBO, target);

                lastUpdatedFBO = currentFBO.get();
            }
        }
    } // namespace FramebufferImpl

    namespace RenderStateImpl {
        static Uint16 g_syncedRenderStateVersion = 0;
        static Bool g_hasSyncedRenderState = false;
        static RenderStateParameters g_syncedRenderStateParameters;
        static IntVec4 g_syncedBackendViewport = IntVec4(-1, -1, -1, -1);
        void SyncRenderState() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            Uint16 currentRenderStateVersion = MG_State::pGLContext->GetRenderStateParametersVersion();
            if (g_hasSyncedRenderState && currentRenderStateVersion == g_syncedRenderStateVersion) return;

            const auto& parameters = MG_State::pGLContext->GetRenderStateParameters();

            IntVec4 backendViewport = parameters.Viewport;
            if (backendViewport.z() <= 0 || backendViewport.w() <= 0) {
                Int surfaceWidth = 0;
                Int surfaceHeight = 0;
                if (QueryCurrentSurfaceSize(surfaceWidth, surfaceHeight)) {
                    backendViewport = IntVec4(0, 0, surfaceWidth, surfaceHeight);
                }
            }
            if (backendViewport != g_syncedBackendViewport) {
                g_GLESFuncs.glViewport(backendViewport.x(), backendViewport.y(), backendViewport.z(),
                                       backendViewport.w());
                g_syncedBackendViewport = backendViewport;
            }

#define SYNC_CAPABILITY(cap_mg, cap_gl)                                                                                \
    if (parameters.cap_mg##Enabled != g_syncedRenderStateParameters.cap_mg##Enabled) {                                 \
        if (parameters.cap_mg##Enabled) {                                                                              \
            g_GLESFuncs.glEnable(cap_gl);                                                                              \
        } else {                                                                                                       \
            g_GLESFuncs.glDisable(cap_gl);                                                                             \
        }                                                                                                              \
    }
            SYNC_CAPABILITY(DepthTest, GL_DEPTH_TEST);
            SYNC_CAPABILITY(ColorLogicOp, GL_COLOR_LOGIC_OP);
            SYNC_CAPABILITY(Dither, GL_DITHER);
            SYNC_CAPABILITY(Multisample, GL_MULTISAMPLE);
            SYNC_CAPABILITY(SampleAlphaToCoverage, GL_SAMPLE_ALPHA_TO_COVERAGE);
            SYNC_CAPABILITY(SampleCoverage, GL_SAMPLE_COVERAGE);
            SYNC_CAPABILITY(SampleMask, GL_SAMPLE_MASK);
            SYNC_CAPABILITY(PolygonOffsetFill, GL_POLYGON_OFFSET_FILL);
            SYNC_CAPABILITY(RasterizerDiscard, GL_RASTERIZER_DISCARD);
            SYNC_CAPABILITY(ScissorTest, GL_SCISSOR_TEST);
            SYNC_CAPABILITY(StencilTest, GL_STENCIL_TEST);
            SYNC_CAPABILITY(CullFace, GL_CULL_FACE);

#undef SYNC_CAPABILITY

            const auto& ToGLBoolean = [](Bool b) -> GLboolean { return b ? GL_TRUE : GL_FALSE; };

            { // Blend State
                using FBO = MG_State::GLState::FramebufferObject;
                const auto& targetStates = parameters.BlendStates;
                auto& syncedStates = g_syncedRenderStateParameters.BlendStates;

                Bool allEnabled = true;
                Bool allDisabled = true;
                Bool anyCapDirty = false;

                for (Uint i = 0; i < FBO::MAX_DRAW_BUFFERS; ++i) {
                    Bool enabled = targetStates[i].Enabled;
                    if (enabled)
                        allDisabled = false;
                    else
                        allEnabled = false;

                    if (enabled != syncedStates[i].Enabled) {
                        anyCapDirty = true;
                    }
                }

                if (anyCapDirty) {
                    if (allEnabled) {
                        g_GLESFuncs.glEnable(GL_BLEND);
                        for (auto& s : syncedStates)
                            s.Enabled = true;
                    } else if (allDisabled) {
                        g_GLESFuncs.glDisable(GL_BLEND);
                        for (auto& s : syncedStates)
                            s.Enabled = false;
                    } else {
                        for (Uint i = 0; i < FBO::MAX_DRAW_BUFFERS; ++i) {
                            if (targetStates[i].Enabled != syncedStates[i].Enabled) {
                                syncedStates[i].Enabled = targetStates[i].Enabled;
                                syncedStates[i].Enabled ? g_GLESFuncs.glEnablei(GL_BLEND, i)
                                                        : g_GLESFuncs.glDisablei(GL_BLEND, i);
                            }
                        }
                    }
                }

                Bool allFuncsSame = true;
                Bool anyFuncDirty = false;
                const auto& first = targetStates[0];

                for (Uint i = 0; i < FBO::MAX_DRAW_BUFFERS; ++i) {
                    const auto& cur = targetStates[i];
                    const auto& syn = syncedStates[i];

                    Bool isDiffFromSyn =
                        (cur.SrcFactorRGB != syn.SrcFactorRGB || cur.DstFactorRGB != syn.DstFactorRGB ||
                         cur.SrcFactorAlpha != syn.SrcFactorAlpha || cur.DstFactorAlpha != syn.DstFactorAlpha);

                    if (isDiffFromSyn) anyFuncDirty = true;

                    if (allFuncsSame && i > 0) {
                        if (cur.SrcFactorRGB != first.SrcFactorRGB || cur.DstFactorRGB != first.DstFactorRGB ||
                            cur.SrcFactorAlpha != first.SrcFactorAlpha || cur.DstFactorAlpha != first.DstFactorAlpha) {
                            allFuncsSame = false;
                        }
                    }
                }

                if (anyFuncDirty) {
                    if (allFuncsSame) {
                        g_GLESFuncs.glBlendFuncSeparate(MG_Util::ConvertBlendFactorToGLEnum(first.SrcFactorRGB),
                                                        MG_Util::ConvertBlendFactorToGLEnum(first.DstFactorRGB),
                                                        MG_Util::ConvertBlendFactorToGLEnum(first.SrcFactorAlpha),
                                                        MG_Util::ConvertBlendFactorToGLEnum(first.DstFactorAlpha));

                        for (auto& syn : syncedStates) {
                            syn.SrcFactorRGB = first.SrcFactorRGB;
                            syn.DstFactorRGB = first.DstFactorRGB;
                            syn.SrcFactorAlpha = first.SrcFactorAlpha;
                            syn.DstFactorAlpha = first.DstFactorAlpha;
                        }
                    } else {
                        for (Uint i = 0; i < FBO::MAX_DRAW_BUFFERS; ++i) {
                            const auto& cur = targetStates[i];
                            auto& syn = syncedStates[i];

                            if (cur.SrcFactorRGB != syn.SrcFactorRGB || cur.DstFactorRGB != syn.DstFactorRGB ||
                                cur.SrcFactorAlpha != syn.SrcFactorAlpha || cur.DstFactorAlpha != syn.DstFactorAlpha) {
                                syn.SrcFactorRGB = cur.SrcFactorRGB;
                                syn.DstFactorRGB = cur.DstFactorRGB;
                                syn.SrcFactorAlpha = cur.SrcFactorAlpha;
                                syn.DstFactorAlpha = cur.DstFactorAlpha;

                                g_GLESFuncs.glBlendFuncSeparatei(
                                    i, MG_Util::ConvertBlendFactorToGLEnum(cur.SrcFactorRGB),
                                    MG_Util::ConvertBlendFactorToGLEnum(cur.DstFactorRGB),
                                    MG_Util::ConvertBlendFactorToGLEnum(cur.SrcFactorAlpha),
                                    MG_Util::ConvertBlendFactorToGLEnum(cur.DstFactorAlpha));
                            }
                        }
                    }
                }
            }

            { // Depth state
                if (parameters.DepthFunc != g_syncedRenderStateParameters.DepthFunc) {
                    g_GLESFuncs.glDepthFunc(MG_Util::ConvertDepthTestFuncToGLEnum(parameters.DepthFunc));
                }
                if (parameters.DepthMask != g_syncedRenderStateParameters.DepthMask) {
                    g_GLESFuncs.glDepthMask(parameters.DepthMask ? GL_TRUE : GL_FALSE);
                }
                if (parameters.DepthRange != g_syncedRenderStateParameters.DepthRange) {
                    g_GLESFuncs.glDepthRangef(parameters.DepthRange.x(), parameters.DepthRange.y());
                }
            }

            { // Stencil state
                for (SizeT faceIndex = 0; faceIndex < parameters.StencilStates.size(); ++faceIndex) {
                    const StencilFaceState& current = parameters.StencilStates[faceIndex];
                    const StencilFaceState& synced = g_syncedRenderStateParameters.StencilStates[faceIndex];
                    const GLenum glFace = faceIndex == 0 ? GL_FRONT : GL_BACK;

                    if (current.Func != synced.Func || current.Ref != synced.Ref ||
                        current.ValueMask != synced.ValueMask) {
                        g_GLESFuncs.glStencilFuncSeparate(
                            glFace, MG_Util::ConvertDepthTestFuncToGLEnum(current.Func), current.Ref,
                            current.ValueMask);
                    }
                    if (current.WriteMask != synced.WriteMask) {
                        g_GLESFuncs.glStencilMaskSeparate(glFace, current.WriteMask);
                    }
                    if (current.FailOp != synced.FailOp || current.PassDepthFailOp != synced.PassDepthFailOp ||
                        current.PassDepthPassOp != synced.PassDepthPassOp) {
                        g_GLESFuncs.glStencilOpSeparate(
                            glFace, MG_Util::ConvertStencilOperationToGLEnum(current.FailOp),
                            MG_Util::ConvertStencilOperationToGLEnum(current.PassDepthFailOp),
                            MG_Util::ConvertStencilOperationToGLEnum(current.PassDepthPassOp));
                    }
                }
            }

            { // Color mask
                if (parameters.ColorMask != g_syncedRenderStateParameters.ColorMask) {
                    const BoolVec4& colorMask = parameters.ColorMask;
                    g_GLESFuncs.glColorMask(ToGLBoolean(colorMask.x()), ToGLBoolean(colorMask.y()),
                                            ToGLBoolean(colorMask.z()), ToGLBoolean(colorMask.w()));
                }
            }

            { // Clear values
                if (parameters.ClearColor != g_syncedRenderStateParameters.ClearColor) {
                    const FloatVec4& clearCol = parameters.ClearColor;
                    g_GLESFuncs.glClearColor(clearCol.x(), clearCol.y(), clearCol.z(), clearCol.w());
                }
                if (parameters.ClearDepth != g_syncedRenderStateParameters.ClearDepth) {
                    g_GLESFuncs.glClearDepthf(parameters.ClearDepth);
                }
                if (parameters.BlendColor != g_syncedRenderStateParameters.BlendColor) {
                    const FloatVec4& blendColor = parameters.BlendColor;
                    g_GLESFuncs.glBlendColor(blendColor.x(), blendColor.y(), blendColor.z(), blendColor.w());
                }
            }

            { // Cull face mode
                if (parameters.CullFaceModeSetting != g_syncedRenderStateParameters.CullFaceModeSetting) {
                    const CullFaceMode& cfm = parameters.CullFaceModeSetting;
                    g_GLESFuncs.glCullFace(MG_Util::ConvertCullFaceModeToGLEnum(cfm));
                }
                if (parameters.FrontFaceModeSetting != g_syncedRenderStateParameters.FrontFaceModeSetting) {
                    const FrontFaceMode& ffm = parameters.FrontFaceModeSetting;
                    g_GLESFuncs.glFrontFace(MG_Util::ConvertFrontFaceModeToGLEnum(ffm));
                }
            }

            { // Scissor box
                if (parameters.ScissorBox != g_syncedRenderStateParameters.ScissorBox) {
                    const IntVec4& scissorBox = parameters.ScissorBox;
                    g_GLESFuncs.glScissor(scissorBox.x(), scissorBox.y(), scissorBox.z(), scissorBox.w());
                }
            }

            { // Logic op
                if (parameters.LogicOp != g_syncedRenderStateParameters.LogicOp) {
                    g_GLESFuncs.glLogicOp(MG_Util::ConvertLogicOperationToGLEnum(parameters.LogicOp));
                }
            }

            { // Polygon offset
                if (parameters.PolygonOffsetFactor != g_syncedRenderStateParameters.PolygonOffsetFactor ||
                    parameters.PolygonOffsetUnits != g_syncedRenderStateParameters.PolygonOffsetUnits) {
                    g_GLESFuncs.glPolygonOffset(parameters.PolygonOffsetFactor, parameters.PolygonOffsetUnits);
                }
            }

            { // Line width
                if (parameters.LineWidth != g_syncedRenderStateParameters.LineWidth) {
                    g_GLESFuncs.glLineWidth(parameters.LineWidth);
                }
            }

            { // Point size
                if (parameters.PointSize != g_syncedRenderStateParameters.PointSize) {
                    g_GLESFuncs.glPointSize(parameters.PointSize);
                }
            }

            { // Sample coverage
                if (parameters.SampleCoverageValue != g_syncedRenderStateParameters.SampleCoverageValue ||
                    parameters.SampleCoverageInvert != g_syncedRenderStateParameters.SampleCoverageInvert) {
                    g_GLESFuncs.glSampleCoverage(parameters.SampleCoverageValue,
                                                ToGLBoolean(parameters.SampleCoverageInvert));
                }
            }

            { // Sample mask
                if (g_GLESFuncs.glSampleMaski && parameters.SampleMaskValue != g_syncedRenderStateParameters.SampleMaskValue) {
                    g_GLESFuncs.glSampleMaski(0, parameters.SampleMaskValue);
                }
            }

            g_syncedRenderStateVersion = currentRenderStateVersion;
            g_syncedRenderStateParameters = parameters;
            g_hasSyncedRenderState = true;
        }
    } // namespace RenderStateImpl

    namespace PrgramImpl {
        void SyncCurrentProgram() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            g_backendProgramObjects.CollectGarbageIfNeeded();
            SamplerImpl::g_backendSamplerObjects.CollectGarbageIfNeeded();

            auto& currentProgram = MG_State::pGLContext->GetCurrentProgram();
            if (!currentProgram || !currentProgram->GetLinkStatus()) {
                g_GLESFuncs.glUseProgram(0);
                return;
            }
            const auto& backendProgramIt = g_backendProgramObjects.find(currentProgram.get());
            Bool exist = (backendProgramIt != g_backendProgramObjects.end());
            auto& backendObj = exist ? backendProgramIt->second : g_backendProgramObjects.GetOrCreate(currentProgram);
            if (!exist) {
                backendObj = MakeShared<BackendProgramObjectImpl>();
                backendObj->SyncToBackend(currentProgram);
            } else {
                if (!backendObj->GetBackendProgramId()) {
                    backendObj->SyncToBackend(currentProgram);
                }
            }
        }
    } // namespace PrgramImpl

    void BindCurrentFBO(FramebufferTarget target) {
#ifdef TRACY_ENABLE
        ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
        auto& slot = MG_State::pGLContext->GetFramebufferBindingSlot(target);
        if (slot.GetVersion() == FramebufferImpl::g_fboBindVersions[(SizeT)target]) return;

        const auto& currentFBO = slot.GetBoundObject();
        if (currentFBO && currentFBO != MG_Impl::GLImpl::FramebufferImpl::pDefaultFramebufferInfo->defaultFBO) {
            const auto& backendFBOIt = FramebufferImpl::g_backendFramebufferObjects.find(currentFBO.get());
            if (backendFBOIt != FramebufferImpl::g_backendFramebufferObjects.end()) {
                backendFBOIt->second->Bind(target);
            } else {
                MGLOG_E("No backend FBO found (maybe not synced) for current %s FBO, cannot bind FBO.",
                        (target == FramebufferTarget::Read ? "READ" : "DRAW"));
            }
        } else {
            MGLOG_D("Binding default framebuffer as %s FBO", (target == FramebufferTarget::Read ? "READ" : "DRAW"));
            g_GLESFuncs.glBindFramebuffer(target == FramebufferTarget::Draw ? GL_DRAW_FRAMEBUFFER : GL_READ_FRAMEBUFFER,
                                          0);
        }
    }

    void SyncAndBindFramebufferObject(const SharedPtr<MG_State::GLState::FramebufferObject>& framebuffer,
                                      FramebufferTarget target, Bool forceSync = false) {
#ifdef TRACY_ENABLE
        ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
        if (!framebuffer || framebuffer == MG_Impl::GLImpl::FramebufferImpl::pDefaultFramebufferInfo->defaultFBO) {
            g_GLESFuncs.glBindFramebuffer(target == FramebufferTarget::Draw ? GL_DRAW_FRAMEBUFFER : GL_READ_FRAMEBUFFER,
                                          0);
            return;
        }

        auto& registry = FramebufferImpl::g_backendFramebufferObjects;
        const auto& backendFBOIt = registry.find(framebuffer.get());
        const Bool exists = backendFBOIt != registry.end();
        auto& backendObj = exists ? backendFBOIt->second : registry.GetOrCreate(framebuffer);
        if (!exists) {
            backendObj = MakeShared<FramebufferImpl::BackendFramebufferObject>();
        }
        if (forceSync) {
            backendObj->InvalidateSyncedState();
        }

        backendObj->SyncToBackend(framebuffer, target);
        backendObj->Bind(target);
    }

    void ForceBindCurrentFBO(FramebufferTarget target) {
#ifdef TRACY_ENABLE
        ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
        auto& slot = MG_State::pGLContext->GetFramebufferBindingSlot(target);
        SyncAndBindFramebufferObject(slot.GetBoundObject(), target);
        FramebufferImpl::g_fboBindVersions[(SizeT)target] = slot.GetVersion();
    }

    void PrepareForDraw(DrawSyncBit syncBit) {
#ifdef TRACY_ENABLE
        ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
        BufferImpl::SyncNeccessaryBuffers(syncBit & DrawSyncBit::IndexBuffer, syncBit & DrawSyncBit::IndirectBuffer);
        VertexArrayImpl::SyncCurrentVAO();
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        PrgramImpl::SyncCurrentProgram();
        RenderStateImpl::SyncRenderState();

        BindCurrentFBO(FramebufferTarget::Draw);

        {
#ifdef TRACY_ENABLE
            ZoneScopedNC("BindCurrentVAO", TRACY_ZONECOLOR_BACKEND);
#endif
            const auto& currentVAO = MG_State::pGLContext->GetBoundVertexArray();
            if (currentVAO) {
                const auto& backendVAOIt = VertexArrayImpl::g_backendVertexArrayObjects.find(currentVAO.get());
                if (backendVAOIt != VertexArrayImpl::g_backendVertexArrayObjects.end()) {
                    backendVAOIt->second->Bind();
                }
            } else {
                g_GLESFuncs.glBindVertexArray(0);
            }
        }

        {
#ifdef TRACY_ENABLE
            ZoneScopedNC("BindCurrentTextures", TRACY_ZONECOLOR_BACKEND);
#endif
            Int maxTextureUnits = MG_State::GLState::TextureState::MAX_TEXTURE_IMAGE_UNITS;
            for (Int unit = 0; unit < maxTextureUnits; ++unit) {
                auto& textureUnit = MG_State::pGLContext->GetTextureUnitObject(unit);

                for (const auto& bindingSlot : textureUnit.GetAllBindingSlots()) {
                    const auto& textureObject = bindingSlot.GetBoundObject();
                    if (!textureObject) continue;

                    // Bind texture object
                    auto target = textureObject->GetTarget();
                    if (!TextureImpl::IsSupportedTextureTarget(target)) {
                        MGLOG_D("    Texture target %s is not supported, skipping.",
                                MG_Util::ConvertTextureTargetToString(target).c_str());
                        continue;
                    }
                    const auto& backendTextureIt = TextureImpl::g_backendTextureObjects.find(textureObject.get());
                    if (backendTextureIt == TextureImpl::g_backendTextureObjects.end()) continue;

                    GLenum targetGL = MG_Util::ConvertTextureTargetToGLEnum(target);
                    backendTextureIt->second->Bind(targetGL, unit);
                }

                // Bind sampler object if necessary
                const auto& samplerObject = textureUnit.GetSamplerObject();
                if (samplerObject) {
                    const auto& backendSamplerIt = SamplerImpl::g_backendSamplerObjects.find(samplerObject.get());
                    if (backendSamplerIt != SamplerImpl::g_backendSamplerObjects.end()) {
                        backendSamplerIt->second->Bind(unit);
                    }

                } else {
                }
            }
        }

        const auto& currentProgram = MG_State::pGLContext->GetCurrentProgram();
        if (currentProgram && currentProgram->GetLinkStatus()) {
#ifdef TRACY_ENABLE
            ZoneScopedNC("BindCurrentProgram", TRACY_ZONECOLOR_BACKEND);
#endif
            const auto& backendProgramIt = PrgramImpl::g_backendProgramObjects.find(currentProgram.get());
            if (backendProgramIt != PrgramImpl::g_backendProgramObjects.end()) {
                backendProgramIt->second->Use();
                auto backendProgramId = backendProgramIt->second->GetBackendProgramId();

                // Global UBO
                if (currentProgram->GetUBOSize() > 0) {
#ifdef TRACY_ENABLE
                    ZoneScopedNC("UpdateGlobalUBO", TRACY_ZONECOLOR_BACKEND);
#endif
                    g_GLESFuncs.glBindBuffer(GL_UNIFORM_BUFFER, backendProgramIt->second->GetBackendGlobalUBOId());
                    g_GLESFuncs.glBufferSubData(GL_UNIFORM_BUFFER, 0, currentProgram->GetUBOSize(),
                                                currentProgram->MapUBO());
                    g_GLESFuncs.glBindBuffer(GL_UNIFORM_BUFFER, 0);

                    Uint blockIndex = g_GLESFuncs.glGetUniformBlockIndex(backendProgramId,
                                                                         MG_Util::ShaderTranspiler::GLOBAL_UBO_NAME);
                    if (blockIndex == GL_INVALID_INDEX) {
                        MGLOG_W("Program %u has frontend global UBO storage, but backend has no %s block.",
                                currentProgram->GetExternalIndex(), MG_Util::ShaderTranspiler::GLOBAL_UBO_NAME);
                    } else {
                        g_GLESFuncs.glUniformBlockBinding(backendProgramId, blockIndex, 0);

                        g_GLESFuncs.glBindBufferBase(GL_UNIFORM_BUFFER, 0,
                                                     backendProgramIt->second->GetBackendGlobalUBOId());
                    }
                }

                {
#ifdef TRACY_ENABLE
                    ZoneScopedNC("UpdateUBO", TRACY_ZONECOLOR_BACKEND);
#endif
                    // Normal UBO
                    auto uboCount = currentProgram->GetActiveUniformBlocksCount();
                    Uint lastUBOBinding = 0; // to prevent overlapping bindings between global UBO and normal UBOs
                    for (Int i = 0; i < uboCount; ++i) {
                        ++lastUBOBinding;
                        // program state binding index == backend binding index

                        // Connect program ubo index to backend binding point
                        auto binding = currentProgram->GetUniformBlockBinding(i);
                        auto& name = currentProgram->GetUniformBlockName(i);
                        GLuint backendBlkIdx = g_GLESFuncs.glGetUniformBlockIndex(backendProgramId, name.c_str());
                        g_GLESFuncs.glUniformBlockBinding(backendProgramId, backendBlkIdx, lastUBOBinding);

                        // Connect buffer to backend binding point
                        auto& point = MG_State::pGLContext->GetBufferBindingPoint(BufferTarget::Uniform, binding);
                        auto& bufferObj = point.GetBoundObject();
                        auto range = point.GetRange();

                        if (bufferObj) {
                            const auto& backendBufferIt = BufferImpl::g_backendBufferObjects.find(bufferObj.get());
                            if (backendBufferIt != BufferImpl::g_backendBufferObjects.end()) {
                                const auto& backendBufferObject = backendBufferIt->second;
                                backendBufferObject->Bind(GL_UNIFORM_BUFFER);
                                if (range.end == 0) {
                                    g_GLESFuncs.glBindBufferBase(GL_UNIFORM_BUFFER, lastUBOBinding,
                                                                 backendBufferObject->GetBackendBufferId());
                                } else {
                                    g_GLESFuncs.glBindBufferRange(
                                        GL_UNIFORM_BUFFER, lastUBOBinding, backendBufferObject->GetBackendBufferId(),
                                        (GLintptr)range.start, (GLintptr)(range.end - range.start));
                                }
                            } else {
                                MGLOG_E("No backend buffer found for UBO binding, cannot bind UBO.");
                            }
                        }
                    }
                }

                {
#ifdef TRACY_ENABLE
                    ZoneScopedNC("BindSamplerUnit", TRACY_ZONECOLOR_BACKEND);
#endif
                    // Sampler unit binding
                    auto maxUniformLoc = currentProgram->GetMaxUniformLocation();
                    for (Uint loc = 0; loc <= maxUniformLoc; ++loc) {
                        auto& name = currentProgram->GetUniformName(loc);
                        if (name.empty()) continue;
                        auto unit = currentProgram->GetUniformSamplerOrImageUnitIndex(loc);
                        if (unit == -1) continue;
                        auto locAtBackend = g_GLESFuncs.glGetUniformLocation(
                            backendProgramIt->second->GetBackendProgramId(), name.c_str());
                        g_GLESFuncs.glUniform1i(locAtBackend, unit);

                        auto& samplerObject = MG_State::pGLContext->GetTextureUnitObject(unit).GetSamplerObject();

                        if (samplerObject) {
                            const auto& backendSamplerIt =
                                SamplerImpl::g_backendSamplerObjects.find(samplerObject.get());
                            Bool exist = (backendSamplerIt != SamplerImpl::g_backendSamplerObjects.end());
                            auto& backendObj = exist ? backendSamplerIt->second
                                                     : SamplerImpl::g_backendSamplerObjects.GetOrCreate(samplerObject);
                            if (!exist) {
                                backendObj = MakeShared<SamplerImpl::BackendSamplerObject>();
                            }
                            backendObj->SyncToBackend(samplerObject);
                        } else {
                            SamplerImpl::UnbindSampler(unit);
                        }
                    }
                }
            } else {
                g_GLESFuncs.glUseProgram(0);
                MGLOG_E("No backend program found (maybe not synced) for current program, cannot use program.");
            }
        }
    }

    void SetCurrentBaseInstance(Uint32 baseInstance) {
        const auto& currentProgram = MG_State::pGLContext->GetCurrentProgram();
        if (!currentProgram || !currentProgram->GetLinkStatus()) {
            return;
        }
        const auto& backendProgramIt = PrgramImpl::g_backendProgramObjects.find(currentProgram.get());
        if (backendProgramIt != PrgramImpl::g_backendProgramObjects.end()) {
            backendProgramIt->second->SetBaseInstance(baseInstance);
        }
    }

    void PrepareForCompute(Bool includeDispatchIndirectBuffer) {
#ifdef TRACY_ENABLE
        ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
        BufferImpl::SyncComputeBuffers(includeDispatchIndirectBuffer);
        TextureImpl::SyncNeccessaryTextures();
        TextureImpl::SyncImageTextureBindings();
        PrgramImpl::SyncCurrentProgram();

        const auto& currentProgram = MG_State::pGLContext->GetCurrentProgram();
        if (!currentProgram || !currentProgram->GetLinkStatus()) {
            g_GLESFuncs.glUseProgram(0);
            return;
        }

        const auto& backendProgramIt = PrgramImpl::g_backendProgramObjects.find(currentProgram.get());
        if (backendProgramIt != PrgramImpl::g_backendProgramObjects.end()) {
            backendProgramIt->second->Use();
        } else {
            g_GLESFuncs.glUseProgram(0);
            MGLOG_E("No backend program found (maybe not synced) for current compute program.");
        }
    }

    GLuint GetBackendProgramId(GLuint program) {
        if (!MG_State::pGLContext->ValidateProgramName(program)) {
            MGLOG_E("Invalid frontend program object: %u", program);
            return 0;
        }

        auto& programObject = MG_State::pGLContext->GetProgramObject(program);
        if (!programObject) {
            MGLOG_E("Program object %u is null.", program);
            return 0;
        }

        const auto& backendProgramIt = PrgramImpl::g_backendProgramObjects.find(programObject.get());
        Bool exist = (backendProgramIt != PrgramImpl::g_backendProgramObjects.end());
        auto& backendObj =
            exist ? backendProgramIt->second : PrgramImpl::g_backendProgramObjects.GetOrCreate(programObject);
        if (!exist) {
            backendObj = MakeShared<PrgramImpl::BackendProgramObjectImpl>();
        }
        if (!backendObj->GetBackendProgramId()) {
            backendObj->SyncToBackend(programObject);
        }
        return backendObj->GetBackendProgramId();
    }

    void Clear(GLbitfield mask) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        RenderStateImpl::SyncRenderState();

        BindCurrentFBO(FramebufferTarget::Draw);

        g_GLESFuncs.glClear(mask);
    }

    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer;
        PrepareForDraw(syncBit);
        g_GLESFuncs.glDrawElements(mode, count, type, indices);
    }

    void DrawArrays(GLenum mode, GLint first, GLsizei count) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DrawSyncBit syncBit = DrawSyncBit::None;
        PrepareForDraw(syncBit);
        const auto& currentVAO = MG_State::pGLContext->GetBoundVertexArray();
        if (currentVAO) {
            const auto& backendVAOIt = VertexArrayImpl::g_backendVertexArrayObjects.find(currentVAO.get());
            if (backendVAOIt != VertexArrayImpl::g_backendVertexArrayObjects.end()) {
                backendVAOIt->second->SyncClientSideAttributesForDrawArrays(currentVAO, first, count);
            }
        }
        g_GLESFuncs.glDrawArrays(mode, first, count);
    }

    void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLint basevertex) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer;
        PrepareForDraw(syncBit);
        g_GLESFuncs.glDrawElementsBaseVertex(mode, count, type, indices, basevertex);
    }

    void MultiDrawElements(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices,
                           GLsizei drawcount) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer;
        PrepareForDraw(syncBit);

        for (GLsizei i = 0; i < drawcount; ++i) {
            g_GLESFuncs.glDrawElements(mode, count[i], type, indices[i]);
        }
    }

    void MultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices,
                                     GLsizei drawcount, const GLint* basevertex) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer;
        PrepareForDraw(syncBit);

        for (GLsizei i = 0; i < drawcount; ++i) {
            g_GLESFuncs.glDrawElementsBaseVertex(mode, count[i], type, indices[i], basevertex[i]);
        }
    }

    void MultiDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        if (drawcount <= 0) {
            return;
        }
        if (stride == 0) {
            stride = sizeof(DrawElementsIndirectCommand);
        }
        if (stride < static_cast<GLsizei>(sizeof(DrawElementsIndirectCommand))) {
            MGLOG_E("MultiDrawElementsIndirect skipped: stride %d is smaller than command size %zu",
                    stride, sizeof(DrawElementsIndirectCommand));
            return;
        }

        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer | DrawSyncBit::IndirectBuffer | DrawSyncBit::Instancing;
        PrepareForDraw(syncBit);

        const SizeT indexSize = MG_Util::GetGLTypeSize(type);
        if (indexSize == 0) {
            MGLOG_E("MultiDrawElementsIndirect skipped: unsupported index type 0x%x", type);
            return;
        }

        auto drawBuffer = MG_State::pGLContext->GetBufferBindingSlot(BufferTarget::DrawIndirect).GetBoundObject();
        if (!drawBuffer) {
            MGLOG_E("MultiDrawElementsIndirect skipped: no GL_DRAW_INDIRECT_BUFFER is bound");
            return;
        }
        drawBuffer->MarkPersistentMappedRangeDirty();
        const auto drawData = drawBuffer->GetDataReadOnly();
        const SizeT commandOffset = reinterpret_cast<SizeT>(indirect);
        const SizeT commandBytes = commandOffset + static_cast<SizeT>(stride) * static_cast<SizeT>(drawcount - 1) +
            sizeof(DrawElementsIndirectCommand);
        if (!drawData || commandBytes > drawData->size()) {
            MGLOG_E("MultiDrawElementsIndirect skipped: invalid GL_DRAW_INDIRECT_BUFFER binding or range");
            return;
        }

        for (GLsizei i = 0; i < drawcount; ++i) {
            DrawElementsIndirectCommand cmd{};
            std::memcpy(&cmd, drawData->data() + commandOffset + static_cast<SizeT>(i) * stride, sizeof(cmd));
            if (cmd.count == 0 || cmd.instanceCount == 0) {
                continue;
            }
            SetCurrentBaseInstance(cmd.baseInstance);
            const auto indexByteOffset = static_cast<SizeT>(cmd.firstIndex) * indexSize;
            g_GLESFuncs.glDrawElementsInstancedBaseVertex(
                mode, static_cast<GLsizei>(cmd.count), type, reinterpret_cast<const GLvoid*>(indexByteOffset),
                static_cast<GLsizei>(cmd.instanceCount), cmd.baseVertex);
        }
        SetCurrentBaseInstance(0);
    }

    void MultiDrawElementsIndirectCount(GLenum mode, GLenum type, const void* indirect, GLintptr drawcount,
                                        GLsizei maxdrawcount, GLsizei stride) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        if (maxdrawcount <= 0) {
            return;
        }
        if (stride == 0) {
            stride = sizeof(DrawElementsIndirectCommand);
        }
        if (stride < static_cast<GLsizei>(sizeof(DrawElementsIndirectCommand))) {
            MGLOG_E("MultiDrawElementsIndirectCount skipped: stride %d is smaller than command size %zu",
                    stride, sizeof(DrawElementsIndirectCommand));
            return;
        }

        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer | DrawSyncBit::IndirectBuffer | DrawSyncBit::Instancing;
        PrepareForDraw(syncBit);

        const SizeT indexSize = MG_Util::GetGLTypeSize(type);
        if (indexSize == 0) {
            MGLOG_E("MultiDrawElementsIndirectCount skipped: unsupported index type 0x%x", type);
            return;
        }

        auto drawBuffer = MG_State::pGLContext->GetBufferBindingSlot(BufferTarget::DrawIndirect).GetBoundObject();
        auto parameterBuffer = MG_State::pGLContext->GetBufferBindingSlot(BufferTarget::Parameter).GetBoundObject();
        if (!drawBuffer) {
            MGLOG_E("MultiDrawElementsIndirectCount skipped: no GL_DRAW_INDIRECT_BUFFER is bound");
            return;
        }
        if (!parameterBuffer) {
            MGLOG_E("MultiDrawElementsIndirectCount skipped: no GL_PARAMETER_BUFFER is bound");
            return;
        }

        drawBuffer->MarkPersistentMappedRangeDirty();
        parameterBuffer->MarkPersistentMappedRangeDirty();
        const auto drawData = drawBuffer->GetDataReadOnly();
        const auto parameterData = parameterBuffer->GetDataReadOnly();

        const SizeT commandOffset = reinterpret_cast<SizeT>(indirect);
        const SizeT commandBytes = commandOffset + static_cast<SizeT>(stride) * static_cast<SizeT>(maxdrawcount - 1) +
            sizeof(DrawElementsIndirectCommand);
        if (!drawData || commandBytes > drawData->size()) {
            MGLOG_E("MultiDrawElementsIndirectCount skipped: invalid GL_DRAW_INDIRECT_BUFFER binding or range");
            return;
        }
        if (!parameterData || drawcount < 0 || static_cast<SizeT>(drawcount) + sizeof(Uint32) > parameterData->size()) {
            MGLOG_E("MultiDrawElementsIndirectCount skipped: invalid GL_PARAMETER_BUFFER binding or range");
            return;
        }

        Uint32 actualDrawCount = 0;
        std::memcpy(&actualDrawCount, parameterData->data() + drawcount, sizeof(actualDrawCount));
        actualDrawCount = std::min<Uint32>(actualDrawCount, static_cast<Uint32>(maxdrawcount));
        for (Uint32 i = 0; i < actualDrawCount; ++i) {
            DrawElementsIndirectCommand cmd{};
            std::memcpy(&cmd, drawData->data() + commandOffset + static_cast<SizeT>(i) * stride, sizeof(cmd));
            if (cmd.count == 0 || cmd.instanceCount == 0) {
                continue;
            }
            SetCurrentBaseInstance(cmd.baseInstance);
            const auto indexByteOffset = static_cast<SizeT>(cmd.firstIndex) * indexSize;
            g_GLESFuncs.glDrawElementsInstancedBaseVertex(
                mode, static_cast<GLsizei>(cmd.count), type, reinterpret_cast<const GLvoid*>(indexByteOffset),
                static_cast<GLsizei>(cmd.instanceCount), cmd.baseVertex);
        }
        SetCurrentBaseInstance(0);
    }

    void MultiDrawArraysIndirect(GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DrawSyncBit syncBit = DrawSyncBit::IndirectBuffer;
        PrepareForDraw(syncBit);

        for (GLsizei i = 0; i < drawcount; ++i) {
            const GLvoid* cmd = reinterpret_cast<const GLvoid*>(reinterpret_cast<const uint8_t*>(indirect) +
                                                                i * (stride ? stride : sizeof(GLsizei) * 4));
            g_GLESFuncs.glDrawArraysIndirect(mode, cmd);
        }
    }

    void DrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type,
                                     const void* indices, GLint basevertex) {
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer;
        PrepareForDraw(syncBit);
        g_GLESFuncs.glDrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex);
    }

    void DrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices) {
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer;
        PrepareForDraw(syncBit);
        g_GLESFuncs.glDrawRangeElements(mode, start, end, count, type, indices);
    }

    void DrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                                     GLsizei instancecount, GLint basevertex, GLuint baseinstance) {
        // Not supported in OpenGL ES
        MGLOG_W("DrawElementsInstancedBaseVertexBaseInstance is not supported in OpenGL ES.");
    }

    void DrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                         GLsizei instancecount, GLint basevertex) {
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer | DrawSyncBit::Instancing;
        PrepareForDraw(syncBit);
        g_GLESFuncs.glDrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex);
    }

    void DrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                           GLsizei instancecount, GLuint baseinstance) {
        // Not supported in OpenGL ES
        MGLOG_W("DrawElementsInstancedBaseInstance is not supported in OpenGL ES.");
    }

    void DrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount) {
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer | DrawSyncBit::Instancing;
        PrepareForDraw(syncBit);
        g_GLESFuncs.glDrawElementsInstanced(mode, count, type, indices, instancecount);
    }

    void DrawElementsIndirect(GLenum mode, GLenum type, const void* indirect) {
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer | DrawSyncBit::IndirectBuffer;
        PrepareForDraw(syncBit);
        g_GLESFuncs.glDrawElementsIndirect(mode, type, indirect);
    }

    void DrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instancecount,
                                         GLuint baseinstance) {
        // Not supported in OpenGL ES
        MGLOG_W("DrawArraysInstancedBaseInstance is not supported in OpenGL ES.");
    }

    void DrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {
        DrawSyncBit syncBit = DrawSyncBit::Instancing;
        PrepareForDraw(syncBit);
        g_GLESFuncs.glDrawArraysInstanced(mode, first, count, instancecount);
    }

    void DrawArraysIndirect(GLenum mode, const void* indirect) {
        DrawSyncBit syncBit = DrawSyncBit::IndirectBuffer;
        PrepareForDraw(syncBit);
        g_GLESFuncs.glDrawArraysIndirect(mode, indirect);
    }

    void BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1,
                         GLint dstY1, GLbitfield mask, GLenum filter) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif

        TextureImpl::SyncNeccessaryTextures();
        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        FramebufferImpl::SyncCurrentFBO();
        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        RenderStateImpl::SyncRenderState();
        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });

        BindCurrentFBO(FramebufferTarget::Draw);
        BindCurrentFBO(FramebufferTarget::Read);
        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        MGLOG_D("ES %s(%d, %d, %d, %d, %d, %d, %d, %d, 0x%x, %s)", __func__, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0,
                dstX1, dstY1, mask, MG_Util::ConvertGLEnumToString(filter).c_str());
        g_GLESFuncs.glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
    }

    void BlitNamedFramebuffer(const SharedPtr<MG_State::GLState::FramebufferObject>& readFramebuffer,
                              const SharedPtr<MG_State::GLState::FramebufferObject>& drawFramebuffer,
                              GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                              GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                              GLbitfield mask, GLenum filter) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        TextureImpl::SyncNeccessaryTextures();
        RenderStateImpl::SyncRenderState();

        SyncAndBindFramebufferObject(readFramebuffer, FramebufferTarget::Read, true);
        SyncAndBindFramebufferObject(drawFramebuffer, FramebufferTarget::Draw, true);

        MGLOG_D("ES %s(%d, %d, %d, %d, %d, %d, %d, %d, 0x%x, %s)", __func__, srcX0, srcY0, srcX1, srcY1,
                dstX0, dstY0, dstX1, dstY1, mask, MG_Util::ConvertGLEnumToString(filter).c_str());
        g_GLESFuncs.glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });

        ForceBindCurrentFBO(FramebufferTarget::Read);
        ForceBindCurrentFBO(FramebufferTarget::Draw);
    }

    Bool UpdateTextureBindingAtTarget(GLenum target) {
#ifdef TRACY_ENABLE
        ZoneScopedNC(__func__, TRACY_ZONECOLOR_BACKEND);
#endif
        auto unit = MG_State::pGLContext->GetActiveTextureUnit();
        auto& textureUnit = MG_State::pGLContext->GetTextureUnitObject(unit);

        auto textureTarget = MG_Util::ConvertGLEnumToTextureTarget(target);
        if (!TextureImpl::IsSupportedTextureTarget(textureTarget)) {
            MGLOG_E("    Texture target %s is not supported, skipping.",
                    MG_Util::ConvertTextureTargetToString(textureTarget).c_str());
            return false;
        }

        const auto& bindingSlot = textureUnit.GetBindingSlot(textureTarget);
        {
            const auto& textureObject = bindingSlot.GetBoundObject();
            if (!textureObject) {
                MGLOG_W("%s: Texture target %s does not have texture bound.", __func__,
                        MG_Util::ConvertTextureTargetToString(textureTarget).c_str());
            }

            const auto& backendTextureIt = TextureImpl::g_backendTextureObjects.find(textureObject.get());
            Bool exist = (backendTextureIt != TextureImpl::g_backendTextureObjects.end());
            auto& backendObj =
                exist ? backendTextureIt->second : TextureImpl::g_backendTextureObjects.GetOrCreate(textureObject);
            if (!exist) {
                backendObj = MakeShared<TextureImpl::BackendTextureObject>();
            }
            backendObj->Bind(target, unit);
        }
        return true;
    }

    static GLuint s_prevDrawFBO = 0;
    static GLuint s_prevReadFBO = 0;
    void BindTempFBO(Bool isRead) {
        MGLOG_D("%s: Binding temporary FBO for operations like CopyTexImage2D that require framebuffer binding, "
                "previous draw FBO=%u, read FBO=%u",
                __func__, s_prevDrawFBO, s_prevReadFBO);
        static GLuint tempFBO = 0;
        if (!tempFBO) {
            g_GLESFuncs.glGenFramebuffers(1, &tempFBO);
        }
        if (isRead) {
            g_GLESFuncs.glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&s_prevReadFBO);
            g_GLESFuncs.glBindFramebuffer(GL_READ_FRAMEBUFFER, tempFBO);
        } else {
            g_GLESFuncs.glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&s_prevDrawFBO);
            g_GLESFuncs.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tempFBO);
        }
    }
    void RestoreFBOFromTemp(Bool isRead) {
        if (isRead) {
            MGLOG_D("%s: Restoring previous read FBO=%u", __func__, s_prevReadFBO);
            g_GLESFuncs.glBindFramebuffer(GL_READ_FRAMEBUFFER, s_prevReadFBO);
        } else {
            MGLOG_D("%s: Restoring previous draw FBO=%u", __func__, s_prevDrawFBO);
            g_GLESFuncs.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, s_prevDrawFBO);
        }
    }

    class TempFBOBinder {
    public:
        TempFBOBinder(Bool isRead) : m_isRead(isRead) { BindTempFBO(isRead); }
        ~TempFBOBinder() { RestoreFBOFromTemp(m_isRead); }

    private:
        const Bool m_isRead = false;
    };

    void CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width,
                        GLsizei height, GLint border) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DebugImpl::ErrorLopper errorLopper;
        MGLOG_D("%s: Backend", __func__);
        TextureImpl::SyncNeccessaryTextures();
        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        FramebufferImpl::SyncCurrentFBO();
        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        RenderStateImpl::SyncRenderState();
        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        if (!UpdateTextureBindingAtTarget(target)) return;

        // Bind necessary FBO and texture
        BindCurrentFBO(FramebufferTarget::Read);
        Uint activeTextureUnit = MG_State::pGLContext->GetActiveTextureUnit();
        const auto& textureObject = MG_State::pGLContext->GetTextureUnitObject((Int)activeTextureUnit)
                                        .GetBindingSlot(MG_Util::ConvertGLEnumToTextureTarget(target))
                                        .GetBoundObject();
        const auto& backendTextureIt = TextureImpl::g_backendTextureObjects.find(textureObject.get());
        if (backendTextureIt == TextureImpl::g_backendTextureObjects.end()) {
            MGLOG_E("CopyTexSubImage2D: No backend texture found for texture %u.",
                    textureObject ? textureObject->GetExternalIndex() : 0);
            return;
        }
        backendTextureIt->second->Bind(target, activeTextureUnit);

        auto mgInternalFormat = textureObject->GetFormat();
        GLenum format = GL_DEPTH_COMPONENT;
        GLenum type = GL_UNSIGNED_INT;
        TextureImpl::GenerateTextureFormatInfo(mgInternalFormat, &internalformat, &format, &type);
        MOBILEGL_ASSERT(format != GL_NONE && type != GL_NONE,
                        "%s: cannot GenerateTextureFormatInfo(%s): out internalformat=%s, format=%s, type=%s",
                        MG_Util::ConvertTextureInternalFormatToString(mgInternalFormat).c_str(),
                        MG_Util::ConvertGLEnumToString(internalformat).c_str(),
                        MG_Util::ConvertGLEnumToString(format).c_str(), MG_Util::ConvertGLEnumToString(type).c_str());
        TexturePixelDataType texturePixelDataType = MG_Util::ConvertGLEnumToTexturePixelDataType(type);

        Bool isDepthFormat =
            MG_Util::IsDepthFormatInternalFormat(MG_Util::ConvertGLEnumToTextureInternalFormat(internalformat));
        Bool isStencilFormat =
            MG_Util::IsStencilFormatInternalFormat(MG_Util::ConvertGLEnumToTextureInternalFormat(internalformat));

        if (!isDepthFormat) {
            g_GLESFuncs.glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
            DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
        } else {
            MGLOG_D("%s: Backend depth", __func__);
            g_GLESFuncs.glTexImage2D(target, level, (GLint)internalformat, width, height, border, format, type,
                                     nullptr);
            DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });

            auto currentTex = (GLint)backendTextureIt->second->GetBackendTextureId();
            DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });

            GLenum attachment = isStencilFormat ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
            TempFBOBinder tempFBOBinder(false);
            g_GLESFuncs.glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, target, currentTex, level);

            if (g_GLESFuncs.glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                MGLOG_E("ES glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE");
                return;
            }

            g_GLESFuncs.glBlitFramebuffer(x, y, x + width, y + height, 0, 0, width, height,
                                          GL_DEPTH_BUFFER_BIT | (isStencilFormat ? GL_STENCIL_BUFFER_BIT : 0),
                                          GL_NEAREST);
            DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
        }
    }

    void CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width,
                           GLsizei height) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DebugImpl::ErrorLopper errorLopper;

        MGLOG_D("%s: Backend", __func__);
        TextureImpl::SyncNeccessaryTextures();
        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        FramebufferImpl::SyncCurrentFBO();
        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        RenderStateImpl::SyncRenderState();
        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });

        if (!UpdateTextureBindingAtTarget(target)) return;

        // Bind necessary FBO and texture
        BindCurrentFBO(FramebufferTarget::Read);
        auto activeTextureUnit = MG_State::pGLContext->GetActiveTextureUnit();
        const auto& textureObject = MG_State::pGLContext->GetTextureUnitObject(activeTextureUnit)
                                        .GetBindingSlot(MG_Util::ConvertGLEnumToTextureTarget(target))
                                        .GetBoundObject();
        const auto& backendTextureIt = TextureImpl::g_backendTextureObjects.find(textureObject.get());
        if (backendTextureIt == TextureImpl::g_backendTextureObjects.end()) {
            MGLOG_E("CopyTexSubImage2D: No backend texture found for texture %u.",
                    textureObject ? textureObject->GetExternalIndex() : 0);
            return;
        }
        backendTextureIt->second->Bind(target, activeTextureUnit);

        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        GLenum internalFormat;
        g_GLESFuncs.glGetTexLevelParameteriv(target, level, GL_TEXTURE_INTERNAL_FORMAT, (GLint*)&internalFormat);
        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        auto mgInternalFormat = MG_Util::ConvertGLEnumToTextureInternalFormat(internalFormat);

        Bool isDepthFormat = MG_Util::IsDepthFormatInternalFormat(mgInternalFormat);
        Bool isStencilFormat = MG_Util::IsStencilFormatInternalFormat(mgInternalFormat);

        if (!isDepthFormat) {
            g_GLESFuncs.glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
            DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
        } else {
            MGLOG_D("%s: Backend depth", __func__);
            auto currentTex = backendTextureIt->second->GetBackendTextureId();
            DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
            GLenum attachment = isStencilFormat ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
            TempFBOBinder tempFBOBinder(false);
            g_GLESFuncs.glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, target, currentTex, level);
            DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
            if (g_GLESFuncs.glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                MGLOG_E("ES glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE");
                return;
            }

            g_GLESFuncs.glBlitFramebuffer(
                x, y, x + width, y + height, xoffset, yoffset, xoffset + width, yoffset + height,
                GL_DEPTH_BUFFER_BIT | (isStencilFormat ? GL_STENCIL_BUFFER_BIT : 0), GL_NEAREST);
            DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
        }
    }

    void GenerateMipmap(GLenum target) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        auto unitIndex = MG_State::pGLContext->GetActiveTextureUnit();
        auto& unit = MG_State::pGLContext->GetTextureUnitObject(unitIndex);
        auto& slot = unit.GetBindingSlot(MG_Util::ConvertGLEnumToTextureTarget(target));
        auto& texture = slot.GetBoundObject();
        auto& backendTexture = TextureImpl::SyncTextureObjectToBackend(texture);

        backendTexture->Bind(target, unitIndex);
        g_GLESFuncs.glGenerateMipmap(target);
    }

    const GLubyte* GetString(GLenum name) {
        return g_GLESFuncs.glGetString(name);
    }

    void DispatchCompute(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        PrepareForCompute(false);
        g_GLESFuncs.glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
    }

    void DispatchComputeIndirect(GLintptr indirect) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        PrepareForCompute(true);
        g_GLESFuncs.glDispatchComputeIndirect(indirect);
    }

    void MemoryBarrier(GLbitfield barriers) {
        g_GLESFuncs.glMemoryBarrier(barriers);
    }

    void MemoryBarrierByRegion(GLbitfield barriers) {
        g_GLESFuncs.glMemoryBarrierByRegion(barriers);
    }

    void BindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access,
                          GLenum format) {
        (void)texture;
        (void)level;
        (void)layered;
        (void)layer;
        (void)access;
        (void)format;
        TextureImpl::SyncImageTextureBinding(unit);
    }

    void GetIntegeri_v(GLenum target, GLuint index, GLint* data) {
        if (!data) return;

        switch (target) {
        case GL_SHADER_STORAGE_BUFFER_BINDING: {
            auto& point = MG_State::pGLContext->GetBufferBindingPoint(BufferTarget::ShaderStorage, index);
            auto& obj = point.GetBoundObject();
            *data = obj ? static_cast<GLint>(obj->GetExternalIndex()) : 0;
            return;
        }
        case GL_SHADER_STORAGE_BUFFER_START: {
            auto& point = MG_State::pGLContext->GetBufferBindingPoint(BufferTarget::ShaderStorage, index);
            *data = static_cast<GLint>(point.GetRange().start);
            return;
        }
        case GL_SHADER_STORAGE_BUFFER_SIZE: {
            auto& point = MG_State::pGLContext->GetBufferBindingPoint(BufferTarget::ShaderStorage, index);
            auto& obj = point.GetBoundObject();
            if (!obj) {
                *data = 0;
                return;
            }
            const auto& range = point.GetRange();
            const auto start = std::min(range.start, obj->GetSize());
            const auto end = std::min(range.end, obj->GetSize());
            *data = static_cast<GLint>(end - start);
            return;
        }
        case GL_IMAGE_BINDING_NAME: {
            if (index >= MG_State::GLState::TextureState::MAX_TEXTURE_IMAGE_UNITS) {
                *data = 0;
                return;
            }
            auto& imageBinding = MG_State::pGLContext->GetImageTextureBinding(static_cast<Int>(index));
            *data = imageBinding.Texture ? static_cast<GLint>(imageBinding.Texture->GetExternalIndex()) : 0;
            return;
        }
        case GL_IMAGE_BINDING_LEVEL: {
            if (index >= MG_State::GLState::TextureState::MAX_TEXTURE_IMAGE_UNITS) {
                *data = 0;
                return;
            }
            auto& imageBinding = MG_State::pGLContext->GetImageTextureBinding(static_cast<Int>(index));
            *data = imageBinding.Level;
            return;
        }
        case GL_IMAGE_BINDING_LAYERED: {
            if (index >= MG_State::GLState::TextureState::MAX_TEXTURE_IMAGE_UNITS) {
                *data = 0;
                return;
            }
            auto& imageBinding = MG_State::pGLContext->GetImageTextureBinding(static_cast<Int>(index));
            *data = imageBinding.Layered;
            return;
        }
        case GL_IMAGE_BINDING_LAYER: {
            if (index >= MG_State::GLState::TextureState::MAX_TEXTURE_IMAGE_UNITS) {
                *data = 0;
                return;
            }
            auto& imageBinding = MG_State::pGLContext->GetImageTextureBinding(static_cast<Int>(index));
            *data = imageBinding.Layer;
            return;
        }
        case GL_IMAGE_BINDING_ACCESS: {
            if (index >= MG_State::GLState::TextureState::MAX_TEXTURE_IMAGE_UNITS) {
                *data = 0;
                return;
            }
            auto& imageBinding = MG_State::pGLContext->GetImageTextureBinding(static_cast<Int>(index));
            *data = static_cast<GLint>(imageBinding.Access);
            return;
        }
        case GL_IMAGE_BINDING_FORMAT: {
            if (index >= MG_State::GLState::TextureState::MAX_TEXTURE_IMAGE_UNITS) {
                *data = 0;
                return;
            }
            auto& imageBinding = MG_State::pGLContext->GetImageTextureBinding(static_cast<Int>(index));
            *data = static_cast<GLint>(imageBinding.Format);
            return;
        }
        default:
            if (g_GLESFuncs.glGetIntegeri_v) {
                g_GLESFuncs.glGetIntegeri_v(target, index, data);
            } else {
                *data = 0;
            }
            return;
        }
    }

    void GetInteger64i_v(GLenum target, GLuint index, GLint64* data) {
        if (!data) return;

        switch (target) {
        case GL_SHADER_STORAGE_BUFFER_START: {
            auto& point = MG_State::pGLContext->GetBufferBindingPoint(BufferTarget::ShaderStorage, index);
            *data = static_cast<GLint64>(point.GetRange().start);
            return;
        }
        case GL_SHADER_STORAGE_BUFFER_SIZE: {
            auto& point = MG_State::pGLContext->GetBufferBindingPoint(BufferTarget::ShaderStorage, index);
            auto& obj = point.GetBoundObject();
            if (!obj) {
                *data = 0;
                return;
            }
            const auto& range = point.GetRange();
            const auto start = std::min(range.start, obj->GetSize());
            const auto end = std::min(range.end, obj->GetSize());
            *data = static_cast<GLint64>(end - start);
            return;
        }
        default:
            if (g_GLESFuncs.glGetInteger64i_v) {
                g_GLESFuncs.glGetInteger64i_v(target, index, data);
            } else {
                *data = 0;
            }
            return;
        }
    }

    void GetProgramiv(GLuint program, GLenum pname, GLint* params) {
        if (!params) return;
        GLuint backendProgramId = GetBackendProgramId(program);
        if (!backendProgramId) {
            params[0] = 0;
            return;
        }
        g_GLESFuncs.glGetProgramiv(backendProgramId, pname, params);
    }

    void GetProgramInterfaceiv(GLuint program, GLenum programInterface, GLenum pname, GLint* params) {
        GLuint backendProgramId = GetBackendProgramId(program);
        if (!backendProgramId) return;
        g_GLESFuncs.glGetProgramInterfaceiv(backendProgramId, programInterface, pname, params);
    }

    GLuint GetProgramResourceIndex(GLuint program, GLenum programInterface, const GLchar* name) {
        GLuint backendProgramId = GetBackendProgramId(program);
        if (!backendProgramId) return GL_INVALID_INDEX;
        return g_GLESFuncs.glGetProgramResourceIndex(backendProgramId, programInterface, name);
    }

    void GetProgramResourceName(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length,
                                GLchar* name) {
        GLuint backendProgramId = GetBackendProgramId(program);
        if (!backendProgramId) return;
        g_GLESFuncs.glGetProgramResourceName(backendProgramId, programInterface, index, bufSize, length, name);
    }

    void GetProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount,
                              const GLenum* props, GLsizei bufSize, GLsizei* length, GLint* params) {
        GLuint backendProgramId = GetBackendProgramId(program);
        if (!backendProgramId) return;
        g_GLESFuncs.glGetProgramResourceiv(backendProgramId, programInterface, index, propCount, props, bufSize, length,
                                           params);
    }

    GLint GetProgramResourceLocation(GLuint program, GLenum programInterface, const GLchar* name) {
        GLuint backendProgramId = GetBackendProgramId(program);
        if (!backendProgramId) return -1;
        return g_GLESFuncs.glGetProgramResourceLocation(backendProgramId, programInterface, name);
    }

    GLint GetProgramResourceLocationIndex(GLuint program, GLenum programInterface, const GLchar* name) {
        (void)program;
        (void)programInterface;
        (void)name;
        return -1;
    }

    void ShaderStorageBlockBinding(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding) {
        GLuint backendProgramId = GetBackendProgramId(program);
        if (!backendProgramId) return;
        g_GLESFuncs.glShaderStorageBlockBinding(backendProgramId, storageBlockIndex, storageBlockBinding);
    }

    void ClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        RenderStateImpl::SyncRenderState();

        BindCurrentFBO(FramebufferTarget::Draw);

        g_GLESFuncs.glClearBufferfi(buffer, drawbuffer, depth, stencil);
    }

    void ClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value) {
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        RenderStateImpl::SyncRenderState();

        BindCurrentFBO(FramebufferTarget::Draw);

        g_GLESFuncs.glClearBufferfv(buffer, drawbuffer, value);
    }

    void ClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value) {
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        RenderStateImpl::SyncRenderState();

        g_GLESFuncs.glClearBufferiv(buffer, drawbuffer, value);
    }

    void ClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value) {
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        RenderStateImpl::SyncRenderState();

        BindCurrentFBO(FramebufferTarget::Draw);

        g_GLESFuncs.glClearBufferuiv(buffer, drawbuffer, value);
    }

    void ClearNamedFramebufferfv(const SharedPtr<MG_State::GLState::FramebufferObject>& framebuffer,
                                 GLenum buffer, GLint drawbuffer, const GLfloat* value) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        TextureImpl::SyncNeccessaryTextures();
        RenderStateImpl::SyncRenderState();

        SyncAndBindFramebufferObject(framebuffer, FramebufferTarget::Draw, true);
        g_GLESFuncs.glClearBufferfv(buffer, drawbuffer, value);
        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });

        ForceBindCurrentFBO(FramebufferTarget::Draw);
    }

    void ClearNamedFramebufferfi(const SharedPtr<MG_State::GLState::FramebufferObject>& framebuffer,
                                 GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        TextureImpl::SyncNeccessaryTextures();
        RenderStateImpl::SyncRenderState();

        SyncAndBindFramebufferObject(framebuffer, FramebufferTarget::Draw, true);
        g_GLESFuncs.glClearBufferfi(buffer, drawbuffer, depth, stencil);
        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });

        ForceBindCurrentFBO(FramebufferTarget::Draw);
    }

    class TempPixelStoreParameterSync {
    public:
        TempPixelStoreParameterSync(Bool isUnpack) : m_isUnpack(isUnpack) {
            const auto& currentParams = MG_State::pGLContext->GetPixelStoreParameters(isUnpack);
            m_prevParams = QueryCurrentGLPixelStoreParams(isUnpack);
            Sync(isUnpack, currentParams);
        }

        ~TempPixelStoreParameterSync() { Sync(m_isUnpack, m_prevParams); }

    private:
        const Bool m_isUnpack;

        PixelStoreParameters m_prevParams;

        static PixelStoreParameters QueryCurrentGLPixelStoreParams(Bool isUnpack) {
            PixelStoreParameters p;
            if (!isUnpack) {
                g_GLESFuncs.glGetIntegerv(GL_PACK_ALIGNMENT, (GLint*)&p.Alignment);
                g_GLESFuncs.glGetIntegerv(GL_PACK_ROW_LENGTH, (GLint*)&p.RowLength);
                g_GLESFuncs.glGetIntegerv(GL_PACK_SKIP_ROWS, (GLint*)&p.SkipRows);
                g_GLESFuncs.glGetIntegerv(GL_PACK_SKIP_PIXELS, (GLint*)&p.SkipPixels);
                // g_GLESFuncs.glGetIntegerv(GL_PACK_IMAGE_HEIGHT, (GLint*)&p.ImageHeight);
                // g_GLESFuncs.glGetIntegerv(GL_PACK_SKIP_IMAGES, (GLint*)&p.SkipImages);
                // GLint tmp;
                // g_GLESFuncs.glGetIntegerv(GL_PACK_SWAP_BYTES, &tmp);
                // p.SwapBytes = tmp ? true : false;
                // g_GLESFuncs.glGetIntegerv(GL_PACK_LSB_FIRST, &tmp);
                // p.LSBFirst = tmp ? true : false;
            } else {
                g_GLESFuncs.glGetIntegerv(GL_UNPACK_ALIGNMENT, (GLint*)&p.Alignment);
                g_GLESFuncs.glGetIntegerv(GL_UNPACK_ROW_LENGTH, (GLint*)&p.RowLength);
                g_GLESFuncs.glGetIntegerv(GL_UNPACK_SKIP_ROWS, (GLint*)&p.SkipRows);
                g_GLESFuncs.glGetIntegerv(GL_UNPACK_SKIP_PIXELS, (GLint*)&p.SkipPixels);
                g_GLESFuncs.glGetIntegerv(GL_UNPACK_IMAGE_HEIGHT, (GLint*)&p.ImageHeight);
                g_GLESFuncs.glGetIntegerv(GL_UNPACK_SKIP_IMAGES, (GLint*)&p.SkipImages);
                // GLint tmp;
                // g_GLESFuncs.glGetIntegerv(GL_UNPACK_SWAP_BYTES, &tmp);
                // p.SwapBytes = tmp ? true : false;
                // g_GLESFuncs.glGetIntegerv(GL_UNPACK_LSB_FIRST, &tmp);
                // p.LSBFirst = tmp ? true : false;
            }
            return p;
        }

        static void Sync(Bool isUnpack, const PixelStoreParameters& params) {
            if (!isUnpack) {
                g_GLESFuncs.glPixelStorei(GL_PACK_ALIGNMENT, params.Alignment);
                g_GLESFuncs.glPixelStorei(GL_PACK_ROW_LENGTH, params.RowLength);
                g_GLESFuncs.glPixelStorei(GL_PACK_SKIP_ROWS, params.SkipRows);
                g_GLESFuncs.glPixelStorei(GL_PACK_SKIP_PIXELS, params.SkipPixels);
                // g_GLESFuncs.glPixelStorei(GL_PACK_IMAGE_HEIGHT, params.ImageHeight);
                // g_GLESFuncs.glPixelStorei(GL_PACK_SKIP_IMAGES, params.SkipImages);
                // g_GLESFuncs.glPixelStorei(GL_PACK_SWAP_BYTES, params.SwapBytes ? GL_TRUE : GL_FALSE);
                // g_GLESFuncs.glPixelStorei(GL_PACK_LSB_FIRST, params.LSBFirst ? GL_TRUE : GL_FALSE);
            } else {
                g_GLESFuncs.glPixelStorei(GL_UNPACK_ALIGNMENT, params.Alignment);
                g_GLESFuncs.glPixelStorei(GL_UNPACK_ROW_LENGTH, params.RowLength);
                g_GLESFuncs.glPixelStorei(GL_UNPACK_SKIP_ROWS, params.SkipRows);
                g_GLESFuncs.glPixelStorei(GL_UNPACK_SKIP_PIXELS, params.SkipPixels);
                g_GLESFuncs.glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, params.ImageHeight);
                g_GLESFuncs.glPixelStorei(GL_UNPACK_SKIP_IMAGES, params.SkipImages);
                // g_GLESFuncs.glPixelStorei(GL_UNPACK_SWAP_BYTES, params.SwapBytes ? GL_TRUE : GL_FALSE);
                // g_GLESFuncs.glPixelStorei(GL_UNPACK_LSB_FIRST, params.LSBFirst ? GL_TRUE : GL_FALSE);
            }
        }
    };

    void ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels) {
        MGLOG_D("ReadPixels: x=%d y=%d w=%d h=%d format=%s type=%s pixels=%p", x, y, width, height,
                MG_Util::ConvertGLEnumToString(format).c_str(), MG_Util::ConvertGLEnumToString(type).c_str(), pixels);

        MOBILEGL_ASSERT(format == GL_RGBA || format == GL_RGBA_INTEGER,
                        "Only GL_RGBA and GL_RGBA_INTEGER are supported currently, while requested %s.",
                        MG_Util::ConvertGLEnumToString(format).c_str());
        MOBILEGL_ASSERT(type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_INT || type == GL_UNSIGNED_INT_2_10_10_10_REV ||
                            type == GL_INT || type == GL_FLOAT,
                        "Only GL_UNSIGNED_BYTE, GL_UNSIGNED_INT, GL_UNSIGNED_INT_2_10_10_10_REV, "
                        "GL_INT and GL_FLOAT are supported currently, while requested %s.",
                        MG_Util::ConvertGLEnumToString(type).c_str());

        MGLOG_D("ReadPixels: SyncNeccessaryTextures()");
        TextureImpl::SyncNeccessaryTextures();

        MGLOG_D("ReadPixels: SyncCurrentFBO()");
        FramebufferImpl::SyncCurrentFBO();

        MGLOG_D("ReadPixels: BindCurrentFBO(Read)");
        BindCurrentFBO(FramebufferTarget::Read);

        MGLOG_D("ReadPixels: Applying TempPixelStoreParameterSync (PACK)");
        TempPixelStoreParameterSync tempPackParamsSync(false);

        GLenum fbStatus = g_GLESFuncs.glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
        MGLOG_D("ReadPixels: GL_READ_FRAMEBUFFER status = %s", MG_Util::ConvertGLEnumToString(fbStatus).c_str());

        if (fbStatus != GL_FRAMEBUFFER_COMPLETE) {
            MGLOG_E("ReadPixels: bound READ FBO is not complete");
            return;
        }

        // Handle PBO
        auto& pixelPackBufferObject =
            MG_State::pGLContext->GetBufferBindingSlot(BufferTarget::PixelPack).GetBoundObject();
        Bool usePBO;
        GLuint prevPixelPackBuffer = 0;
        if (pixelPackBufferObject) {
            BufferImpl::CreateAndSyncBufferObject(pixelPackBufferObject);
            MGLOG_D("ReadPixels: Using PBO %u", pixelPackBufferObject->GetExternalIndex());
            usePBO = true;
            const auto& backendBufferIt = BufferImpl::g_backendBufferObjects.find(pixelPackBufferObject.get());

            if (backendBufferIt == BufferImpl::g_backendBufferObjects.end()) {
                MGLOG_E("ReadPixels: No backend buffer found for PBO %u.",
                        pixelPackBufferObject ? pixelPackBufferObject->GetExternalIndex() : 0);
                return;
            }
            const auto& backendBufferObject = backendBufferIt->second;
            backendBufferObject->Bind(GL_PIXEL_PACK_BUFFER);
            g_GLESFuncs.glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, (GLint*)&prevPixelPackBuffer);
        } else {
            usePBO = false;
            MGLOG_D("ReadPixels: Not using PBO");
        }

        MGLOG_D("ReadPixels: glReadPixels()");
        g_GLESFuncs.glReadPixels(x, y, width, height, format, type, pixels);
        if (usePBO) {
            // pull back to client memory if PBO is used
            MGLOG_D("ReadPixels: PBO used, mapping buffer to client memory");
            GLvoid* pboMappedPtr = g_GLESFuncs.glMapBufferRange(
                GL_PIXEL_PACK_BUFFER, 0, (GLsizeiptr)pixelPackBufferObject->GetSize(), GL_MAP_READ_BIT);
            if (pboMappedPtr) {
                MGLOG_D("ReadPixels: Copying data from PBO to client memory");
                SizeT size = pixelPackBufferObject->GetSize();
                pixelPackBufferObject->UploadSubData({pboMappedPtr, size}, 0);
                pixelPackBufferObject->ClearDirty();
                MGLOG_D("ReadPixels: Unmapping PBO");
                g_GLESFuncs.glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            } else {
                MGLOG_E("ReadPixels: glMapBufferRange returned nullptr");
                MGLOG_E("ReadPixels: glMapBufferRange returned nullptr");
            }
            MGLOG_D("ReadPixels: Restoring previous pixel pack buffer binding %u", prevPixelPackBuffer);
            g_GLESFuncs.glBindBuffer(GL_PIXEL_PACK_BUFFER, prevPixelPackBuffer);
        }
        MGLOG_D("ReadPixels: finished");
    }

    void GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void* pixels) {
        DebugImpl::ErrorLopper errorLopper;
        MGLOG_D("GetTexImage: target=%s level=%d format=%s type=%s pixels=%p",
                MG_Util::ConvertGLEnumToString(target).c_str(), level, MG_Util::ConvertGLEnumToString(format).c_str(),
                MG_Util::ConvertGLEnumToString(type).c_str(), pixels);

        MOBILEGL_ASSERT(format == GL_RGBA || format == GL_RGBA_INTEGER || format == GL_BGRA,
                        "Only GL_RGBA, GL_RGBA_INTEGER and GL_BGRA are supported currently, while requested %s.",
                        MG_Util::ConvertGLEnumToString(format).c_str());
        MOBILEGL_ASSERT(type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_INT || type == GL_UNSIGNED_INT_2_10_10_10_REV ||
                            type == GL_INT || type == GL_FLOAT || type == GL_UNSIGNED_INT_8_8_8_8 ||
                            type == GL_UNSIGNED_INT_8_8_8_8_REV,
                        "Only GL_UNSIGNED_BYTE, GL_UNSIGNED_INT, GL_UNSIGNED_INT_2_10_10_10_REV, "
                        "GL_INT, GL_FLOAT, GL_UNSIGNED_INT_8_8_8_8 and GL_UNSIGNED_INT_8_8_8_8_REV "
                        "are supported currently, while requested %s.",
                        MG_Util::ConvertGLEnumToString(type).c_str());

        GLenum esFormat = format, esType = type;
        if (esFormat == GL_BGRA) esFormat = GL_RGBA;
        if (esType == GL_UNSIGNED_INT_8_8_8_8 || esType == GL_UNSIGNED_INT_8_8_8_8_REV) esType = GL_UNSIGNED_BYTE;

        MGLOG_D("GetTexImage: SyncNeccessaryTextures()");
        TextureImpl::SyncNeccessaryTextures();

        MGLOG_D("GetTexImage: SyncCurrentFBO()");
        FramebufferImpl::SyncCurrentFBO();

        auto activeTextureUnit = MG_State::pGLContext->GetActiveTextureUnit();
        MGLOG_D("GetTexImage: active texture unit = %u", activeTextureUnit);

        const auto& textureObject = MG_State::pGLContext->GetTextureUnitObject(activeTextureUnit)
                                        .GetBindingSlot(MG_Util::ConvertGLEnumToTextureTarget(target))
                                        .GetBoundObject();

        MGLOG_D("GetTexImage: bound texture object = %p (name=%u)", textureObject.get(),
                textureObject ? textureObject->GetExternalIndex() : 0);

        const auto& backendTextureIt = TextureImpl::g_backendTextureObjects.find(textureObject.get());

        if (backendTextureIt == TextureImpl::g_backendTextureObjects.end()) {
            MGLOG_E("GetTexImage: No backend texture found for texture %u.",
                    textureObject ? textureObject->GetExternalIndex() : 0);
            return;
        }

        GLuint backendTexId = backendTextureIt->second->GetBackendTextureId();
        MGLOG_D("GetTexImage: backend texture id = %u", backendTexId);

        MGLOG_D("GetTexImage: Binding temporary FBO");
        TempFBOBinder tempFBOBinder(true);

        MGLOG_D("GetTexImage: glFramebufferTexture2D(level=%d)", level);
        g_GLESFuncs.glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, backendTexId, level);
        MGLOG_D("GetTexImage: glReadBuffer(GL_COLOR_ATTACHMENT0)");
        g_GLESFuncs.glReadBuffer(GL_COLOR_ATTACHMENT0);

        GLenum fbStatus = g_GLESFuncs.glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
        MGLOG_D("GetTexImage: GL_READ_FRAMEBUFFER status = %s", MG_Util::ConvertGLEnumToString(fbStatus).c_str());

        if (fbStatus != GL_FRAMEBUFFER_COMPLETE) {
            MGLOG_E("GetTexImage: READ FBO incomplete");
            MGLOG_E("GetTexImage: bound READ FBO is not complete");
            return;
        }

        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });

        MGLOG_D("GetTexImage: Applying TempPixelStoreParameterSync (PACK)");
        TempPixelStoreParameterSync tempPackParamsSync(false);

        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });

        const auto& storageType = textureObject->GetStorageType();
        MGLOG_D("GetTexImage: texture storage type = %d", (int)storageType);

        if (storageType == TextureStorageType::Buffer) {
            MGLOG_E("GetTexImage: Texture storage type Buffer is not supported.");
            return;
        }

        auto* textureMipmapObject = static_cast<MG_State::GLState::TextureObjectMipmap*>(textureObject.get());

        auto& levelRange = textureMipmapObject->GetLevelRange();
        MGLOG_D("GetTexImage: mipmap level range = [%d, %d)", levelRange.x(), levelRange.y());

        if (level < levelRange.x() || level >= levelRange.y()) {
            MGLOG_E("GetTexImage: Requested level %d out of range", level);
            MOBILEGL_ASSERT(false,
                            "GetTexImage: Requested level %d is out of range "
                            "(base level %d, max level %d).",
                            level, levelRange.x(), levelRange.y());
            return;
        }

        auto size = textureMipmapObject->GetMipmapTexelSize(MG_Util::ConvertGLEnumToTextureUploadTarget(target), level);

        MGLOG_D("GetTexImage: mip level %d size = %dx%d", level, size.x(), size.y());

        // Handle PBO
        auto& pixelPackBufferObject =
            MG_State::pGLContext->GetBufferBindingSlot(BufferTarget::PixelPack).GetBoundObject();
        Bool usePBO;
        GLuint prevPixelPackBuffer = 0;
        if (pixelPackBufferObject) {
            BufferImpl::CreateAndSyncBufferObject(pixelPackBufferObject);
            MGLOG_D("GetTexImage: Using PBO %u", pixelPackBufferObject->GetExternalIndex());
            usePBO = true;
            const auto& backendBufferIt = BufferImpl::g_backendBufferObjects.find(pixelPackBufferObject.get());
            if (backendBufferIt == BufferImpl::g_backendBufferObjects.end()) {
                MGLOG_E("GetTexImage: No backend buffer found for PBO %u.",
                        pixelPackBufferObject ? pixelPackBufferObject->GetExternalIndex() : 0);
                return;
            }
            const auto& backendBufferObject = backendBufferIt->second;
            backendBufferObject->Bind(GL_PIXEL_PACK_BUFFER);
            g_GLESFuncs.glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, (GLint*)&prevPixelPackBuffer);
        } else {
            usePBO = false;
            MGLOG_D("GetTexImage: Not using PBO");
        }

        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        MGLOG_D("GetTexImage: glReadPixels(0, 0, %d, %d, %s, %s, %p)", size.x(), size.y(),
                MG_Util::ConvertGLEnumToString(esFormat).c_str(), MG_Util::ConvertGLEnumToString(esType).c_str(),
                pixels);
        g_GLESFuncs.glReadPixels(0, 0, size.x(), size.y(), esFormat, esType, pixels);

        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        if (usePBO) {
            // pull back to client memory if PBO is used
            MGLOG_D("ReadPixels: PBO used, mapping buffer to client memory");
            GLvoid* pboMappedPtr = g_GLESFuncs.glMapBufferRange(
                GL_PIXEL_PACK_BUFFER, 0, (GLsizeiptr)pixelPackBufferObject->GetSize(), GL_MAP_READ_BIT);
            if (pboMappedPtr) {
                MGLOG_D("ReadPixels: Copying data from PBO to client memory");
                SizeT size = pixelPackBufferObject->GetSize();
                pixelPackBufferObject->UploadSubData({pboMappedPtr, size}, 0);
                pixelPackBufferObject->ClearDirty();
                MGLOG_D("ReadPixels: Unmapping PBO");
                g_GLESFuncs.glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            } else {
                MGLOG_E("ReadPixels: glMapBufferRange returned nullptr");
            }
            MGLOG_D("ReadPixels: Restoring previous pixel pack buffer binding %u", prevPixelPackBuffer);

            g_GLESFuncs.glBindBuffer(GL_PIXEL_PACK_BUFFER, prevPixelPackBuffer);
        } else {
            if (esFormat == GL_RGBA && format == GL_BGRA && esType == GL_UNSIGNED_BYTE &&
                type == GL_UNSIGNED_INT_8_8_8_8_REV) {
                MGLOG_D("ReadPixels: ProcessColorSwizzle BGRA (not implemented)");
            }
        }

        DebugImpl::ErrorLopper::Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        MGLOG_D("GetTexImage: finished");
    }

    void SetEGLFuncsTable(const MG_External::EGLFunctionsTable& eglFuncs) {
        g_EGLFuncs = eglFuncs;
    }

    void SetGLESFuncsTable(const MG_External::GLESFunctionsTable& glesFuncs) {
        g_GLESFuncs = glesFuncs;
    }

    void SetGLESCapabilities(const MG_External::GLESCapabilities& capabilities) {
        g_GLESCapabilities = capabilities;
    }

    static EGLDisplay g_Display = EGL_NO_DISPLAY;
    static EGLContext g_Context = EGL_NO_CONTEXT;
    static EGLSurface g_Surface = EGL_NO_SURFACE;
    static EGLConfig g_Config = nullptr;

    static Bool QueryCurrentSurfaceSize(Int& outWidth, Int& outHeight) {
        outWidth = 0;
        outHeight = 0;
        if (!g_EGLFuncs.eglQuerySurface || g_Display == EGL_NO_DISPLAY || g_Surface == EGL_NO_SURFACE) {
            return false;
        }

        EGLint width = 0;
        EGLint height = 0;
        if (!g_EGLFuncs.eglQuerySurface(g_Display, g_Surface, EGL_WIDTH, &width) ||
            !g_EGLFuncs.eglQuerySurface(g_Display, g_Surface, EGL_HEIGHT, &height) ||
            width <= 0 || height <= 0) {
            return false;
        }

        outWidth = static_cast<Int>(width);
        outHeight = static_cast<Int>(height);
        return true;
    }

    static Bool PresentStatsEnabled() {
        static const Bool enabled = [] {
            const char* value = std::getenv("MOBILEGL_GLES_PRESENT_STATS");
            return value != nullptr && value[0] != '\0' && std::strcmp(value, "0") != 0;
        }();
        return enabled;
    }

    static void DumpDefaultFramebufferStats() {
        if (!PresentStatsEnabled() || !g_GLESFuncs.glReadPixels || !g_EGLFuncs.eglQuerySurface ||
            g_Display == EGL_NO_DISPLAY || g_Surface == EGL_NO_SURFACE) {
            return;
        }

        Int width = 0;
        Int height = 0;
        if (!QueryCurrentSurfaceSize(width, height)) {
            return;
        }

        GLint viewport[4] = {0, 0, 0, 0};
        g_GLESFuncs.glGetIntegerv(GL_VIEWPORT, viewport);
        GLint previousReadFramebuffer = 0;
        g_GLESFuncs.glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previousReadFramebuffer);
        g_GLESFuncs.glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

        Vector<Uint8> pixels(static_cast<SizeT>(width) * static_cast<SizeT>(height) * 4);
        g_GLESFuncs.glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

        SizeT nonBlack = 0;
        SizeT nonZeroAlpha = 0;
        for (SizeT offset = 0; offset + 3 < pixels.size(); offset += 4) {
            if (pixels[offset] != 0 || pixels[offset + 1] != 0 || pixels[offset + 2] != 0) {
                ++nonBlack;
            }
            if (pixels[offset + 3] != 0) {
                ++nonZeroAlpha;
            }
        }

        g_GLESFuncs.glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(previousReadFramebuffer));
        std::fprintf(stderr,
                     "MOBILEGL_GLES_PRESENT_STATS nonBlack=%zu/%zu alpha=%zu/%zu size=%dx%d viewport=%d,%d,%d,%d\n",
                     nonBlack, pixels.size() / 4, nonZeroAlpha, pixels.size() / 4, width, height,
                     viewport[0], viewport[1], viewport[2], viewport[3]);
    }

#if defined(__linux__) && !defined(__ANDROID__)
    static void* OpenX11Lib() {
        void* x11Lib = dlopen("libX11.so.6", RTLD_LOCAL | RTLD_NOW);
        if (!x11Lib) {
            x11Lib = dlopen("libX11.so", RTLD_LOCAL | RTLD_NOW);
        }
        return x11Lib;
    }
#endif

    static EGLint QueryDefaultX11VisualId() {
#if defined(__linux__) && !defined(__ANDROID__)
        const char* displayName = std::getenv("DISPLAY");
        if (!displayName) {
            return 0;
        }

        void* x11Lib = OpenX11Lib();
        if (!x11Lib) {
            return 0;
        }

        using XOpenDisplayFn = void* (*)(const char*);
        using XDefaultScreenFn = int (*)(void*);
        using XDefaultVisualFn = void* (*)(void*, int);
        using XVisualIDFromVisualFn = unsigned long (*)(void*);
        using XCloseDisplayFn = int (*)(void*);

        auto* xOpenDisplay = reinterpret_cast<XOpenDisplayFn>(dlsym(x11Lib, "XOpenDisplay"));
        auto* xDefaultScreen = reinterpret_cast<XDefaultScreenFn>(dlsym(x11Lib, "XDefaultScreen"));
        auto* xDefaultVisual = reinterpret_cast<XDefaultVisualFn>(dlsym(x11Lib, "XDefaultVisual"));
        auto* xVisualIDFromVisual = reinterpret_cast<XVisualIDFromVisualFn>(dlsym(x11Lib, "XVisualIDFromVisual"));
        auto* xCloseDisplay = reinterpret_cast<XCloseDisplayFn>(dlsym(x11Lib, "XCloseDisplay"));
        if (!xOpenDisplay || !xDefaultScreen || !xDefaultVisual || !xVisualIDFromVisual || !xCloseDisplay) {
            dlclose(x11Lib);
            return 0;
        }

        void* display = xOpenDisplay(displayName);
        if (!display) {
            dlclose(x11Lib);
            return 0;
        }
        const int screen = xDefaultScreen(display);
        void* visual = xDefaultVisual(display, screen);
        const auto visualId = visual ? static_cast<EGLint>(xVisualIDFromVisual(visual)) : 0;
        xCloseDisplay(display);
        dlclose(x11Lib);
        return visualId;
#else
        return 0;
#endif
    }

    static EGLint QueryX11WindowVisualId(NativeWindowType window) {
#if defined(__linux__) && !defined(__ANDROID__) && __has_include(<X11/Xlib.h>)
        if (!window) {
            return 0;
        }
        const char* displayName = std::getenv("DISPLAY");
        if (!displayName) {
            return 0;
        }

        void* x11Lib = OpenX11Lib();
        if (!x11Lib) {
            return 0;
        }

        using XOpenDisplayFn = Display* (*)(const char*);
        using XGetWindowAttributesFn = int (*)(Display*, Window, XWindowAttributes*);
        using XVisualIDFromVisualFn = unsigned long (*)(Visual*);
        using XCloseDisplayFn = int (*)(Display*);

        auto* xOpenDisplay = reinterpret_cast<XOpenDisplayFn>(dlsym(x11Lib, "XOpenDisplay"));
        auto* xGetWindowAttributes =
            reinterpret_cast<XGetWindowAttributesFn>(dlsym(x11Lib, "XGetWindowAttributes"));
        auto* xVisualIDFromVisual = reinterpret_cast<XVisualIDFromVisualFn>(dlsym(x11Lib, "XVisualIDFromVisual"));
        auto* xCloseDisplay = reinterpret_cast<XCloseDisplayFn>(dlsym(x11Lib, "XCloseDisplay"));
        if (!xOpenDisplay || !xGetWindowAttributes || !xVisualIDFromVisual || !xCloseDisplay) {
            dlclose(x11Lib);
            return 0;
        }

        Display* display = xOpenDisplay(displayName);
        if (!display) {
            dlclose(x11Lib);
            return 0;
        }

        XWindowAttributes attrs{};
        EGLint visualId = 0;
        if (xGetWindowAttributes(display, static_cast<Window>(window), &attrs) && attrs.visual) {
            visualId = static_cast<EGLint>(xVisualIDFromVisual(attrs.visual));
        }
        xCloseDisplay(display);
        dlclose(x11Lib);
        return visualId;
#else
        (void)window;
        return 0;
#endif
    }

    static Bool GetConfigAttrib(EGLConfig config, EGLint attr, EGLint& value) {
        return g_EGLFuncs.eglGetConfigAttrib && g_EGLFuncs.eglGetConfigAttrib(g_Display, config, attr, &value);
    }

    static Bool ConfigSupports(EGLConfig config, EGLint surfaceBit) {
        EGLint surfaceType = 0;
        EGLint renderableType = 0;
        if (!GetConfigAttrib(config, EGL_SURFACE_TYPE, surfaceType)) {
            return false;
        }
        if (!GetConfigAttrib(config, EGL_RENDERABLE_TYPE, renderableType)) {
            return false;
        }
        return (surfaceType & surfaceBit) && (renderableType & EGL_OPENGL_ES3_BIT);
    }

    static Bool ChooseConfigForSurface(EGLint surfaceBit, EGLConfig& outConfig,
                                       NativeWindowType window = static_cast<NativeWindowType>(0)) {
        const EGLint configAttribs[] = {EGL_SURFACE_TYPE, surfaceBit, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
                                        EGL_RED_SIZE,     8,          EGL_GREEN_SIZE,      8,
                                        EGL_BLUE_SIZE,    8,          EGL_ALPHA_SIZE,      8,
                                        EGL_DEPTH_SIZE,   24,         EGL_STENCIL_SIZE,    8,
                                        EGL_NONE};

        EGLint numConfigs = 0;
        if (!g_EGLFuncs.eglChooseConfig(g_Display, configAttribs, nullptr, 0, &numConfigs) || numConfigs == 0) {
            return false;
        }

        Vector<EGLConfig> configs(static_cast<SizeT>(numConfigs));
        if (!g_EGLFuncs.eglChooseConfig(g_Display, configAttribs, configs.data(), numConfigs, &numConfigs) ||
            numConfigs == 0) {
            return false;
        }
        configs.resize(static_cast<SizeT>(numConfigs));

        if (surfaceBit == EGL_WINDOW_BIT) {
            const EGLint windowVisualId = QueryX11WindowVisualId(window);
            const EGLint visualIds[] = {windowVisualId, QueryDefaultX11VisualId()};
            for (const auto visualId : visualIds) {
                if (visualId == 0) {
                    continue;
                }
                for (const auto config : configs) {
                    EGLint nativeVisualId = 0;
                    if (ConfigSupports(config, surfaceBit) &&
                        GetConfigAttrib(config, EGL_NATIVE_VISUAL_ID, nativeVisualId) &&
                        nativeVisualId == visualId) {
                        outConfig = config;
                        return true;
                    }
                }
            }
        }

        for (const auto config : configs) {
            if (ConfigSupports(config, surfaceBit)) {
                outConfig = config;
                return true;
            }
        }

        outConfig = configs.front();
        return true;
    }

    static Bool InitDisplayAndContext(EGLint surfaceBit, NativeWindowType window = static_cast<NativeWindowType>(0)) {
        DestroyEGLContext();

        g_Display = g_EGLFuncs.eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (g_Display == EGL_NO_DISPLAY) return false;

        if (!g_EGLFuncs.eglInitialize(g_Display, nullptr, nullptr)) return false;
        g_EGLFuncs.eglBindAPI(EGL_OPENGL_ES_API);

        if (!ChooseConfigForSurface(surfaceBit, g_Config, window)) return false;

        const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};

        g_Context = g_EGLFuncs.eglCreateContext(g_Display, g_Config, EGL_NO_CONTEXT, contextAttribs);
        return g_Context != EGL_NO_CONTEXT;
    }

    Bool InitWindowSurface(NativeWindowType window) {
        if (!window) return false;

        if (!InitDisplayAndContext(EGL_WINDOW_BIT, window)) return false;

        g_Surface = g_EGLFuncs.eglCreateWindowSurface(g_Display, g_Config, window, nullptr);
        if (g_Surface == EGL_NO_SURFACE) return false;

        if (!g_EGLFuncs.eglMakeCurrent(g_Display, g_Surface, g_Surface, g_Context)) return false;

        MGLOG_D("EGL context created successfully: display=%p, surface=%p, context=%p. window=%p", g_Display, g_Surface,
                g_Context, window);
        return true;
    }

    Bool InitPbufferSurface(EGLint width, EGLint height) {
        if (width <= 0 || height <= 0) return false;
        if (!InitDisplayAndContext(EGL_PBUFFER_BIT)) return false;

        const EGLint surfaceAttribs[] = {EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE};
        g_Surface = g_EGLFuncs.eglCreatePbufferSurface(g_Display, g_Config, surfaceAttribs);
        if (g_Surface == EGL_NO_SURFACE) return false;

        if (!g_EGLFuncs.eglMakeCurrent(g_Display, g_Surface, g_Surface, g_Context)) return false;

        MGLOG_D("EGL pbuffer context created successfully: display=%p, surface=%p, context=%p. size=%dx%d", g_Display,
                g_Surface, g_Context, width, height);
        return true;
    }

    void Present() {
        if (g_Display != EGL_NO_DISPLAY && g_Surface != EGL_NO_SURFACE) {
            DumpDefaultFramebufferStats();
            if (g_GLESFuncs.glFlush) {
                g_GLESFuncs.glFlush();
            }
            g_EGLFuncs.eglSwapBuffers(g_Display, g_Surface);
        }
    }

    void DestroyEGLContext() {
        if (g_Display != EGL_NO_DISPLAY) {
            g_EGLFuncs.eglMakeCurrent(g_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (g_Context != EGL_NO_CONTEXT) {
                g_EGLFuncs.eglDestroyContext(g_Display, g_Context);
                g_Context = EGL_NO_CONTEXT;
            }
            if (g_Surface != EGL_NO_SURFACE) {
                g_EGLFuncs.eglDestroySurface(g_Display, g_Surface);
                g_Surface = EGL_NO_SURFACE;
            }
            g_EGLFuncs.eglTerminate(g_Display);
            g_Display = EGL_NO_DISPLAY;
        }
    }

} // namespace MobileGL::MG_Backend::DirectGLES
