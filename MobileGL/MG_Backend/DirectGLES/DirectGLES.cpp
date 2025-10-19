#include "DirectGLES.h"
#include "Utils.h"
#include "Managers.h"
#include <MG_State/GLState/Core.h>
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
            //   1. textures bound to current texture units (TODO: only sync ones that are used in current program)
            //   2. textures used in current FBO
            //   3. textures bound to image units (TODO)

            Vector<SharedPtr<MG_State::GLState::ITextureObject>> texturesToSync;

            auto& currentUnit =
                MG_State::pGLContext->GetTextureUnitObject(MG_State::pGLContext->GetActiveTextureUnit());
            for (auto& bindingSlot : currentUnit.GetAllBindingSlots()) {
                const auto& textureObject = bindingSlot.GetBoundObject();
                if (textureObject) {
                    texturesToSync.push_back(textureObject);
                }
            }

            const auto& currentFBO =
                MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw).GetBoundObject();
            if (currentFBO) {
                for (auto& attachment : currentFBO->GetAllAttachments()) {
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
            /*
                void SetViewport(IntVec4 viewport); // x, y, width, height
                const IntVec4& GetViewport() const; // x, y, width, height
                void SetCapability(CapabilityInput cap, Bool enabled);
                Bool IsCapabilityEnabled(CapabilityInput cap) const;
                void SetBlendFunc(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha, BlendFactor dstAlpha);
                void GetBlendFunc(BlendFactor& srcRGB, BlendFactor& dstRGB, BlendFactor& srcAlpha,
                                  BlendFactor& dstAlpha) const;
                void SetDepthFunc(DepthTestFunc func);
                DepthTestFunc GetDepthFunc() const;
                void SetDepthMask(Bool flag);
                Bool GetDepthMask() const;
                void SetColorMask(BoolVec4 mask);
                const BoolVec4 GetColorMask() const;
                void SetClearColor(FloatVec4 color);
                const FloatVec4& GetClearColor() const;
                void SetClearDepth(Float depth);
                Float GetClearDepth() const;
                void SetPixelStoreParam(PixelStoreParam param, Int value);
                Int GetPixelStoreParam(PixelStoreParam param) const;
                void SetCullFaceMode(CullFaceMode mode);
                CullFaceMode GetCullFaceMode() const;*/

            MG_External::GLES::glViewport(
                MG_State::pGLContext->GetViewport().x(), MG_State::pGLContext->GetViewport().y(),
                MG_State::pGLContext->GetViewport().z(), MG_State::pGLContext->GetViewport().w());
            if (MG_State::pGLContext->IsCapabilityEnabled(CapabilityInput::DepthTest)) {
                MG_External::GLES::glEnable(GL_DEPTH_TEST);
            } else {
                MG_External::GLES::glDisable(GL_DEPTH_TEST);
            }
            if (MG_State::pGLContext->IsCapabilityEnabled(CapabilityInput::Blend)) {
                MG_External::GLES::glEnable(GL_BLEND);
            } else {
                MG_External::GLES::glDisable(GL_BLEND);
            }

            auto ToGLBoolean = [](Bool b) -> GLboolean { return b ? GL_TRUE : GL_FALSE; };

            {
                BlendFactor srcRGB, dstRGB, srcAlpha, dstAlpha;
                MG_State::pGLContext->GetBlendFunc(srcRGB, dstRGB, srcAlpha, dstAlpha);

                MG_External::GLES::glBlendFuncSeparate(
                    MG_Util::ConvertBlendFactorToGLEnum(srcRGB), MG_Util::ConvertBlendFactorToGLEnum(dstRGB),
                    MG_Util::ConvertBlendFactorToGLEnum(srcAlpha), MG_Util::ConvertBlendFactorToGLEnum(dstAlpha));
            }

            {
                DepthTestFunc df = MG_State::pGLContext->GetDepthFunc();
                MG_External::GLES::glDepthFunc(MG_Util::ConvertDepthTestFuncToGLEnum(df));

                MG_External::GLES::glDepthMask(MG_State::pGLContext->GetDepthMask() ? GL_TRUE : GL_FALSE);
            }

            {
                BoolVec4 colorMask = MG_State::pGLContext->GetColorMask();
                MG_External::GLES::glColorMask(ToGLBoolean(colorMask.x()), ToGLBoolean(colorMask.y()),
                                               ToGLBoolean(colorMask.z()), ToGLBoolean(colorMask.w()));
            }

            {
                const FloatVec4& clearCol = MG_State::pGLContext->GetClearColor();
                MG_External::GLES::glClearColor(clearCol.x(), clearCol.y(), clearCol.z(), clearCol.w());
                MG_External::GLES::glClearDepthf(MG_State::pGLContext->GetClearDepth());
            }

            {
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

            if (MG_State::pGLContext->IsCapabilityEnabled(CapabilityInput::CullFace)) {
                MG_External::GLES::glEnable(GL_CULL_FACE);
            } else {
                MG_External::GLES::glDisable(GL_CULL_FACE);
            }

            {
                CullFaceMode cfm = MG_State::pGLContext->GetCullFaceMode();
                MG_External::GLES::glCullFace(MG_Util::ConvertCullFaceModeToGLEnum(cfm));
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
            auto backendProgramIt = g_backendProgramObjects.find(currentProgram);
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
            backendProgram->Use();
        }
    } // namespace PrgramImpl

    void PrepareForDraw() {
        BufferImpl::SyncNeccessaryBuffers();
        VertexArrayImpl::SyncCurrentVAO();
        TextureImpl::SyncNeccessaryTextures();
        FramebufferImpl::SyncCurrentFBO();
        PrgramImpl::SyncCurrentProgram();
        RenderStateImpl::SyncRenderState();
    }

    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
        PrepareForDraw();
        MG_External::GLES::glDrawElements(mode, count, type, indices);
    }

    void Clear(GLbitfield mask) {
        FramebufferImpl::SyncCurrentFBO();
        RenderStateImpl::SyncRenderState();
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
} // namespace MobileGL::MG_Backend::DirectGLES
