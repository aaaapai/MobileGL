//
// Created by BZLZHH on 2025/3/15.
//

#ifndef MOBILEGL_GL_FRAMEBUFFER_H
#define MOBILEGL_GL_FRAMEBUFFER_H

#include "../../../../Includes.h"

namespace MG_GL::GL {
    void UpdateDefaultFramebuffer();

    void GenFramebuffers(GLsizei n, GLuint* framebuffers);
    void DeleteFramebuffers(GLsizei n, const GLuint* framebuffers);
    void BindFramebuffer(GLenum target, GLuint framebuffer);
    void FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    GLenum CheckFramebufferStatus(GLenum target);
    void BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
}

#endif //MOBILEGL_GL_FRAMEBUFFER_H
