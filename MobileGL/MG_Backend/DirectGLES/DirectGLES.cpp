#include "DirectGLES.h"
#include "Utils.h"
#include "Managers.h"
#include <MG_State/GLState/Core.h>
#include <MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.h>
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>
#include <MG_Util/Converters/GLToStr/GLEnumConverter.h>
#include <MG_Util/Converters/MGToGL/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToGL/RenderStateEnumConverter.h>

namespace MobileGL::MG_Backend::DirectGLES {
    // TODO: deletion of deleted objects

    namespace BufferImpl {
        void SyncNeccessaryBuffers() {
            // All buffers we need are:
            //   1.VBOs 2.IBO 3.UBOs (TODO) 4.SSBOs (TODO)
            Vector<SharedPtr<MG_State::GLState::BufferObject>> buffersToSync;
            const auto& currentVAOObject = MG_State::pGLContext->GetBoundVertexArray();
            if (!currentVAOObject) {
                MGLOG_E("No VAO is currently bound, cannot sync necessary buffers.");
                return;
            }

            for (const auto& attrib : currentVAOObject->GetAllAttributes()) {
                const auto& bufferObject = attrib.Buffer;
                if (bufferObject) {
                    buffersToSync.push_back(bufferObject);
                }
            }
            const auto& possibleIBO = currentVAOObject->GetIndexBufferBindingSlot().GetBoundObject();
            if (possibleIBO) {
                buffersToSync.push_back(possibleIBO);
            }

            const auto& pbo = MG_State::pGLContext->GetBufferBindingSlot(BufferTarget::PixelUnpack).GetBoundObject();
            if (pbo) {
                buffersToSync.push_back(pbo);
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
        void SyncCurrentVAO() {
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
                        texturesToSync.push_back(textureObject);
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
                        texturesToSync.push_back(textureObject);
                    }
                }
            }

            // Do real sync
            for (auto& textureObject : texturesToSync) {
                const auto& backendTextureIt = g_backendTextureObjects.find(textureObject);
                SharedPtr<BackendTextureObject> backendTextureObject;
                if (backendTextureIt == g_backendTextureObjects.end()) {
                    backendTextureObject = MakeShared<BackendTextureObject>();
                    g_backendTextureObjects[textureObject] = backendTextureObject;
                } else {
                    backendTextureObject = backendTextureIt->second;
                }
                backendTextureObject->SyncToBackend(textureObject);
            }
        }
    } // namespace TextureImpl

    namespace FramebufferImpl {
        void SyncCurrentFBO() {
            auto currentFBO = MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw).GetBoundObject();
            if (!currentFBO) {
                MGLOG_E("No FBO is currently bound, cannot sync current FBO.");
                return;
            }

            if (currentFBO == MG_Impl::GLImpl::FramebufferImpl::pDefaultFramebufferInfo->defaultFBO) {
                // Default FBO, nothing to sync
                return;
            }

            const auto& backendFBOIt = g_backendFramebufferObjects.find(currentFBO);
            SharedPtr<BackendFramebufferObject> backendFBOObject;
            if (backendFBOIt == g_backendFramebufferObjects.end()) {
                backendFBOObject = MakeShared<BackendFramebufferObject>();
                g_backendFramebufferObjects[currentFBO] = backendFBOObject;
            } else {
                backendFBOObject = backendFBOIt->second;
            }
            backendFBOObject->SyncToBackend(currentFBO);
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

            { // Pixel store params
                {
                    PixelStoreParam p = PixelStoreParam::UnpackAlignment;
                    GLint v = MG_State::pGLContext->GetPixelStoreParam(p);
                    MG_External::GLES::glPixelStorei(MG_Util::ConvertPixelStoreParamToGLEnum(p), v);
                }
                {
                    PixelStoreParam p = PixelStoreParam::PackAlignment;
                    GLint v = MG_State::pGLContext->GetPixelStoreParam(p);
                    MG_External::GLES::glPixelStorei(MG_Util::ConvertPixelStoreParamToGLEnum(p), v);
                }
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
            if (!currentProgram) {
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
            }
        } else {
            if (target == FramebufferTarget::Read)
                MG_External::GLES::glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            else
                MG_External::GLES::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        }
    }

    void PrepareForDraw() {
        BufferImpl::SyncNeccessaryBuffers();
        VertexArrayImpl::SyncCurrentVAO();
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
            if (!textureUnit.GetBindingSlot(TextureTarget::Texture2D).GetBoundObject()) continue;
            // TODO

            MG_External::GLES::glActiveTexture(GL_TEXTURE0 + unit);

            for (const auto& bindingSlot : textureUnit.GetAllBindingSlots()) {
                const auto& textureObject = bindingSlot.GetBoundObject();
                if (!textureObject) continue;

                const auto& backendTextureIt = TextureImpl::g_backendTextureObjects.find(textureObject);
                if (backendTextureIt == TextureImpl::g_backendTextureObjects.end()) continue;

                GLenum target = MG_Util::ConvertTextureTargetToGLEnum(textureObject->GetTarget());
                backendTextureIt->second->Bind(target);

                const auto& samplerObj = textureObject->GetSamplerObject();
                const auto minfilter = samplerObj->GetMinFilter();
                MG_External::GLES::glTexParameteri(target, GL_TEXTURE_MIN_FILTER,
                                                   MG_Util::ConvertSamplerFilterModeToGLEnum(minfilter));
                const auto magfilter = samplerObj->GetMagFilter();
                MG_External::GLES::glTexParameteri(target, GL_TEXTURE_MAG_FILTER,
                                                   MG_Util::ConvertSamplerFilterModeToGLEnum(magfilter));
                MG_External::GLES::glTexParameterf(target, GL_TEXTURE_MIN_LOD, samplerObj->GetMinLod());
                MG_External::GLES::glTexParameterf(target, GL_TEXTURE_MAX_LOD, samplerObj->GetMaxLod());
            }
        }

        Int originalActiveUnit = MG_State::pGLContext->GetActiveTextureUnit();
        MG_External::GLES::glActiveTexture(GL_TEXTURE0 + originalActiveUnit);

        const auto& currentProgram = MG_State::pGLContext->GetCurrentProgram();
        if (currentProgram) {
            const auto& backendProgramIt = PrgramImpl::g_backendProgramObjects.find(currentProgram);
            if (backendProgramIt != PrgramImpl::g_backendProgramObjects.end()) {
                backendProgramIt->second->Use();
                // UBO
                MG_External::GLES::glBindBuffer(GL_UNIFORM_BUFFER, backendProgramIt->second->GetBackendGlobalUBOId());
                MG_External::GLES::glBufferSubData(GL_UNIFORM_BUFFER, 0, currentProgram->GetUBOSize(),
                                                   currentProgram->MapUBO());
                MG_External::GLES::glBindBuffer(GL_UNIFORM_BUFFER, 0);

                Uint blockIndex = MG_External::GLES::glGetUniformBlockIndex(
                    backendProgramIt->second->GetBackendProgramId(), MG_Util::ShaderTranspiler::GLOBAL_UBO_NAME);

                MG_External::GLES::glUniformBlockBinding(backendProgramIt->second->GetBackendProgramId(), blockIndex,
                                                         0);

                MG_External::GLES::glBindBufferBase(GL_UNIFORM_BUFFER, 0,
                                                    backendProgramIt->second->GetBackendGlobalUBOId());

                // Sampler unit binding
                auto maxUniformLoc = currentProgram->GetMaxUniformLocation();
                for (int loc = 0; loc < maxUniformLoc; ++loc) {
                    auto unit = currentProgram->GetUniformSamplerOrImageUnitIndex(loc);
                    if (unit == -1) continue;
                    auto& name = currentProgram->GetUniformName(loc);
                    auto locAtBackend = MG_External::GLES::glGetUniformLocation(
                        backendProgramIt->second->GetBackendProgramId(), name.c_str());
                    MG_External::GLES::glUniform1i(locAtBackend, unit);
                }
            } else {
                MG_External::GLES::glUseProgram(0);
            }
        }
    }

    void DrawArrays(GLenum mode, GLint first, GLsizei count) {
        PrepareForDraw();
        MG_External::GLES::glDrawArrays(mode, first, count);
    }

    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
        PrepareForDraw();
        MG_External::GLES::glDrawElements(mode, count, type, indices);
    }

    void Clear(GLbitfield mask) {
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        RenderStateImpl::SyncRenderState();

        BindCurrentFBO(FramebufferTarget::Draw);

        MG_External::GLES::glClear(mask);
    }

    void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint baseVertex) {
        PrepareForDraw();
        MG_External::GLES::glDrawElementsBaseVertex(mode, count, type, indices, baseVertex);
    }

    void MultiDrawElements(GLenum mode, const GLsizei* counts, GLenum type, const void* const* indices,
                           GLsizei drawcount) {
        PrepareForDraw();
        for (GLsizei i = 0; i < drawcount; ++i) {
            MG_External::GLES::glDrawElements(mode, counts[i], type, indices[i]);
        }
    }

    void MultiDrawElementsBaseVertex(GLenum mode, const GLsizei* counts, GLenum type, const void* const* indices,
                                     GLsizei drawcount, const GLint* baseVertices) {
        PrepareForDraw();
        for (GLsizei i = 0; i < drawcount; ++i) {
            MG_External::GLES::glDrawElementsBaseVertex(mode, counts[i], type, indices[i], baseVertices[i]);
        }
    }

    void BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1,
                         GLint dstY1, GLbitfield mask, GLenum filter) {
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        RenderStateImpl::SyncRenderState();

        BindCurrentFBO(FramebufferTarget::Draw);
        BindCurrentFBO(FramebufferTarget::Read);

        MG_External::GLES::glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    }

    const GLubyte* GetString(GLenum name) {
        return MG_External::GLES::glGetString(name);
    }
} // namespace MobileGL::MG_Backend::DirectGLES
