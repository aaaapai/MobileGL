//
// Created by BZLZHH on 2025/5/1.
//

// TODO: Add more gl error check for buffer state manager.

#include "VertexArrayState.h"

VertexArrayState::VertexArrayState() {
    vaos_[0];
}

GLenum VertexArrayState::Create(GLuint* array) {
    if (array == nullptr) return GL_INVALID_VALUE;

    GLuint id = freeIds_.empty() ? ++lastId_ : *freeIds_.begin();
    if (!freeIds_.empty()) {
        freeIds_.erase(freeIds_.begin());
    }

    *array = id;
    vaos_[id].generated = true;
    return GL_NO_ERROR;
}

GLenum VertexArrayState::CreateN(GLsizei n, GLuint* arrays) {
    if (n < 0) {
        return GL_INVALID_VALUE; 
    }

    if (n > 0 && arrays == nullptr) {
        return GL_INVALID_VALUE;
    }

    for (GLsizei i = 0; i < n; ++i) {
        if (GLenum error = Create(&arrays[i]); error != GL_NO_ERROR) {
            return error;
        }
    }
    return GL_NO_ERROR;
}

GLenum VertexArrayState::Bind(GLuint array) {
    if (array != 0 && !vaos_.count(array)) return GL_INVALID_OPERATION;
    currentVao_ = array;
    return GL_NO_ERROR;
}

GLenum VertexArrayState::EnableAttrib(GLuint index) {
    if (currentVao_ == 0) return GL_INVALID_OPERATION;
    if (index >= GL_MAX_VERTEX_ATTRIBS) return GL_INVALID_VALUE;
    vaos_[currentVao_].attribs[index].enabled = true;
    return GL_NO_ERROR;
}

GLenum VertexArrayState::DisableAttrib(GLuint index) {
    if (currentVao_ == 0) return GL_INVALID_OPERATION;
    if (index >= GL_MAX_VERTEX_ATTRIBS) return GL_INVALID_VALUE;
    vaos_[currentVao_].attribs[index].enabled = false;
    return GL_NO_ERROR;
}

GLenum VertexArrayState::SetAttribPointer(GLuint index, GLint size, GLenum type,
                                          GLboolean normalized, GLsizei stride,
                                          const void* pointer, bool isInteger,
                                          GLuint currentArrayBuffer) {
    if (currentVao_ == 0) return GL_INVALID_OPERATION;
    if (index >= GL_MAX_VERTEX_ATTRIBS) return GL_INVALID_VALUE;

    VertexAttribState state;
    state.size = size;
    state.type = type;
    state.normalized = normalized;
    state.stride = stride;
    state.pointer = pointer;
    state.buffer = currentArrayBuffer;
    state.isInteger = isInteger;

    vaos_[currentVao_].attribs[index] = state;
    return GL_NO_ERROR;
}

GLuint VertexArrayState::GetBoundElementBuffer() {
    auto it = vaos_.find(currentVao_);
    return (it != vaos_.end()) ? it->second.elementBuffer : 0;
}

VertexArrayObject* VertexArrayState::GetCurrentVAO() {
    auto it = vaos_.find(currentVao_);
    return (it != vaos_.end()) ? &it->second : &vaos_.at(0);
}