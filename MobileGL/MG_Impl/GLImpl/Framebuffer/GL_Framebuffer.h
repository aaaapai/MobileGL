#pragma once
#include "MG_State/GLState/FramebufferState/FramebufferObject.h"
#include <Includes.h>
#include <MG_State/GLState/Core.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        /* @INSERTION_POINT:FUNCTION_DECLARATION@ */
        void SampleMaski(GLuint maskNumber, GLbitfield mask);
        void RenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
                                            GLsizei height);
        void RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
        GLboolean IsRenderbuffer(GLuint renderbuffer);
        GLboolean IsFramebuffer(GLuint framebuffer);
        void GetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params);
        void GenerateMipmap(GLenum target);
        void GenRenderbuffers(GLsizei n, GLuint* renderbuffers);
        void GenFramebuffers(GLsizei n, GLuint* framebuffers);
        void FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
        void FramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level,
                                  GLint zoffset);
        void FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
        void FramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
        void FramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level);
        void FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
        void DrawBuffers(GLsizei n, const GLenum* bufs);
        void DeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers);
        void DeleteFramebuffers(GLsizei n, const GLuint* framebuffers);
        GLenum CheckFramebufferStatus(GLenum target);
        void BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1,
                             GLint dstY1, GLbitfield mask, GLenum filter);
        void BindRenderbuffer(GLenum target, GLuint renderbuffer);
        void BindFramebuffer(GLenum target, GLuint framebuffer);

        namespace FramebufferImpl {
            struct DefaultFramebufferInfo {
                SharedPtr<MG_State::GLState::FramebufferObject> defaultFBO;
                SharedPtr<MG_State::GLState::ITextureObject> colorAttachment;
                SharedPtr<MG_State::GLState::ITextureObject> depthAttachment;
                SharedPtr<MG_State::GLState::ITextureObject> stencilAttachment;
            };

            extern DefaultFramebufferInfo* pDefaultFramebufferInfo;
        } // namespace FramebufferImpl
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
