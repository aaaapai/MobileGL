//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_Texture.h"

namespace MG_GL::GL {
    void ActiveTexture(GLenum texture) {
        MG_Util::Debug::LogD("glActiveTexture, texture: %d", texture);
        GLenum result = MG_State::BindTextureUnit(texture);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void BindTexture(GLenum target, GLuint texture) {
        MG_Util::Debug::LogD("glBindTexture, target: %d, texture: %d", target, texture);
        GLenum result = MG_State::BindTexture(target, texture);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void DeleteTextures(GLsizei n, const GLuint* textures) {
        MG_Util::Debug::LogD("glDeleteTextures, n: %d, textures: %p", n, textures);
        GLenum result = MG_State::DeleteTextures(n, textures);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void GenTextures(GLsizei n, GLuint* textures) {
        MG_Util::Debug::LogD("glGenTextures, n: %d, textures: %p", n, textures);
        GLenum result = MG_State::CreateTextures(n, textures);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void TexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, 
                    GLenum format, GLenum type, const void* data) {
        MG_Util::Debug::LogD("glTexImage2D, target: %d, level: %d, internalFormat: %d, width: %d, height: %d, format: %d, type: %d, data: %p",
                             target, level, internalFormat, width, height, format, type, data);
        GLenum result = MG_State::UploadTexture2D(target, level, internalFormat, width, height, border, format, type, data);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void TexParameterf(GLenum target, GLenum pname, GLfloat param) {
        MG_Util::Debug::LogD("glTexParameterf, target: %d, pname: %d, param: %f", target, pname, param);
        GLenum result = MG_State::SetTexturePropertyFloat(target, pname, param);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void TexParameteri(GLenum target, GLenum pname, GLint param) {
        MG_Util::Debug::LogD("glTexParameteri, target: %d, pname: %d, param: %d", target, pname, param);
        GLenum result = MG_State::SetTexturePropertyInt(target, pname, param);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                       GLsizei height, GLenum format, GLenum type, const void* pixels) {
        MG_Util::Debug::LogD("glTexSubImage2D, target: %d, level: %d, xoffset: %d, yoffset: %d, width: %d, height: %d, format: %d, type: %d, pixels: %p",
                             target, level, xoffset, yoffset, width, height, format, type, pixels);
        GLenum result = MG_State::UpdateTextureRegion2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void GetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params) {
        MG_Util::Debug::LogD("glGetTexLevelParameteriv, target: %d, level: %d, pname: %d, params: %p",
                             target, level, pname, params);
        GLenum result = MG_State::QueryTextureLevelPropertyIntVector(target, level, pname, params);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }
}