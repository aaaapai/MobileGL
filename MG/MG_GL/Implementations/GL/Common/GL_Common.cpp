//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_Common.h"

namespace MG_GL::GL {
    void Enable(GLenum cap) {
        MG_Util::Debug::LogD("glEnable, cap: %s", MG_Util::Debug::GLEnumToString(cap));
        GLenum result = MG_State::glEnable(cap);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error enabling capability: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void Disable(GLenum cap) {
        MG_Util::Debug::LogD("glDisable, cap: %s", MG_Util::Debug::GLEnumToString(cap));
        GLenum result = MG_State::glDisable(cap);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error disabling capability: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void BlendFunc(GLenum sfactor, GLenum dfactor) {
        MG_Util::Debug::LogD("glBlendFunc, sfactor: %s, dfactor: %s",
                             MG_Util::Debug::GLEnumToString(sfactor),
                             MG_Util::Debug::GLEnumToString(dfactor));
        GLenum result = MG_State::glBlendFunc(sfactor, dfactor);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting blend func: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
        MG_Util::Debug::LogD("glBlendFuncSeparate, srcRGB: %s, dstRGB: %s, srcAlpha: %s, dstAlpha: %s",
                             MG_Util::Debug::GLEnumToString(srcRGB),
                             MG_Util::Debug::GLEnumToString(dstRGB),
                             MG_Util::Debug::GLEnumToString(srcAlpha),
                             MG_Util::Debug::GLEnumToString(dstAlpha));
        GLenum result = MG_State::glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting separate blend func: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void Clear(GLbitfield mask) {
        MG_Util::Debug::LogD("glClear, mask: 0x%X", mask);
        GLenum result = MG_State::glClear(mask);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error clearing buffers: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
        MG_Util::Debug::LogD("glClearColor, rgba: [%.2f, %.2f, %.2f, %.2f]", red, green, blue, alpha);
        GLenum result = MG_State::glClearColor(red, green, blue, alpha);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting clear color: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void ClearDepth(GLdouble depth) {
        MG_Util::Debug::LogD("glClearDepth, depth: %.3f", depth);
        GLenum result = MG_State::glClearDepth(depth);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting clear depth: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
        MG_Util::Debug::LogD("glColorMask, rgba: [%d, %d, %d, %d]", red, green, blue, alpha);
        GLenum result = MG_State::glColorMask(red, green, blue, alpha);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting color mask: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void DepthFunc(GLenum func) {
        MG_Util::Debug::LogD("glDepthFunc, func: %s", MG_Util::Debug::GLEnumToString(func));
        GLenum result = MG_State::glDepthFunc(func);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting depth func: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void DepthMask(GLboolean flag) {
        MG_Util::Debug::LogD("glDepthMask, flag: %d", flag);
        GLenum result = MG_State::glDepthMask(flag);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting depth mask: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void Viewport(GLint x, GLint y, GLsizei width, GLsizei height) {
        MG_Util::Debug::LogD("glViewport, x: %d, y: %d, width: %d, height: %d",
                             x, y, width, height);
        GLenum result = MG_State::glViewport(x, y, width, height);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting viewport: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void PixelStorei(GLenum pname, GLint param) {
        MG_Util::Debug::LogD("glPixelStorei, pname: %d, param: %d", pname, param);
        GLenum result = MG_State::SetPixelStoreInt(pname,param);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }   
}