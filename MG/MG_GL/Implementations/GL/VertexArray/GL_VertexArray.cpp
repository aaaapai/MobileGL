//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_VertexArray.h"

namespace MG_GL::GL {
    void GenVertexArrays(GLsizei n, GLuint* arrays) {
        MG_Util::Debug::LogD("glGenVertexArrays, n: %d, arrays: %p", n, arrays);
        GLenum result = MG_State::CreateVertexArrays(n, arrays);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void BindVertexArray(GLuint array) {
        MG_Util::Debug::LogD("glBindVertexArray, array: %u", array);
        GLenum result = MG_State::BindVertexArray(array);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void EnableVertexAttribArray(GLuint index) {
        MG_Util::Debug::LogD("glEnableVertexAttribArray, index: %u", index);
        GLenum result = MG_State::EnableVertexAttribArray(index);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void VertexAttribPointer(GLuint index, GLint size, GLenum type,
                             GLboolean normalized, GLsizei stride, const void* pointer) {
        MG_Util::Debug::LogD("glVertexAttribPointer, index: %u, size: %d, type: %s, norm: %d, stride: %d, ptr: %p",
                             index, size, MG_Util::Debug::GLEnumToString(type), normalized, stride, pointer);

        GLenum result = MG_State::SetVertexAttributeLayout(index, size, type, normalized, stride, pointer);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void VertexAttribIPointer(GLuint index, GLint size, GLenum type,
                              GLsizei stride, const void* pointer) {
        MG_Util::Debug::LogD("glVertexAttribIPointer, index: %u, size: %d, type: %s, stride: %d, ptr: %p",
                             index, size, MG_Util::Debug::GLEnumToString(type), stride, pointer);

        GLenum result = MG_State::SetVertexAttributeLayoutInt(index, size, type, stride, pointer);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }
}