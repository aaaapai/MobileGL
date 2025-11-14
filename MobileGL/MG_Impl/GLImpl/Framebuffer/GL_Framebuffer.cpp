#include "GL_Framebuffer.h"
#include "Validators.h"
#include "Config.h"
#include "MG_Util/Converters/GLToStr/GLEnumConverter.h"
#include <MG_Impl/GLImpl/Texture/Validators.h>
#include <MG_State/GLState/ErrorState/Error.h>
#include <MG_Util/Converters/GLToMG/FramebufferEnumConverter.h>
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
#include <MG_Backend/DirectGLES/DirectGLES.h>
#endif

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        void BlitFramebuffer_Backend(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0,
                                     GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::BlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask,
                                                    filter);
#endif
        }

        void SampleMaski_State(GLuint maskNumber, GLbitfield mask) {
            // TODO: implement
        }

        void RenderbufferStorageMultisample_State(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
                                                  GLsizei height) {
            // TODO: implement
        }

        void RenderbufferStorage_State(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
            // TODO: implement
        }

        GLboolean IsRenderbuffer_State(GLuint renderbuffer) {
            // TODO: implement
            return GL_FALSE;
        }

        GLboolean IsFramebuffer_State(GLuint framebuffer) {
            // TODO: implement
            return GL_FALSE;
        }

        void GetFramebufferAttachmentParameteriv_State(GLenum target, GLenum attachment, GLenum pname, GLint* params) {
            // TODO: implement
        }

        void GenerateMipmap_State(GLenum target) {
            // TODO: implement
        }

        void GenRenderbuffers_State(GLsizei n, GLuint* renderbuffers) {
            // TODO: implement
        }

        void GenFramebuffers_State(GLsizei n, GLuint* framebuffers) {
            if (n < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "GenFramebuffers_State", "n must be non-negative"));
                return;
            }
            auto framebuffersNames = MG_State::pGLContext->GenFramebufferNames(n);
            Copy(framebuffersNames.data(), framebuffers, framebuffersNames.size());
        }

        void FramebufferTextureLayer_State(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
            // TODO: implement
        }

        void FramebufferTexture3D_State(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level,
                                        GLint zoffset) {
            // TODO: implement
        }

        void FramebufferTexture2D_State(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,
                                        GLint level) {
            if (target == GL_FRAMEBUFFER) {
                target = GL_DRAW_FRAMEBUFFER;
            }

            if (attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
                FramebufferTexture2D_State(target, GL_DEPTH_ATTACHMENT, textarget, texture, level);
                FramebufferTexture2D_State(target, GL_STENCIL_ATTACHMENT, textarget, texture, level);
            }

            FramebufferAttachmentType attachmentType = MG_Util::ConvertGLEnumToFramebufferAttachmentType(attachment);
            FramebufferTarget framebufferTarget = MG_Util::ConvertGLEnumToFramebufferTarget(target);

            if (!FramebufferImpl::ValidateFramebufferAttachmentType(attachmentType)) return;
            if (!FramebufferImpl::ValidateFramebufferTarget(framebufferTarget)) return;
            if (!TextureImpl::ValidateTextureName(texture, true)) return;

            auto& bindingSlot = MG_State::pGLContext->GetFramebufferBindingSlot(framebufferTarget);
            auto framebufferObject = bindingSlot.GetBoundObject();
            if (!framebufferObject) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "FramebufferTexture2D_State",
                                                 "Framebuffer target is bound to no framebuffer object."));
                return;
            }

            if (texture == 0) {
                framebufferObject->Detach(attachmentType);
                return;
            }

            auto textureObject = MG_State::pGLContext->GetTextureObject(texture);
            if (!textureObject) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "FramebufferTexture2D_State",
                                                 std::format("Texture object {} is not valid.", texture)));
                return;
            }

            framebufferObject->AttachTexture(attachmentType, textureObject, textureObject ? level : 0);
        }

        void FramebufferTexture1D_State(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,
                                        GLint level) {
            // TODO: implement
        }

        void FramebufferTexture_State(GLenum target, GLenum attachment, GLuint texture, GLint level) {
            // TODO: implement
        }

        void FramebufferRenderbuffer_State(GLenum target, GLenum attachment, GLenum renderbuffertarget,
                                           GLuint renderbuffer) {
            // TODO: implement
        }

        void DrawBuffers_State(GLsizei n, const GLenum* bufs) {
            if (n < 0) {
                MG_State::pGLContext->RecordError(
                        ErrorCode::InvalidValue, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                                                              "`n` is less than 0."));
                return;
            } else if (n > MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS) {
                MG_State::pGLContext->RecordError(
                        ErrorCode::InvalidValue, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                                                              "`n` is greater than `GL_MAX_DRAW_BUFFERS`."));
                return;
            }

            // Get bound framebuffer
            auto& bindingSlot = MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw);
            auto fbo = bindingSlot.GetBoundObject();
            bool isDefaultFBO = (fbo == FramebufferImpl::pDefaultFramebufferInfo->defaultFBO);

            static int existenceMap[(SizeT)FramebufferAttachmentType::FramebufferAttachmentTypeCount] = {
                    -1
            };
            std::fill(existenceMap, existenceMap + (SizeT)FramebufferAttachmentType::FramebufferAttachmentTypeCount, -1);

            for (GLsizei i = 0; i < n; ++i) {
                auto attType = MG_Util::ConvertGLEnumToFramebufferAttachmentType(bufs[i]);

                // ------------------- Check validity begin ------------------------
                if (attType == FramebufferAttachmentType::Unknown) {
                    MG_State::pGLContext->RecordError(
                            ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                                                                  std::format("bufs[{}] = %s is not an accepted value.", i, MG_Util::ConvertGLEnumToString(bufs[i]))));
                    return;
                }

                if (isDefaultFBO &&
                    attType >= FramebufferAttachmentType::Color0 && attType <= FramebufferAttachmentType::Color31) {
                    MG_State::pGLContext->RecordError(
                            ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                                                                  std::format("FBO is default FBO, but bufs[{}] = {} is one of the `GL_COLOR_ATTACHMENTn` tokens.", i, MG_Util::ConvertGLEnumToString(bufs[i]))));
                    return;
                }

                if (!isDefaultFBO &&
                    attType >= FramebufferAttachmentType::FrontLeft && attType <= FramebufferAttachmentType::BackRight) {
                    MG_State::pGLContext->RecordError(
                            ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                                                                 std::format("FBO is not default FBO, but bufs[{}] = {} is anything other than `GL_NONE` or one of the `GL_COLOR_ATTACHMENTn` tokens.", i, MG_Util::ConvertGLEnumToString(bufs[i]))));
                    return;
                }

                if (attType != FramebufferAttachmentType::None && existenceMap[(SizeT)attType] >= 0) {
                    MG_State::pGLContext->RecordError(
                            ErrorCode::InvalidOperation, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                            std::format("a symbolic constant other than `GL_NONE` appears more than once in bufs. bufs[{}] == bufs[{}] == {}.", i, existenceMap[(SizeT)attType], MG_Util::ConvertGLEnumToString(bufs[i]))));
                    return;
                }

                existenceMap[(SizeT)attType] = i;

                if ((SizeT)attType > (SizeT)FramebufferAttachmentType::Color0 + MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS) {
                    MG_State::pGLContext->RecordError(
                            ErrorCode::InvalidOperation, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                                                                      std::format("bufs[{}] == {} indicates a color buffer that does not exist in the current GL context.", i, MG_Util::ConvertGLEnumToString(bufs[i]))));
                    return;
                }
                // ------------------------- Check validity end ----------------------------------
                fbo->SetDrawBuffer(i, attType);
            }
            for (GLsizei i = n; i < MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS; ++i) {
                fbo->SetDrawBuffer(i, FramebufferAttachmentType::None);
            }
        }

        void DeleteRenderbuffers_State(GLsizei n, const GLuint* renderbuffers) {
            // TODO: implement
        }

        void DeleteFramebuffers_State(GLsizei n, const GLuint* framebuffers) {
            if (n < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "DeleteFramebuffers_State",
                                                                          "n must be non-negative."));
                return;
            }

            if (!framebuffers) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "DeleteFramebuffers_State",
                                                                          "Framebuffer names array cannot be null."));
                return;
            }

            for (SizeT i = 0; i < static_cast<SizeT>(n); ++i) {
                Uint bufferName = framebuffers[i];
                if (bufferName == 0) continue;
                if (!FramebufferImpl::ValidateFramebufferName(bufferName)) continue;
                MG_State::pGLContext->MarkFramebufferObjectForDeletion(bufferName);
            }
        }

        GLenum CheckFramebufferStatus_State(GLenum target) {
            FramebufferTarget framebufferTarget = MG_Util::ConvertGLEnumToFramebufferTarget(target);
            if (!FramebufferImpl::ValidateFramebufferTarget(framebufferTarget)) return GL_FRAMEBUFFER_UNDEFINED;

            auto& bindingSlot = MG_State::pGLContext->GetFramebufferBindingSlot(framebufferTarget);
            auto framebufferObject = bindingSlot.GetBoundObject();
            if (!framebufferObject) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "CheckFramebufferStatus_State",
                                                 "Framebuffer target is bound to no framebuffer object."));
                return GL_FRAMEBUFFER_UNDEFINED;
            }

            // TODO: distinguish GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT and GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
            // TODO: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER, GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
            //       GL_FRAMEBUFFER_UNSUPPORTED, GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
            //       GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
            return framebufferObject->CheckCompleteness() ? GL_FRAMEBUFFER_COMPLETE
                                                          : GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
        }

        void BindRenderbuffer_State(GLenum target, GLuint renderbuffer) {
            // TODO: implement
        }

        void BindFramebuffer_State(GLenum target, GLuint framebuffer) {
            if (target == GL_FRAMEBUFFER) {
                BindFramebuffer_State(GL_DRAW_FRAMEBUFFER, framebuffer);
                BindFramebuffer_State(GL_READ_FRAMEBUFFER, framebuffer);
                return;
            }

            if (!FramebufferImpl::ValidateFramebufferName(framebuffer)) return;
            FramebufferTarget framebufferTarget = MG_Util::ConvertGLEnumToFramebufferTarget(target);
            if (!FramebufferImpl::ValidateFramebufferTarget(framebufferTarget)) return;

            auto framebufferObject = MG_State::pGLContext->GetFramebufferObject(framebuffer);
            if (!framebufferObject) {
                MG_State::pGLContext->CreateFramebufferObject(framebuffer);
                framebufferObject = MG_State::pGLContext->GetFramebufferObject(framebuffer);
            }

            auto& bindingSlot = MG_State::pGLContext->GetFramebufferBindingSlot(framebufferTarget);
            bindingSlot.Bind(framebufferObject);
        }

        /* @INSERTION_POINT:FUNCTION_IMPLEMENTATION@ */
        void SampleMaski(GLuint maskNumber, GLbitfield mask) {
            SampleMaski_State(maskNumber, mask);
        }

        void RenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
                                            GLsizei height) {
            RenderbufferStorageMultisample_State(target, samples, internalformat, width, height);
        }

        void RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
            RenderbufferStorage_State(target, internalformat, width, height);
        }

        GLboolean IsRenderbuffer(GLuint renderbuffer) {
            return IsRenderbuffer_State(renderbuffer);
        }

        GLboolean IsFramebuffer(GLuint framebuffer) {
            return IsFramebuffer_State(framebuffer);
        }

        void GetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params) {
            GetFramebufferAttachmentParameteriv_State(target, attachment, pname, params);
        }

        void GenerateMipmap(GLenum target) {
            GenerateMipmap_State(target);
        }

        void GenRenderbuffers(GLsizei n, GLuint* renderbuffers) {
            GenRenderbuffers_State(n, renderbuffers);
        }

        void GenFramebuffers(GLsizei n, GLuint* framebuffers) {
            GenFramebuffers_State(n, framebuffers);
        }

        void FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
            FramebufferTextureLayer_State(target, attachment, texture, level, layer);
        }

        void FramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level,
                                  GLint zoffset) {
            FramebufferTexture3D_State(target, attachment, textarget, texture, level, zoffset);
        }

        void FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
            FramebufferTexture2D_State(target, attachment, textarget, texture, level);
        }

        void FramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
            FramebufferTexture1D_State(target, attachment, textarget, texture, level);
        }

        void FramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level) {
            FramebufferTexture_State(target, attachment, texture, level);
        }

        void FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
            FramebufferRenderbuffer_State(target, attachment, renderbuffertarget, renderbuffer);
        }

        void DrawBuffers(GLsizei n, const GLenum* bufs) {
            DrawBuffers_State(n, bufs);
        }

        void DeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers) {
            DeleteRenderbuffers_State(n, renderbuffers);
        }

        void DeleteFramebuffers(GLsizei n, const GLuint* framebuffers) {
            DeleteFramebuffers_State(n, framebuffers);
        }

        GLenum CheckFramebufferStatus(GLenum target) {
            return CheckFramebufferStatus_State(target);
        }

        void BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1,
                             GLint dstY1, GLbitfield mask, GLenum filter) {
            BlitFramebuffer_Backend(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
        }

        void BindRenderbuffer(GLenum target, GLuint renderbuffer) {
            BindRenderbuffer_State(target, renderbuffer);
        }

        void BindFramebuffer(GLenum target, GLuint framebuffer) {
            BindFramebuffer_State(target, framebuffer);
        }

        namespace FramebufferImpl {
            DefaultFramebufferInfo* pDefaultFramebufferInfo;
        }
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
