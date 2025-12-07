#include "DirectGLES.h"
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

    namespace DebugImpl {
        void ErrorLopper::Loop(std::function<void(GLenum)> func) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG
            GLenum err = MG_External::GLES::glGetError();
            while (err != GL_NO_ERROR) {
                func(err);
                err = MG_External::GLES::glGetError();
            }
#endif
        }

        void ErrorLopper::Clear() {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG
            GLenum err = MG_External::GLES::glGetError();
            while (err != GL_NO_ERROR) {
                MGLOG_D("Stray GL Error cleared: %s", MG_Util::ConvertGLEnumToString(err).c_str());
                err = MG_External::GLES::glGetError();
            }
#endif
        }

        ErrorLopper::ErrorLopper() {
            Clear();
        }
        ErrorLopper::~ErrorLopper() {
            Clear();
        }
    } // namespace DebugImpl

    // TODO: deletion for deleted objects

    namespace BufferImpl {
        void SyncNeccessaryBuffers(Bool includeIBO = false, Bool includeIndirectBuffer = false) {
            // All buffers we need are:
            //   1.VBO 2.IBO (if needed) 3.UBO 4.IndirectBuffer (if needed) 5.SSBO (TODO)
            // PBO is not needed since it should be handled in frontend

            Vector<SharedPtr<MG_State::GLState::BufferObject>> buffersToSync;
            const auto& currentVAOObject = MG_State::pGLContext->GetBoundVertexArray();
            if (!currentVAOObject) {
                MGLOG_E("No VAO is currently bound, cannot sync necessary buffers.");
                return;
            }

            // VBO
            for (const auto& attrib : currentVAOObject->GetAllAttributes()) {
                if (!attrib.Enabled) continue;
                const auto& bufferObject = attrib.Buffer;
                if (bufferObject) {
                    const auto& end = buffersToSync.end();
                    if (std::find(buffersToSync.begin(), end, bufferObject) == end) {
                        buffersToSync.push_back(bufferObject);
                    }
                }
            }

            // IBO
            if (includeIBO) {
                const auto& possibleIBO = currentVAOObject->GetIndexBufferBindingSlot().GetBoundObject();
                if (possibleIBO) {
                    const auto& end = buffersToSync.end();
                    if (std::find(buffersToSync.begin(), end, possibleIBO) == end) {
                        buffersToSync.push_back(possibleIBO);
                    }
                }
            }

            // Indirect Buffer Object
            if (includeIndirectBuffer) {
                const auto& possibleIndirectBuffer =
                    MG_State::pGLContext->GetBufferBindingSlot(BufferTarget::DrawIndirect).GetBoundObject();
                if (possibleIndirectBuffer) {
                    const auto& end = buffersToSync.end();
                    if (std::find(buffersToSync.begin(), end, possibleIndirectBuffer) == end) {
                        buffersToSync.push_back(possibleIndirectBuffer);
                    }
                }
            }

            // UBO
            auto uboBindingPointCnt = MG_State::pGLContext->GetBufferBindingPointCount(BufferTarget::Uniform);
            for (SizeT i = 0; i < uboBindingPointCnt; ++i) {
                auto& point = MG_State::pGLContext->GetBufferBindingPoint(BufferTarget::Uniform, i);
                auto obj = point.GetBoundObject();
                if (obj) {
                    const auto& end = buffersToSync.end();
                    if (std::find(buffersToSync.begin(), end, obj) == end) {
                        buffersToSync.push_back(obj);
                    }
                }
            }

            // Do real sync
            for (auto& bufferObject : buffersToSync) {
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
        }
    } // namespace BufferImpl

    namespace VertexArrayImpl {
        void SyncCurrentVAO(Bool needDivisor) {
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
            backendVAOObject->SyncToBackend(currentVAOObject, needDivisor);
        }
    } // namespace VertexArrayImpl

    namespace TextureImpl {
        SharedPtr<BackendTextureObject> SyncTextureObjectToBackend(
            SharedPtr<MG_State::GLState::ITextureObject>& textureObject) {
            const auto& backendTextureIt = g_backendTextureObjects.find(textureObject);
            SharedPtr<BackendTextureObject> backendTextureObject;
            if (backendTextureIt == g_backendTextureObjects.end()) {
                backendTextureObject = MakeShared<BackendTextureObject>();
                g_backendTextureObjects[textureObject] = backendTextureObject;
            } else {
                backendTextureObject = backendTextureIt->second;
            }
            backendTextureObject->SyncToBackend(textureObject);
            return backendTextureObject;
        }

        void SyncNeccessaryTextures() {
            // All textures we need are:
            //   1. textures bound to texture units (TODO: only sync ones that are used in current program)
            //   2. textures used in current FBO
            //   3. textures bound to image units (TODO)

            Vector<SharedPtr<MG_State::GLState::ITextureObject>> texturesToSync;

            for (int index = 0; index < MG_State::GLState::TextureState::MAX_TEXTURE_IMAGE_UNITS; ++index) {
                auto& unit = MG_State::pGLContext->GetTextureUnitObject(index);
                for (const auto& bindingSlot : unit.GetAllBindingSlots()) {
                    const auto& textureObject = bindingSlot.GetBoundObject();
                    if (textureObject) {
                        const auto& end = texturesToSync.end();
                        if (std::find(texturesToSync.begin(), end, textureObject) == end) {
                            texturesToSync.push_back(textureObject);
                        }
                    }
                }
            }

            const auto& currentFBO =
                MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw).GetBoundObject();
            if (currentFBO) {
                for (const auto& attachment : currentFBO->GetAllAttachments()) {
                    if (!attachment.IsTexture()) continue;
                    const auto& textureObject = attachment.GetTexture();
                    if (textureObject) {
                        const auto& end = texturesToSync.end();
                        if (std::find(texturesToSync.begin(), end, textureObject) == end) {
                            texturesToSync.push_back(textureObject);
                        }
                    }
                }
            }

            // Do real sync
            for (auto& textureObject : texturesToSync) {
                SyncTextureObjectToBackend(textureObject);
            }
        }
    } // namespace TextureImpl

    namespace FramebufferImpl {
        void SyncCurrentFBO() {
            const FramebufferTarget fboTargets[] = {FramebufferTarget::Draw, FramebufferTarget::Read};

            MG_State::GLState::FramebufferObject* lastUpdatedFBO = nullptr;

            for (auto target : fboTargets) {
                auto currentFBO = MG_State::pGLContext->GetFramebufferBindingSlot(target).GetBoundObject();

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

                backendFBOObject->Bind(target);

                lastUpdatedFBO = currentFBO.get();
            }
        }
    } // namespace FramebufferImpl

    namespace RenderStateImpl {
        void SyncRenderState() {
            MG_External::GLES::glViewport(
                MG_State::pGLContext->GetViewport().x(), MG_State::pGLContext->GetViewport().y(),
                MG_State::pGLContext->GetViewport().z(), MG_State::pGLContext->GetViewport().w());
#define SYNC_CAPABILITY(cap_mg, cap_gl)                                                                                \
    if (MG_State::pGLContext->IsCapabilityEnabled(cap_mg)) {                                                           \
        MG_External::GLES::glEnable(cap_gl);                                                                           \
    } else {                                                                                                           \
        MG_External::GLES::glDisable(cap_gl);                                                                          \
    }
            SYNC_CAPABILITY(CapabilityInput::Blend, GL_BLEND);
            SYNC_CAPABILITY(CapabilityInput::DepthTest, GL_DEPTH_TEST);
            SYNC_CAPABILITY(CapabilityInput::ScissorTest, GL_SCISSOR_TEST);
            SYNC_CAPABILITY(CapabilityInput::CullFace, GL_CULL_FACE);

#undef SYNC_CAPABILITY

            const auto& ToGLBoolean = [](Bool b) -> GLboolean { return b ? GL_TRUE : GL_FALSE; };

            { // Blend func
                BlendFactor srcRGB, dstRGB, srcAlpha, dstAlpha;
                MG_State::pGLContext->GetBlendFunc(srcRGB, dstRGB, srcAlpha, dstAlpha);

                MG_External::GLES::glBlendFuncSeparate(
                    MG_Util::ConvertBlendFactorToGLEnum(srcRGB), MG_Util::ConvertBlendFactorToGLEnum(dstRGB),
                    MG_Util::ConvertBlendFactorToGLEnum(srcAlpha), MG_Util::ConvertBlendFactorToGLEnum(dstAlpha));
            }

            { // Blend equation
                DepthTestFunc df = MG_State::pGLContext->GetDepthFunc();
                MG_External::GLES::glDepthFunc(MG_Util::ConvertDepthTestFuncToGLEnum(df));

                MG_External::GLES::glDepthMask(MG_State::pGLContext->GetDepthMask() ? GL_TRUE : GL_FALSE);
            }

            { // Color mask
                BoolVec4 colorMask = MG_State::pGLContext->GetColorMask();
                MG_External::GLES::glColorMask(ToGLBoolean(colorMask.x()), ToGLBoolean(colorMask.y()),
                                               ToGLBoolean(colorMask.z()), ToGLBoolean(colorMask.w()));
            }

            { // Clear values
                const FloatVec4& clearCol = MG_State::pGLContext->GetClearColor();
                MG_External::GLES::glClearColor(clearCol.x(), clearCol.y(), clearCol.z(), clearCol.w());
                MG_External::GLES::glClearDepthf(MG_State::pGLContext->GetClearDepth());
            }

            { // Cull face mode
                CullFaceMode cfm = MG_State::pGLContext->GetCullFaceMode();
                MG_External::GLES::glCullFace(MG_Util::ConvertCullFaceModeToGLEnum(cfm));
            }

            { // Scissor box
                const IntVec4& scissorBox = MG_State::pGLContext->GetScissorBox();
                MG_External::GLES::glScissor(scissorBox.x(), scissorBox.y(), scissorBox.z(), scissorBox.w());
            }
        }
    } // namespace RenderStateImpl

    namespace PrgramImpl {
        void SyncCurrentProgram() {
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
        const auto& currentFBO = MG_State::pGLContext->GetFramebufferBindingSlot(target).GetBoundObject();
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
        BufferImpl::SyncNeccessaryBuffers(syncBit & DrawSyncBit::IndexBuffer, syncBit & DrawSyncBit::IndirectBuffer);
        VertexArrayImpl::SyncCurrentVAO(syncBit & DrawSyncBit::Instancing);
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        PrgramImpl::SyncCurrentProgram();
        RenderStateImpl::SyncRenderState();

        BindCurrentFBO(FramebufferTarget::Draw);

        const auto& currentVAO = MG_State::pGLContext->GetBoundVertexArray();
        if (currentVAO) {
            const auto& backendVAOIt = VertexArrayImpl::g_backendVertexArrayObjects.find(currentVAO);
            if (backendVAOIt != VertexArrayImpl::g_backendVertexArrayObjects.end()) {
                backendVAOIt->second->Bind();
            }
        } else {
            MG_External::GLES::glBindVertexArray(0);
        }

        Int maxTextureUnits = MG_State::GLState::TextureState::MAX_TEXTURE_IMAGE_UNITS;
        for (Int unit = 0; unit < maxTextureUnits; ++unit) {
            auto& textureUnit = MG_State::pGLContext->GetTextureUnitObject(unit);

            MG_External::GLES::glActiveTexture(GL_TEXTURE0 + unit);

            for (const auto& bindingSlot : textureUnit.GetAllBindingSlots()) {
                const auto& textureObject = bindingSlot.GetBoundObject();
                if (!textureObject) continue;

                // Bind texture object
                auto target = textureObject->GetTarget();
                if (target == TextureTarget::TextureBuffer || target == TextureTarget::Texture1D ||
                    target == TextureTarget::TextureRectangle || target == TextureTarget::Texture2DMultisampleArray ||
                    target == TextureTarget::Texture1DArray || target == TextureTarget::Texture3D ||
                    target == TextureTarget::Texture2DMultisample || target == TextureTarget::Texture2DArray) {
                    MGLOG_D("    Texture target %s is not supported, skipping.",
                            MG_Util::ConvertTextureTargetToString(target).c_str());
                    continue;
                }
                const auto& backendTextureIt = TextureImpl::g_backendTextureObjects.find(textureObject);
                if (backendTextureIt == TextureImpl::g_backendTextureObjects.end()) continue;

                GLenum targetGL = MG_Util::ConvertTextureTargetToGLEnum(target);
                backendTextureIt->second->Bind(targetGL);
            }

            // Bind sampler object
            const auto& samplerObject = textureUnit.GetSamplerObject();
            if (samplerObject) {
                const auto& backendSamplerIt = SamplerImpl::g_backendSamplerObjects.find(samplerObject);
                if (backendSamplerIt != SamplerImpl::g_backendSamplerObjects.end()) {
                    backendSamplerIt->second->Bind(unit);
                }
            } else {
                MG_External::GLES::glBindSampler(unit, 0);
            }
        }

        const auto& currentProgram = MG_State::pGLContext->GetCurrentProgram();
        if (currentProgram && currentProgram->GetLinkStatus()) {
            const auto& backendProgramIt = PrgramImpl::g_backendProgramObjects.find(currentProgram);
            if (backendProgramIt != PrgramImpl::g_backendProgramObjects.end()) {
                backendProgramIt->second->Use();
                auto backendProgramId = backendProgramIt->second->GetBackendProgramId();
                // Global UBO
                if (currentProgram->GetUBOSize() > 0) {
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
                // Normal UBO
                auto uboCount = currentProgram->GetActiveUniformBlocksCount();
                Uint lastUBOBinding = 0; // to prevent overlapping bindings between global UBO and normal UBOs
                for (Int i = 0; i < uboCount; ++i) {
                    ++lastUBOBinding;
                    // program state binding index == backend binding index

                    // Connect program ubo index to backend binding point
                    auto binding = currentProgram->GetUniformBlockBinding(i);
                    auto& name = currentProgram->GetUniformBlockName(i);
                    GLuint backendBlkIdx = MG_External::GLES::glGetUniformBlockIndex(backendProgramId, name.c_str());
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
                        MG_External::GLES::glBindSampler(unit, 0);
                    }
                }
            } else {
                MG_External::GLES::glUseProgram(0);
                MGLOG_E("No backend program found (maybe not synced) for current program, cannot use program.");
            }
        }
    }

    void Clear(GLbitfield mask) {
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        RenderStateImpl::SyncRenderState();

        BindCurrentFBO(FramebufferTarget::Draw);

        MG_External::GLES::glClear(mask);
    }

    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer;
        PrepareForDraw(syncBit);
        MG_External::GLES::glDrawElements(mode, count, type, indices);
    }

    void DrawArrays(GLenum mode, GLint first, GLsizei count) {
        DrawSyncBit syncBit = DrawSyncBit::None;
        PrepareForDraw(syncBit);
        MG_External::GLES::glDrawArrays(mode, first, count);
    }

    void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLint basevertex) {
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer;
        PrepareForDraw(syncBit);
        MG_External::GLES::glDrawElementsBaseVertex(mode, count, type, indices, basevertex);
    }

    void MultiDrawElements(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices,
                           GLsizei drawcount) {
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer;
        PrepareForDraw(syncBit);

        for (GLsizei i = 0; i < drawcount; ++i) {
            MG_External::GLES::glDrawElements(mode, count[i], type, indices[i]);
        }
    }

    void MultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, 
                                     GLenum type, const GLvoid* const* indices,
                                     GLsizei drawcount, const GLint* basevertex) 
    {
        // 参数验证
        if (drawcount <= 0) {
            return; // 无绘制操作
        }
    
        // 检查是否所有绘制都有效
        bool hasValidDraw = false;
        for (GLsizei i = 0; i < drawcount; ++i) {
            if (count[i] > 0) {
                hasValidDraw = true;
                break;
            }
        }
    
        if (!hasValidDraw) {
            return; // 所有绘制调用都是空的
        }
    
        // 确定同步需求
        DrawSyncBit syncBit = DrawSyncBit::None;
        
        // 执行同步准备
        if (syncBit != DrawSyncBit::None) {
            PrepareForDraw(syncBit);
        }
    
        // 批量处理优化
        // 如果所有绘制使用相同的 basevertex（通常是 0），可以更高效处理
        bool uniformBaseVertex = true;
        const GLint firstBaseVertex = basevertex ? basevertex[0] : 0;
    
        if (basevertex) {
            for (GLsizei i = 1; i < drawcount; ++i) {
                if (basevertex[i] != firstBaseVertex) {
                    uniformBaseVertex = false;
                    break;
                }
            }
        }
    
        // 回退到循环调用单个绘制命令
        // 优化：批量处理连续的相同参数绘制
        GLsizei current = 0;
        while (current < drawcount) {
            // 跳过顶点数为0的绘制
            if (count[current] == 0) {
                current++;
                continue;
            }
        
            // 查找连续的可以使用相同调用的绘制
            GLsizei batchCount = 1;
        
            // 如果可以批量处理，尝试合并更多绘制
            if (basevertex) {
                // 查找具有相同 basevertex 的连续绘制
                GLint currentBaseVertex = basevertex[current];
                for (GLsizei next = current + 1; 
                     next < drawcount && batchCount < 16; // 限制批量大小
                     ++next) 
                {
                    if (count[next] == 0) {
                        continue;
                    }
                
                    // 检查是否可以合并
                    bool canBatch = (basevertex[next] == currentBaseVertex);
                
                    // 这里可以添加更多合并条件，比如检查其他状态是否一致
                
                    if (canBatch) {
                        batchCount++;
                    } else {
                        break;
                    }
                }
            
            }
        
            // 单个绘制调用
            MG_External::GLES::glDrawElementsBaseVertex(
                mode, count[current], type, indices[current], 
                basevertex ? basevertex[current] : 0);
        
            current++;
        }
    }

    void MultiDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride) {
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer | DrawSyncBit::IndirectBuffer;
        PrepareForDraw(syncBit);

        for (GLsizei i = 0; i < drawcount; ++i) {
            const GLvoid* cmd = reinterpret_cast<const GLvoid*>(reinterpret_cast<const uint8_t*>(indirect) +
                                                                i * (stride ? stride : sizeof(GLsizei) * 4));
            MG_External::GLES::glDrawElementsIndirect(mode, type, cmd);
        }
    }

    void MultiDrawArraysIndirect(GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride) {
        DrawSyncBit syncBit = DrawSyncBit::IndirectBuffer;
        PrepareForDraw(syncBit);

        for (GLsizei i = 0; i < drawcount; ++i) {
            const GLvoid* cmd = reinterpret_cast<const GLvoid*>(reinterpret_cast<const uint8_t*>(indirect) +
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
        // Not supported in OpenGL ES
        MGLOG_W("DrawElementsInstancedBaseInstance is not supported in OpenGL ES.");
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
        MG_External::GLES::glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
        errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
    }

    void CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width,
                        GLsizei height, GLint border) {
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

        GLint realInternalFormat;
        MG_External::GLES::glGetTexLevelParameteriv(target, level, GL_TEXTURE_INTERNAL_FORMAT, &realInternalFormat);
        errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
            MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
        });
        internalformat = (GLenum)realInternalFormat;
        auto mglInternalFormat = MG_Util::ConvertGLEnumToTextureInternalFormat(internalformat);

        GLenum format = GL_DEPTH_COMPONENT;
        GLenum type = GL_UNSIGNED_INT;
        TextureImpl::GenerateTextureFormatInfo(mglInternalFormat, &internalformat, &format, &type);
        TexturePixelDataType texturePixelDataType = MG_Util::ConvertGLEnumToTexturePixelDataType(type);

        bool isDepthFormat =
            MG_Util::IsDepthFormatInternalFormat(MG_Util::ConvertGLEnumToTextureInternalFormat(internalformat));
        bool isStencilFormat =
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
            FramebufferImpl::BackendFramebufferBindingProtector drawFboProtector(GL_DRAW_FRAMEBUFFER);
            FramebufferImpl::BackendFramebufferBindingProtector readFboProtector(GL_READ_FRAMEBUFFER);
            errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });

            FramebufferImpl::BackendFramebufferBindingProtector::BindTempFBO(FramebufferTarget::Draw);
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
                // Protector will automatically revert to previous fbo states
                return;
            }

            MG_External::GLES::glBlitFramebuffer(x, y, x + width, y + height, 0, 0, width, height,
                                                 GL_DEPTH_BUFFER_BIT | (isStencilFormat ? GL_STENCIL_BUFFER_BIT : 0),
                                                 GL_NEAREST);
            errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
            // Protector will automatically revert to previous fbo states
        }

        // TODO: potential desync between MG_State and backend
        auto activeUnit = MG_State::pGLContext->GetTextureUnitObject(MG_State::pGLContext->GetActiveTextureUnit());
        TextureTarget textureTarget = MG_Util::ConvertGLEnumToTextureTarget(target);
        auto& bindingSlot = activeUnit.GetBindingSlot(textureTarget);
        auto textureObject = bindingSlot.GetBoundObject();

        textureObject->SetInternalFormat(mglInternalFormat);
        textureObject->AllocateStorage(TextureUploadTarget::Texture2D, level, {{width, height, 1}, 0});
    }

    void CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width,
                           GLsizei height) {
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

        Int unit = MG_State::pGLContext->GetActiveTextureUnit();
        auto& activeUnit = MG_State::pGLContext->GetTextureUnitObject(unit);
        TextureTarget textureTarget = MG_Util::ConvertGLEnumToTextureTarget(target);
        auto& bindingSlot = activeUnit.GetBindingSlot(textureTarget);
        const auto& textureObject = bindingSlot.GetBoundObject();

        const auto& backendTextureIt = TextureImpl::g_backendTextureObjects.find(textureObject);
        SharedPtr<TextureImpl::BackendTextureObject> backendTextureObject;
        if (backendTextureIt == TextureImpl::g_backendTextureObjects.end()) {
            backendTextureObject = MakeShared<TextureImpl::BackendTextureObject>();
            TextureImpl::g_backendTextureObjects[textureObject] = backendTextureObject;
        } else {
            backendTextureObject = backendTextureIt->second;
        }
        backendTextureObject->Bind(target);

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

        bool isDepthFormat = MG_Util::IsDepthFormatInternalFormat(mglInternalFormat);
        bool isStencilFormat = MG_Util::IsStencilFormatInternalFormat(mglInternalFormat);

        if (!isDepthFormat) {
            MG_External::GLES::glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
            errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
        } else {
            MGLOG_D("%s: Backend depth", __func__);
            FramebufferImpl::BackendFramebufferBindingProtector drawFboProtector(GL_DRAW_FRAMEBUFFER);
            FramebufferImpl::BackendFramebufferBindingProtector readFboProtector(GL_READ_FRAMEBUFFER);
            errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
            FramebufferImpl::BackendFramebufferBindingProtector::BindTempFBO(FramebufferTarget::Draw);
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
            errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
            if (MG_External::GLES::glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                // Protector will automatically revert to previous fbo states
                return;
            }

            MG_External::GLES::glBlitFramebuffer(
                x, y, x + width, y + height, xoffset, yoffset, xoffset + width, yoffset + height,
                GL_DEPTH_BUFFER_BIT | (isStencilFormat ? GL_STENCIL_BUFFER_BIT : 0), GL_NEAREST);
            errorLopper.Loop([file = __FILE__, line = __LINE__](auto err) {
                MGLOG_D("ES error (%s:%d): %s", file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
            // Protector will automatically revert to previous fbo states
        }
    }

    void GenerateMipmap(GLenum target) {
        auto unitIndex = MG_State::pGLContext->GetActiveTextureUnit();
        auto& unit = MG_State::pGLContext->GetTextureUnitObject(unitIndex);
        auto& slot = unit.GetBindingSlot(MG_Util::ConvertGLEnumToTextureTarget(target));
        auto texture = slot.GetBoundObject();
        auto backendTexture = TextureImpl::SyncTextureObjectToBackend(texture);

        TextureImpl::BackendTextureBindingProtector protector(target);
        backendTexture->Bind(target);
        MG_External::GLES::glGenerateMipmap(target);
    }

    const GLubyte* GetString(GLenum name) {
        return MG_External::GLES::glGetString(name);
    }
} // namespace MobileGL::MG_Backend::DirectGLES
