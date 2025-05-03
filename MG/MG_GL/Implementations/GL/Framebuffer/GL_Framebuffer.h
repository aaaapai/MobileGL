//
// Created by BZLZHH on 2025/3/15.
//

#ifndef MOBILEGL_GL_FRAMEBUFFER_H
#define MOBILEGL_GL_FRAMEBUFFER_H

#include "../../../../Includes.h"

namespace MG_GL::GL {
    void GenFramebuffers(GLsizei n, GLuint* framebuffers);
    void DeleteFramebuffers(GLsizei n, const GLuint* framebuffers);
    void BindFramebuffer(GLenum target, GLuint framebuffer);
    void FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    GLenum CheckFramebufferStatus(GLenum target);
}

#endif //MOBILEGL_GL_FRAMEBUFFER_H
