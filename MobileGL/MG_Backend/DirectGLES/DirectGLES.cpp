// MobileGL - MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "DirectGLES.h"
#include "MG_State/GLState/SamplerState/SamplerObject.h"
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

namespace MobileGL::MG_Backend::DirectGLES {

    namespace DebugImpl {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG
        void ErrorLopper::Loop(std::function<void(GLenum)> func) {
            GLenum err = MG_External::GLES::glGetError();
            while (err != GL_NO_ERROR) {
                func(err);
                err = MG_External::GLES::glGetError();
            }
        }

        void ErrorLopper::Clear() {
            GLenum err = MG_External::GLES::glGetError();
            while (err != GL_NO_ERROR) {
                MGLOG_D("Stray GL Error cleared: %s", MG_Util::ConvertGLEnumToString(err).c_str());
                err = MG_External::GLES::glGetError();
            }
        }

        ErrorLopper::ErrorLopper() {
            Clear();
        }
        ErrorLopper::~ErrorLopper() {
            Clear();
        }
#else
        void ErrorLopper::Loop(std::function<void(GLenum)> func) {}
        void ErrorLopper::Clear() {}
        ErrorLopper::ErrorLopper() {}
        ErrorLopper::~ErrorLopper() {}
#endif

#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG
        OpenGLScopeMarker::OpenGLScopeMarker(String scopeName) {
            MG_External::GLES::glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, scopeName.c_str());
        }

        OpenGLScopeMarker::~OpenGLScopeMarker() {
            MG_External::GLES::glPopDebugGroup();
        }
#else
        OpenGLScopeMarker::OpenGLScopeMarker(String scopeName) {}

        OpenGLScopeMarker::~OpenGLScopeMarker() {}
#endif
    } // namespace DebugImpl

    // TODO: deletion for deleted objects

    namespace BufferImpl {
        void CreateAndSyncBufferObject(SharedPtr<MG_State::GLState::BufferObject>& bufferObject) {
            if (!(bufferObject->GetChangeBits() & BufferChangeBits::DirtyBit)) return;

            const auto& backendBufferIt = g_backendBufferObjects.find(bufferObject);
            SharedPtr<BackendBufferObject> backendBufferObject;
            if (backendBufferIt == g_backendBufferObjects.end()) {
                backendBufferObject = MakeShared<BackendBufferObject>();
                g_backendBufferObjects[bufferObject] = backendBufferObject;
            } else {
                backendBufferObject = backendBufferIt->second;
            }
            backendBufferObject->SyncToBackend(bufferObject);
        }

        void SyncNeccessaryBuffers(Bool includeIBO = false, Bool includeIndirectBuffer = false) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            // All buffers we need are:
            //   1.VBO 2.IBO (if needed) 3.UBO 4.IndirectBuffer (if needed) 5.SSBO (TODO)
            // PBO is not needed since it should be handled in frontend

            // static Vector<SharedPtr<MG_State::GLState::BufferObject>> buffersToSync;
            // buffersToSync.clear();

            const auto& currentVAOObject = MG_State::pGLContext->GetBoundVertexArray();
            if (!currentVAOObject) {
                MGLOG_E("No VAO is currently bound, cannot sync necessary buffers.");
                return;
            }

            // VBO
            for (const auto& attrib : currentVAOObject->GetAllAttributes()) {
                if (!attrib.Enabled) continue;
                auto bufferObject = attrib.Buffer;
                if (bufferObject) {
                    CreateAndSyncBufferObject(bufferObject);
                }
            }

            // IBO
            if (includeIBO) {
                auto possibleIBO = currentVAOObject->GetIndexBufferBindingSlot().GetBoundObject();
                if (possibleIBO) {
                    CreateAndSyncBufferObject(possibleIBO);
                }
            }

            // Indirect Buffer Object
            if (includeIndirectBuffer) {
                auto possibleIndirectBuffer =
                    MG_State::pGLContext->GetBufferBindingSlot(BufferTarget::DrawIndirect).GetBoundObject();
                if (possibleIndirectBuffer) {
                    CreateAndSyncBufferObject(possibleIndirectBuffer);
                }
            }

            // UBO
            auto uboBindingPointCnt = MG_State::pGLContext->GetBufferBindingPointCount(BufferTarget::Uniform);
            for (SizeT i = 0; i < uboBindingPointCnt; ++i) {
                auto& point = MG_State::pGLContext->GetBufferBindingPoint(BufferTarget::Uniform, i);
                auto obj = point.GetBoundObject();
                if (obj) {
                    CreateAndSyncBufferObject(obj);
                }
            }
        }
    } // namespace BufferImpl

    namespace VertexArrayImpl {
        void SyncCurrentVAO() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            auto currentVAOObject = MG_State::pGLContext->GetBoundVertexArray();
            if (!currentVAOObject) {
                MGLOG_E("No VAO is currently bound, cannot sync current VAO.");
                return;
            }

            const auto& backendVAOIt = g_backendVertexArrayObjects.find(currentVAOObject);
            SharedPtr<VertexArrayImpl::BackendVertexArrayObject> backendVAOObject;
            if (backendVAOIt == g_backendVertexArrayObjects.end()) {
                backendVAOObject = MakeShared<VertexArrayImpl::BackendVertexArrayObject>();
                g_backendVertexArrayObjects[currentVAOObject] = backendVAOObject;
            } else {
                backendVAOObject = backendVAOIt->second;
            }
            backendVAOObject->SyncToBackend(currentVAOObject);
        }
    } // namespace VertexArrayImpl

    namespace TextureImpl {
        SharedPtr<BackendTextureObject> SyncTextureObjectToBackend(
            SharedPtr<MG_State::GLState::ITextureObject>& textureObject) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            const auto& backendTextureIt = g_backendTextureObjects.find(textureObject);
            SharedPtr<BackendTextureObject> backendTextureObject;
            if (backendTextureIt == g_backendTextureObjects.end()) {
                backendTextureObject = MakeShared<BackendTextureObject>();
                g_backendTextureObjects[textureObject] = backendTextureObject;
            } else {
                backendTextureObject = backendTextureIt->second;
            }
            backendTextureObject->SyncTextureParamsToBackend(textureObject);
            backendTextureObject->SyncBuiltinSamplerToBackend(textureObject);
            backendTextureObject->SyncMipmapsToBackend(textureObject);
            return backendTextureObject;
        }

        void SyncNeccessaryTextures() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            // All textures we need are:
            //   1. textures bound to texture units (TODO: only sync ones that are used in current program)
            //   2. textures used in current FBO
            //   3. textures bound to image units (TODO)

            for (int index = 0; index < MG_State::GLState::TextureState::MAX_TEXTURE_IMAGE_UNITS; ++index) {
                auto& unit = MG_State::pGLContext->GetTextureUnitObject(index);
                for (const auto& bindingSlot : unit.GetAllBindingSlots()) {
                    auto textureObject = bindingSlot.GetBoundObject();
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
                    auto textureObject = attachment.GetTexture();
                    if (textureObject) {
                        SyncTextureObjectToBackend(textureObject);
                    }
                }
            }
        }
    } // namespace TextureImpl

    namespace FramebufferImpl {
        void SyncCurrentFBO() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            const FramebufferTarget fboTargets[] = {FramebufferTarget::Draw, FramebufferTarget::Read};

            MG_State::GLState::FramebufferObject* lastUpdatedFBO = nullptr;

            for (auto target : fboTargets) {
                auto slot = MG_State::pGLContext->GetFramebufferBindingSlot(target);
                auto version = slot.GetVersion();
                if (version == g_fboBindVersions[SizeT(target)]) continue;

                auto currentFBO = slot.GetBoundObject();

                if (!currentFBO) {
                    MGLOG_E("No FBO is currently bound, cannot sync current FBO.");
                    continue;
                }

                if (currentFBO == MG_Impl::GLImpl::FramebufferImpl::pDefaultFramebufferInfo->defaultFBO) {
                    // Default FBO, nothing to sync
                    continue;
                }

                const auto& backendFBOIt = g_backendFramebufferObjects.find(currentFBO);
                SharedPtr<BackendFramebufferObject> backendFBOObject;
                if (backendFBOIt == g_backendFramebufferObjects.end()) {
                    backendFBOObject = MakeShared<BackendFramebufferObject>();
                    g_backendFramebufferObjects[currentFBO] = backendFBOObject;
                } else {
                    backendFBOObject = backendFBOIt->second;
                }

                if (currentFBO.get() == lastUpdatedFBO) {
                    MGLOG_D("Draw FBO and read FBO are the same, skipping sync.");
                } else {
                    backendFBOObject->SyncToBackend(currentFBO, target);
                }

                //                backendFBOObject->Bind(target);

                lastUpdatedFBO = currentFBO.get();
            }
        }
    } // namespace FramebufferImpl

    namespace RenderStateImpl {
        static Uint16 g_syncedRenderStateVersion = 0;
        static RenderStateParameters g_syncedRenderStateParameters;
        void SyncRenderState() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            Uint16 currentRenderStateVersion = MG_State::pGLContext->GetRenderStateParametersVersion();
            if (currentRenderStateVersion == g_syncedRenderStateVersion) return;

            const auto& parameters = MG_State::pGLContext->GetRenderStateParameters();

            if (parameters.Viewport != g_syncedRenderStateParameters.Viewport) {
                MG_External::GLES::glViewport(parameters.Viewport.x(), parameters.Viewport.y(), parameters.Viewport.z(),
                                              parameters.Viewport.w());
            }

#define SYNC_CAPABILITY(cap_mg, cap_gl)                                                                                \
    if (parameters.cap_mg##Enabled != g_syncedRenderStateParameters.cap_mg##Enabled) {                                 \
        if (parameters.cap_mg##Enabled) {                                                                              \
            MG_External::GLES::glEnable(cap_gl);                                                                       \
        } else {                                                                                                       \
            MG_External::GLES::glDisable(cap_gl);                                                                      \
        }                                                                                                              \
    }
            SYNC_CAPABILITY(Blend, GL_BLEND);
            SYNC_CAPABILITY(DepthTest, GL_DEPTH_TEST);
            SYNC_CAPABILITY(ScissorTest, GL_SCISSOR_TEST);
            SYNC_CAPABILITY(CullFace, GL_CULL_FACE);

#undef SYNC_CAPABILITY

            const auto& ToGLBoolean = [](Bool b) -> GLboolean { return b ? GL_TRUE : GL_FALSE; };

            if (parameters.SrcFactorRGB != g_syncedRenderStateParameters.SrcFactorRGB ||
                parameters.DstFactorRGB != g_syncedRenderStateParameters.DstFactorRGB ||
                parameters.SrcFactorAlpha != g_syncedRenderStateParameters.SrcFactorAlpha ||
                parameters.DstFactorAlpha != g_syncedRenderStateParameters.DstFactorAlpha) { // Blend func
                const BlendFactor &srcRGB = parameters.SrcFactorRGB, &dstRGB = parameters.DstFactorRGB,
                                  &srcAlpha = parameters.SrcFactorAlpha, &dstAlpha = parameters.DstFactorAlpha;

                MG_External::GLES::glBlendFuncSeparate(
                    MG_Util::ConvertBlendFactorToGLEnum(srcRGB), MG_Util::ConvertBlendFactorToGLEnum(dstRGB),
                    MG_Util::ConvertBlendFactorToGLEnum(srcAlpha), MG_Util::ConvertBlendFactorToGLEnum(dstAlpha));
            }

            { // Blend equation
                if (parameters.DepthFunc != g_syncedRenderStateParameters.DepthFunc) {
                    MG_External::GLES::glDepthFunc(MG_Util::ConvertDepthTestFuncToGLEnum(parameters.DepthFunc));
                }
                if (parameters.DepthMask != g_syncedRenderStateParameters.DepthMask) {
                    MG_External::GLES::glDepthMask(parameters.DepthMask ? GL_TRUE : GL_FALSE);
                }
            }

            { // Color mask
                if (parameters.ColorMask != g_syncedRenderStateParameters.ColorMask) {
                    const BoolVec4& colorMask = parameters.ColorMask;
                    MG_External::GLES::glColorMask(ToGLBoolean(colorMask.x()), ToGLBoolean(colorMask.y()),
                                                   ToGLBoolean(colorMask.z()), ToGLBoolean(colorMask.w()));
                }
            }

            { // Clear values
                if (parameters.ClearColor != g_syncedRenderStateParameters.ClearColor) {
                    const FloatVec4& clearCol = parameters.ClearColor;
                    MG_External::GLES::glClearColor(clearCol.x(), clearCol.y(), clearCol.z(), clearCol.w());
                }
                if (parameters.ClearDepth != g_syncedRenderStateParameters.ClearDepth) {
                    MG_External::GLES::glClearDepthf(parameters.ClearDepth);
                }
            }

            { // Cull face mode
                if (parameters.CullFaceModeSetting != g_syncedRenderStateParameters.CullFaceModeSetting) {
                    const CullFaceMode& cfm = parameters.CullFaceModeSetting;
                    MG_External::GLES::glCullFace(MG_Util::ConvertCullFaceModeToGLEnum(cfm));
                }
            }

            { // Scissor box
                if (parameters.ScissorBox != g_syncedRenderStateParameters.ScissorBox) {
                    const IntVec4& scissorBox = parameters.ScissorBox;
                    MG_External::GLES::glScissor(scissorBox.x(), scissorBox.y(), scissorBox.z(), scissorBox.w());
                }
            }

            g_syncedRenderStateVersion = currentRenderStateVersion;
            g_syncedRenderStateParameters = parameters;
        }
    } // namespace RenderStateImpl

    namespace PrgramImpl {
        void SyncCurrentProgram() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            auto currentProgram = MG_State::pGLContext->GetCurrentProgram();
            if (!currentProgram || !currentProgram->GetLinkStatus()) {
                MG_External::GLES::glUseProgram(0);
                return;
            }
            const auto& backendProgramIt = g_backendProgramObjects.find(currentProgram);
            SharedPtr<BackendProgramObjectImpl> backendProgram;
            if (backendProgramIt == g_backendProgramObjects.end()) {
                backendProgram = MakeShared<BackendProgramObjectImpl>();
                g_backendProgramObjects[currentProgram] = backendProgram;
                backendProgram->SyncToBackend(currentProgram);
            } else {
                backendProgram = backendProgramIt->second;
                if (!backendProgram->GetBackendProgramId()) {
                    backendProgram->SyncToBackend(currentProgram);
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
            const auto& backendFBOIt = FramebufferImpl::g_backendFramebufferObjects.find(currentFBO);
            if (backendFBOIt != FramebufferImpl::g_backendFramebufferObjects.end()) {
                backendFBOIt->second->Bind(target);
            } else {
                MGLOG_E("No backend FBO found (maybe not synced) for current %s FBO, cannot bind FBO.",
                        (target == FramebufferTarget::Read ? "READ" : "DRAW"));
            }
        } else {
            MGLOG_D("Binding default framebuffer as %s FBO", (target == FramebufferTarget::Read ? "READ" : "DRAW"));
            MG_External::GLES::glBindFramebuffer(
                target == FramebufferTarget::Draw ? GL_DRAW_FRAMEBUFFER : GL_READ_FRAMEBUFFER, 0);
        }
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
                const auto& backendVAOIt = VertexArrayImpl::g_backendVertexArrayObjects.find(currentVAO);
                if (backendVAOIt != VertexArrayImpl::g_backendVertexArrayObjects.end()) {
                    backendVAOIt->second->Bind();
                }
            } else {
                MG_External::GLES::glBindVertexArray(0);
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
                    const auto& backendTextureIt = TextureImpl::g_backendTextureObjects.find(textureObject);
                    if (backendTextureIt == TextureImpl::g_backendTextureObjects.end()) continue;

                    GLenum targetGL = MG_Util::ConvertTextureTargetToGLEnum(target);
                    backendTextureIt->second->Bind(targetGL, unit);
                }

                // Bind sampler object if necessary
                const auto& samplerObject = textureUnit.GetSamplerObject();
                if (samplerObject) {
                    const auto& backendSamplerIt = SamplerImpl::g_backendSamplerObjects.find(samplerObject);
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
            const auto& backendProgramIt = PrgramImpl::g_backendProgramObjects.find(currentProgram);
            if (backendProgramIt != PrgramImpl::g_backendProgramObjects.end()) {
                backendProgramIt->second->Use();
                auto backendProgramId = backendProgramIt->second->GetBackendProgramId();
                // Global UBO
                if (currentProgram->GetUBOSize() > 0) {
#ifdef TRACY_ENABLE
                    ZoneScopedNC("UpdateGlobalUBO", TRACY_ZONECOLOR_BACKEND);
#endif
                    MG_External::GLES::glBindBuffer(GL_UNIFORM_BUFFER,
                                                    backendProgramIt->second->GetBackendGlobalUBOId());
                    MG_External::GLES::glBufferSubData(GL_UNIFORM_BUFFER, 0, currentProgram->GetUBOSize(),
                                                       currentProgram->MapUBO());
                    MG_External::GLES::glBindBuffer(GL_UNIFORM_BUFFER, 0);

                    Uint blockIndex = MG_External::GLES::glGetUniformBlockIndex(
                        backendProgramId, MG_Util::ShaderTranspiler::GLOBAL_UBO_NAME);

                    MG_External::GLES::glUniformBlockBinding(backendProgramId, blockIndex, 0);

                    MG_External::GLES::glBindBufferBase(GL_UNIFORM_BUFFER, 0,
                                                        backendProgramIt->second->GetBackendGlobalUBOId());
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
                        GLuint backendBlkIdx =
                            MG_External::GLES::glGetUniformBlockIndex(backendProgramId, name.c_str());
                        MG_External::GLES::glUniformBlockBinding(backendProgramId, backendBlkIdx, lastUBOBinding);

                        // Connect buffer to backend binding point
                        auto& point = MG_State::pGLContext->GetBufferBindingPoint(BufferTarget::Uniform, binding);
                        auto bufferObj = point.GetBoundObject();
                        auto range = point.GetRange();

                        if (bufferObj) {
                            const auto& backendBufferIt = BufferImpl::g_backendBufferObjects.find(bufferObj);
                            if (backendBufferIt != BufferImpl::g_backendBufferObjects.end()) {
                                const auto& backendBufferObject = backendBufferIt->second;
                                backendBufferObject->Bind(GL_UNIFORM_BUFFER);
                                if (range.end == 0) {
                                    MG_External::GLES::glBindBufferBase(GL_UNIFORM_BUFFER, lastUBOBinding,
                                                                        backendBufferObject->GetBackendBufferId());
                                } else {
                                    MG_External::GLES::glBindBufferRange(GL_UNIFORM_BUFFER, lastUBOBinding,
                                                                         backendBufferObject->GetBackendBufferId(),
                                                                         range.start, range.end - range.start);
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
                    for (int loc = 0; loc < maxUniformLoc; ++loc) {
                        auto unit = currentProgram->GetUniformSamplerOrImageUnitIndex(loc);
                        if (unit == -1) continue;
                        auto& name = currentProgram->GetUniformName(loc);
                        auto locAtBackend = MG_External::GLES::glGetUniformLocation(
                            backendProgramIt->second->GetBackendProgramId(), name.c_str());
                        MG_External::GLES::glUniform1i(locAtBackend, unit);

                        auto samplerObject = MG_State::pGLContext->GetTextureUnitObject(unit).GetSamplerObject();

                        if (samplerObject) {
                            const auto& backendSamplerIt = SamplerImpl::g_backendSamplerObjects.find(samplerObject);
                            SharedPtr<SamplerImpl::BackendSamplerObject> backendSamplerObject;
                            if (backendSamplerIt == SamplerImpl::g_backendSamplerObjects.end()) {
                                backendSamplerObject = MakeShared<SamplerImpl::BackendSamplerObject>();
                                SamplerImpl::g_backendSamplerObjects[samplerObject] = backendSamplerObject;
                            } else {
                                backendSamplerObject = backendSamplerIt->second;
                            }
                            backendSamplerObject->SyncToBackend(samplerObject);
                        } else {
                            SamplerImpl::UnbindSampler(unit);
                        }
                    }
                }
            } else {
                MG_External::GLES::glUseProgram(0);
                MGLOG_E("No backend program found (maybe not synced) for current program, cannot use program.");
            }
        }
    }

    void Clear(GLbitfield mask) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        RenderStateImpl::SyncRenderState();

        BindCurrentFBO(FramebufferTarget::Draw);

        MG_External::GLES::glClear(mask);
    }

    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer;
        PrepareForDraw(syncBit);
        MG_External::GLES::glDrawElements(mode, count, type, indices);
    }

    void DrawArrays(GLenum mode, GLint first, GLsizei count) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DrawSyncBit syncBit = DrawSyncBit::None;
        PrepareForDraw(syncBit);
        MG_External::GLES::glDrawArrays(mode, first, count);
    }

    void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLint basevertex) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer;
        PrepareForDraw(syncBit);
        MG_External::GLES::glDrawElementsBaseVertex(mode, count, type, indices, basevertex);
    }

    void MultiDrawElements(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices,
                           GLsizei drawcount) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer;
        PrepareForDraw(syncBit);

        for (GLsizei i = 0; i < drawcount; ++i) {
            MG_External::GLES::glDrawElements(mode, count[i], type, indices[i]);
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
            MG_External::GLES::glDrawElementsBaseVertex(mode, count[i], type, indices[i], basevertex[i]);
        }
        
    }

    void MultiDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer | DrawSyncBit::IndirectBuffer;
        PrepareForDraw(syncBit);

        for (GLsizei i = 0; i < drawcount; ++i) {
            const GLvoid* cmd = reinterpret_cast<const GLvoid*>(reinterpret_cast<const std::uint8_t*>(indirect) +
                                                                i * (stride ? stride : sizeof(GLsizei) * 4));
            MG_External::GLES::glDrawElementsIndirect(mode, type, cmd);
        }
    }

    void MultiDrawArraysIndirect(GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DrawSyncBit syncBit = DrawSyncBit::IndirectBuffer;
        PrepareForDraw(syncBit);

        for (GLsizei i = 0; i < drawcount; ++i) {
            const GLvoid* cmd = reinterpret_cast<const GLvoid*>(reinterpret_cast<const std::uint8_t*>(indirect) +
                                                                i * (stride ? stride : sizeof(GLsizei) * 4));
            MG_External::GLES::glDrawArraysIndirect(mode, cmd);
        }
    }

    void DrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type,
                                     const void* indices, GLint basevertex) {
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer;
        PrepareForDraw(syncBit);
        MG_External::GLES::glDrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex);
    }

    void DrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices) {
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer;
        PrepareForDraw(syncBit);
        MG_External::GLES::glDrawRangeElements(mode, start, end, count, type, indices);
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
        MG_External::GLES::glDrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex);
    }

    void DrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                           GLsizei instancecount, GLuint baseinstance) {
    
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer | DrawSyncBit::Instancing;
        PrepareForDraw(syncBit);
    
        auto currentProgram = MG_State::pGLContext->GetCurrentProgram();
        if (!currentProgram || !currentProgram->GetLinkStatus()) {
            MGLOG_E("No valid program is currently bound.");
            return;
        }
    
        const auto& backendProgramIt = PrgramImpl::g_backendProgramObjects.find(currentProgram);
        if (backendProgramIt == PrgramImpl::g_backendProgramObjects.end()) {
            MGLOG_E("No backend program found.");
            return;
        }
    
        auto backendProgram = backendProgramIt->second;
        auto backendProgramId = backendProgram->GetBackendProgramId();
    
        // 检查是否已经有baseInstance uniform位置
        static UnorderedMap<GLuint, GLint> programBaseInstanceLocations;
        GLint baseInstanceLocation = -1;
    
        auto it = programBaseInstanceLocations.find(backendProgramId);
        if (it == programBaseInstanceLocations.end()) {
            // 第一次使用这个程序，获取uniform位置
            baseInstanceLocation = MG_External::GLES::glGetUniformLocation(backendProgramId, "u_BaseInstance");
            if (baseInstanceLocation == -1) {
                // 尝试其他可能的uniform名称
                baseInstanceLocation = MG_External::GLES::glGetUniformLocation(backendProgramId, "baseInstance");
                if (baseInstanceLocation == -1) {
                    baseInstanceLocation = MG_External::GLES::glGetUniformLocation(backendProgramId, "u_baseinstance");
                }
            }
            programBaseInstanceLocations[backendProgramId] = baseInstanceLocation;
        } else {
            baseInstanceLocation = it->second;
        }
    
        // 保存当前的uniform值（如果有的话）
        GLint savedBaseInstanceValue = 0;
    
        // 设置baseInstance uniform值
        if (baseInstanceLocation != -1) {
            MG_External::GLES::glUniform1ui(baseInstanceLocation, baseinstance);
        } else {
                // 如果程序中没有baseInstance uniform，我们需要修改顶点着色器
                // 或者在绘制时使用其他技术来模拟
                MGLOG_W("Program does not have a baseInstance uniform. Base instance offset may not work correctly.");
        
                // 替代方案：如果baseinstance不为0，我们可以使用多次绘制调用来模拟
                if (baseinstance > 0) {
                    MGLOG_W("Using multiple draw calls to simulate baseinstance = %u", baseinstance);
            
                    // 为每个实例单独绘制（性能较差，仅作为fallback）
                for (GLuint instance = baseinstance; instance < baseinstance + instancecount; instance++) {
                    if (baseInstanceLocation != -1) {
                        MG_External::GLES::glUniform1ui(baseInstanceLocation, instance);
                        MG_External::GLES::glDrawElements(mode, count, type, indices);
                    } else {
                        // 如果没有baseInstance uniform，我们可以尝试通过顶点属性来传递
                        // 这里使用gl_InstanceID直接计算，需要特殊的顶点着色器
                        MG_External::GLES::glDrawElements(mode, count, type, indices);
                    }
                }
                return;
            }
        }
        
        // 执行实例化绘制
        MG_External::GLES::glDrawElementsInstanced(mode, count, type, indices, instancecount);
        
        // 恢复baseInstance uniform值（如果需要）
        if (baseInstanceLocation != -1 && savedBaseInstanceValue != baseinstance) {
            // 我们可以恢复到0，或者保留当前值
            MG_External::GLES::glUniform1ui(baseInstanceLocation, 0);
        }
    }

    void DrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount) {
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer | DrawSyncBit::Instancing;
        PrepareForDraw(syncBit);
        MG_External::GLES::glDrawElementsInstanced(mode, count, type, indices, instancecount);
    }

    void DrawElementsIndirect(GLenum mode, GLenum type, const void* indirect) {
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer | DrawSyncBit::IndirectBuffer;
        PrepareForDraw(syncBit);
        MG_External::GLES::glDrawElementsIndirect(mode, type, indirect);
    }

    void DrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instancecount,
                                         GLuint baseinstance) {
        // Not supported in OpenGL ES
        MGLOG_W("DrawArraysInstancedBaseInstance is not supported in OpenGL ES.");
    }

    void DrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {
        DrawSyncBit syncBit = DrawSyncBit::Instancing;
        PrepareForDraw(syncBit);
        MG_External::GLES::glDrawArraysInstanced(mode, first, count, instancecount);
    }

    void DrawArraysIndirect(GLenum mode, const void* indirect) {
        DrawSyncBit syncBit = DrawSyncBit::IndirectBuffer;
        PrepareForDraw(syncBit);
        MG_External::GLES::glDrawArraysIndirect(mode, indirect);
    }

    void BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1,
                         GLint dstY1, GLbitfield mask, GLenum filter) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DebugImpl::ErrorLopper errorLopper;

        TextureImpl::SyncNeccessaryTextures();
        errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        FramebufferImpl::SyncCurrentFBO();
        errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        RenderStateImpl::SyncRenderState();
        errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });

        BindCurrentFBO(FramebufferTarget::Draw);
        BindCurrentFBO(FramebufferTarget::Read);
        errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        MGLOG_D("ES %s(%d, %d, %d, %d, %d, %d, %d, %d, 0x%x, %s)", __func__, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0,
                dstX1, dstY1, mask, MG_Util::ConvertGLEnumToString(filter).c_str());
        MG_External::GLES::glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
        errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
    }

    Bool UpdateTextureBindingAtTarget(GLenum target) {
#ifdef TRACY_ENABLE
        ZoneScopedNC(__func__, TRACY_ZONECOLOR_BACKEND);
#endif
        auto unit = MG_State::pGLContext->GetActiveTextureUnit();
        auto& textureUnit = MG_State::pGLContext->GetTextureUnitObject(unit);

        auto textureTarget = MG_Util::ConvertGLEnumToTextureTarget(target);
        if (!TextureImpl::IsSupportedTextureTarget(textureTarget)) {
            MOBILEGL_ASSERT(false, "    Texture target %s is not supported, skipping.",
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

            const auto& backendTextureIt = TextureImpl::g_backendTextureObjects.find(textureObject);
            SharedPtr<TextureImpl::BackendTextureObject> backendTextureObject;
            if (backendTextureIt == TextureImpl::g_backendTextureObjects.end()) {
                backendTextureObject = MakeShared<TextureImpl::BackendTextureObject>();
                TextureImpl::g_backendTextureObjects[textureObject] = backendTextureObject;
            } else {
                backendTextureObject = backendTextureIt->second;
            }
            backendTextureObject->Bind(target, unit);
        }
        return true;
    }

    void CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width,
                        GLsizei height, GLint border) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        DebugImpl::ErrorLopper errorLopper;
        MGLOG_D("%s: Backend", __func__);
        TextureImpl::SyncNeccessaryTextures();
        errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        FramebufferImpl::SyncCurrentFBO();
        errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        RenderStateImpl::SyncRenderState();
        errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });

        if (!UpdateTextureBindingAtTarget(target)) return;

        auto mglInternalFormat = MG_Util::ConvertGLEnumToTextureInternalFormat(internalformat);

        GLenum format = GL_DEPTH_COMPONENT;
        GLenum type = GL_UNSIGNED_INT;
        TextureImpl::GenerateTextureFormatInfo(mglInternalFormat, &internalformat, &format, &type);
        MOBILEGL_ASSERT(format != GL_NONE && type != GL_NONE,
                        "%s: cannot GenerateTextureFormatInfo(%s): out internalformat=%s, format=%s, type=%s",
                        MG_Util::ConvertTextureInternalFormatToString(mglInternalFormat).c_str(),
                        MG_Util::ConvertGLEnumToString(internalformat).c_str(),
                        MG_Util::ConvertGLEnumToString(format).c_str(), MG_Util::ConvertGLEnumToString(type).c_str());
        TexturePixelDataType texturePixelDataType = MG_Util::ConvertGLEnumToTexturePixelDataType(type);

        Bool isDepthFormat =
            MG_Util::IsDepthFormatInternalFormat(MG_Util::ConvertGLEnumToTextureInternalFormat(internalformat));
        Bool isStencilFormat =
            MG_Util::IsStencilFormatInternalFormat(MG_Util::ConvertGLEnumToTextureInternalFormat(internalformat));

        if (!isDepthFormat) {
            MG_External::GLES::glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
            errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
        } else {
            MGLOG_D("%s: Backend depth", __func__);
            MG_External::GLES::glTexImage2D(target, level, (GLint)internalformat, width, height, border, format, type,
                                            nullptr);
            errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });

            GLint currentTex;
            MG_External::GLES::glGetIntegerv(Utils::GetBindingQuery(target, false), &currentTex);
            errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });

            GLenum attachment = isStencilFormat ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
            MG_External::GLES::glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, target, currentTex, level);

            if (MG_External::GLES::glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                MGLOG_E("ES glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE");
                return;
            }

            MG_External::GLES::glBlitFramebuffer(x, y, x + width, y + height, 0, 0, width, height,
                                                 GL_DEPTH_BUFFER_BIT | (isStencilFormat ? GL_STENCIL_BUFFER_BIT : 0),
                                                 GL_NEAREST);
            errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
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
        errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        FramebufferImpl::SyncCurrentFBO();
        errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        RenderStateImpl::SyncRenderState();
        errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });

        if (!UpdateTextureBindingAtTarget(target)) return;

        BindCurrentFBO(FramebufferTarget::Read);
        errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        GLenum internalFormat;
        MG_External::GLES::glGetTexLevelParameteriv(target, level, GL_TEXTURE_INTERNAL_FORMAT, (GLint*)&internalFormat);
        errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        auto mglInternalFormat = MG_Util::ConvertGLEnumToTextureInternalFormat(internalFormat);

        Bool isDepthFormat = MG_Util::IsDepthFormatInternalFormat(mglInternalFormat);
        Bool isStencilFormat = MG_Util::IsStencilFormatInternalFormat(mglInternalFormat);

        if (!isDepthFormat) {
            MG_External::GLES::glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
            errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
        } else {
            MGLOG_D("%s: Backend depth", __func__);
            GLint currentTex;
            MG_External::GLES::glGetIntegerv(Utils::GetBindingQuery(target, false), &currentTex);
            errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
            GLenum attachment = isStencilFormat ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
            MG_External::GLES::glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, target, currentTex, level);
            errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
            if (MG_External::GLES::glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                MGLOG_E("ES glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE");
                return;
            }

            MG_External::GLES::glBlitFramebuffer(
                x, y, x + width, y + height, xoffset, yoffset, xoffset + width, yoffset + height,
                GL_DEPTH_BUFFER_BIT | (isStencilFormat ? GL_STENCIL_BUFFER_BIT : 0), GL_NEAREST);
            errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
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
        auto texture = slot.GetBoundObject();
        auto backendTexture = TextureImpl::SyncTextureObjectToBackend(texture);

        backendTexture->Bind(target, unitIndex);
        MG_External::GLES::glGenerateMipmap(target);
    }

    const GLubyte* GetString(GLenum name) {
        return MG_External::GLES::glGetString(name);
    }

    void ClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        RenderStateImpl::SyncRenderState();

        BindCurrentFBO(FramebufferTarget::Draw);

        MG_External::GLES::glClearBufferfi(buffer, drawbuffer, depth, stencil);
    }

    void ClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value) {
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        RenderStateImpl::SyncRenderState();

        BindCurrentFBO(FramebufferTarget::Draw);
        auto backendFBOIt = FramebufferImpl::g_backendFramebufferObjects.find(
            MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw).GetBoundObject());
        if (backendFBOIt == FramebufferImpl::g_backendFramebufferObjects.end()) {
            MGLOG_E("No backend FBO found for current draw FBO, cannot clear buffer.");
            return;
        }
        auto backendFBO = backendFBOIt->second;

        GLint realDrawbuffer = drawbuffer;

        if (buffer == GL_COLOR) {
            auto& stateDrawBuffers = backendFBOIt->first->GetDrawBuffers();

            if (drawbuffer < 0 || drawbuffer >= MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS) {
                MGLOG_E("Invalid drawbuffer index: %d", drawbuffer);
                return;
            }

            FramebufferAttachmentType attachmentType = stateDrawBuffers[drawbuffer];

            if (attachmentType == FramebufferAttachmentType::None) {
                MGLOG_D("Drawbuffer %d has no attachment, skipping clear", drawbuffer);
                return;
            }

            Bool found = false;
            for (int i = 0; i < MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS; i++) {
                if (backendFBO->GetCompactedAttachmentTypeAtDrawBufferIndex(i) == attachmentType) {
                    realDrawbuffer = i;
                    found = true;
                    break;
                }
            }

            if (!found) {
                MGLOG_E("Failed to find backend drawbuffer for attachment type: %d", static_cast<int>(attachmentType));
                return;
            }
        } else if (buffer == GL_DEPTH || buffer == GL_STENCIL) {
            if (drawbuffer != 0) {
                MGLOG_W("Depth/stencil clear buffer index must be 0, got %d. Using 0.", drawbuffer);
            }
            realDrawbuffer = 0;
        }

        MG_External::GLES::glClearBufferfv(buffer, realDrawbuffer, value);
    }
    void ClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value) {
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        RenderStateImpl::SyncRenderState();

        BindCurrentFBO(FramebufferTarget::Draw);
        auto backendFBOIt = FramebufferImpl::g_backendFramebufferObjects.find(
            MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw).GetBoundObject());
        if (backendFBOIt == FramebufferImpl::g_backendFramebufferObjects.end()) {
            MGLOG_E("No backend FBO found for current draw FBO, cannot clear buffer.");
            return;
        }
        auto backendFBO = backendFBOIt->second;

        GLint realDrawbuffer = drawbuffer;

        if (buffer == GL_COLOR) {
            auto& stateDrawBuffers = backendFBOIt->first->GetDrawBuffers();

            if (drawbuffer < 0 || drawbuffer >= MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS) {
                MGLOG_E("Invalid drawbuffer index: %d", drawbuffer);
                return;
            }

            FramebufferAttachmentType attachmentType = stateDrawBuffers[drawbuffer];

            if (attachmentType == FramebufferAttachmentType::None) {
                MGLOG_D("Drawbuffer %d has no attachment, skipping clear", drawbuffer);
                return;
            }

            Bool found = false;
            for (int i = 0; i < MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS; i++) {
                if (backendFBO->GetCompactedAttachmentTypeAtDrawBufferIndex(i) == attachmentType) {
                    realDrawbuffer = i;
                    found = true;
                    break;
                }
            }

            if (!found) {
                MGLOG_E("Failed to find backend drawbuffer for attachment type: %d", static_cast<int>(attachmentType));
                return;
            }
        } else if (buffer == GL_STENCIL) {
            if (drawbuffer != 0) {
                MGLOG_W("Stencil clear buffer index must be 0, got %d. Using 0.", drawbuffer);
            }
            realDrawbuffer = 0;
        }

        MG_External::GLES::glClearBufferiv(buffer, realDrawbuffer, value);
    }

    void ClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value) {
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        RenderStateImpl::SyncRenderState();

        BindCurrentFBO(FramebufferTarget::Draw);
        auto backendFBOIt = FramebufferImpl::g_backendFramebufferObjects.find(
            MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw).GetBoundObject());
        if (backendFBOIt == FramebufferImpl::g_backendFramebufferObjects.end()) {
            MGLOG_E("No backend FBO found for current draw FBO, cannot clear buffer.");
            return;
        }
        auto backendFBO = backendFBOIt->second;

        GLint realDrawbuffer = drawbuffer;

        if (buffer == GL_COLOR) {
            auto& stateDrawBuffers = backendFBOIt->first->GetDrawBuffers();

            if (drawbuffer < 0 || drawbuffer >= MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS) {
                MGLOG_E("Invalid drawbuffer index: %d", drawbuffer);
                return;
            }

            FramebufferAttachmentType attachmentType = stateDrawBuffers[drawbuffer];

            if (attachmentType == FramebufferAttachmentType::None) {
                MGLOG_D("Drawbuffer %d has no attachment, skipping clear", drawbuffer);
                return;
            }

            Bool found = false;
            for (int i = 0; i < MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS; i++) {
                if (backendFBO->GetCompactedAttachmentTypeAtDrawBufferIndex(i) == attachmentType) {
                    realDrawbuffer = i;
                    found = true;
                    break;
                }
            }

            if (!found) {
                MGLOG_E("Failed to find backend drawbuffer for attachment type: %d", static_cast<int>(attachmentType));
                return;
            }
        } else {
            MGLOG_E("ClearBufferuiv can only be used with GL_COLOR buffer, got %s",
                    MG_Util::ConvertGLEnumToString(buffer).c_str());
            return;
        }

        MG_External::GLES::glClearBufferuiv(buffer, realDrawbuffer, value);
    }

} // namespace MobileGL::MG_Backend::DirectGLES
