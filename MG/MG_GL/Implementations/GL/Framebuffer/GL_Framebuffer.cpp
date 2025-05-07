//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_Framebuffer.h"

namespace MG_GL::GL {
    void GenFramebuffers(GLsizei n, GLuint* framebuffers) {
        MG_Util::Debug::LogD("glGenFramebuffers, n: %d, framebuffers: %p", n, framebuffers);
        GLenum result = MG_State::CreateFramebuffers(n, framebuffers);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Framebuffer generation failed: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void DeleteFramebuffers(GLsizei n, const GLuint* framebuffers) {
        MG_Util::Debug::LogD("glDeleteFramebuffers, n: %d, framebuffers: %p", n, framebuffers);
        for (GLsizei i = 0; i < n; ++i) {
            GLenum result = MG_State::DeleteFramebuffer(framebuffers[i]);
            if (result != GL_NO_ERROR) {
                MG_State::SetError(result);
                MG_Util::Debug::LogE("Failed to delete framebuffer %u: %s",
                                     framebuffers[i], MG_Util::Debug::GLEnumToString(result));
            }
        }
    }

    void BindFramebuffer(GLenum target, GLuint framebuffer) {
        MG_Util::Debug::LogD("glBindFramebuffer, target: %s, fb: %u",
                             MG_Util::Debug::GLEnumToString(target), framebuffer);
        GLenum result = MG_State::BindFramebuffer(target, framebuffer);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Framebuffer bind error: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget,
                              GLuint texture, GLint level) {
        MG_Util::Debug::LogD("glFramebufferTexture2D, target: %s, attach: %s, textarget: %s, tex: %u, level: %d",
                             MG_Util::Debug::GLEnumToString(target),
                             MG_Util::Debug::GLEnumToString(attachment),
                             MG_Util::Debug::GLEnumToString(textarget),
                             texture, level);

        GLenum result = MG_State::AttachTexture2DToFramebuffer(
                target, attachment, textarget, texture, level
        );

        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Texture attachment failed: %s", MG_Util::Debug::GLEnumToString(result));
    }

    GLenum CheckFramebufferStatus(GLenum target) {
        MG_Util::Debug::LogD("glCheckFramebufferStatus, target: %s",
                             MG_Util::Debug::GLEnumToString(target));
        GLenum result = MG_State::ValidateFramebufferCompleteness(target);
        if (result >= GL_FRAMEBUFFER_COMPLETE) {
            MG_Util::Debug::LogD("Framebuffer status: %s",
                                 MG_Util::Debug::GLEnumToString(result));
            return result;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Framebuffer incomplete: %s",
                             MG_Util::Debug::GLEnumToString(result));
        return result;
    }
}