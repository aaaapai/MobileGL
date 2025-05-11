//
// Created by BZLZHH on 2025/5/1.
//

// TODO: Add more gl error check for vertex array state manager.

#include "VertexArrayState.h"

VertexArrayState::VertexArrayState() {
    vaos_[0];
}

GLenum VertexArrayState::GenName(GLuint *array) {
    MG_Util::Debug::LogD("MG_State: VAO: GenName");
    if (!array)
        return GL_INVALID_VALUE;
    GLuint id;
    if (freeIds_.empty()) {
        id = ++lastId_;
    } else {
        id = *freeIds_.begin();
        freeIds_.erase(freeIds_.begin());
    }
    *array = id;
    MG_Util::Debug::LogD("MG_State: VAO: Generated new name %d", id);
    return GL_NO_ERROR;
}

GLenum VertexArrayState::GenNameN(GLsizei n, GLuint* arrays) {
    MG_Util::Debug::LogD("MG_State: VAO: GenNameN called with n=%d", n);
    if (n < 0)
        return GL_INVALID_VALUE;
    for (GLsizei i = 0; i < n; ++i) {
        GLenum result = GenName(&arrays[i]);
        if (result != GL_NO_ERROR) {
            MG_Util::Debug::LogE("MG_State: VAO: GenNameN failed at index %d with error 0x%x", i, result);
            return result;
        }
    }
    MG_Util::Debug::LogD("MG_State: VAO: GenNameN created %d names", n);
    return GL_NO_ERROR;
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
    MG_Util::Debug::LogD("MG_State: VAO: Bind called for %u", array);
    if (array != 0) {
        if (!ValidateGeneratedName(array)) {
            MG_Util::Debug::LogE("MG_State: VAO: Bind invalid name %u", array);
            return GL_INVALID_OPERATION;
        }
        auto& vao = vaos_[array];
        if (!vao.generated) {
            MG_Util::Debug::LogD("MG_State: VAO: Creating VAO %u during bind", array);
            vao.generated = true;
        }
    }
    currentVao_ = array;
    MG_Util::Debug::LogD("MG_State: VAO: Bound to %u", array);
    return GL_NO_ERROR;
}

bool VertexArrayState::ValidateGeneratedName(GLuint array) {
    if (array == 0)
        return true;
    bool inFreeList = freeIds_.count(array) > 0;
    bool valid = (array <= lastId_) && !inFreeList;
    MG_Util::Debug::LogD("MG_State: VAO: ValidateGeneratedName %u: %d", array, valid);
    return valid;
}

bool VertexArrayState::ValidateAllocatedHandle(GLuint array) {
    bool exists = vaos_.count(array) && vaos_[array].generated;
    MG_Util::Debug::LogD("MG_State: VAO: ValidateAllocatedHandle %u: %d", array, exists);
    return exists;
}

GLenum VertexArrayState::EnableAttrib(GLuint index) {
    if (!ValidateAllocatedHandle(currentVao_))
        return GL_INVALID_OPERATION;
    if (index >= GL_MAX_VERTEX_ATTRIBS) return GL_INVALID_VALUE;
    GetCurrentVAO()->attribs[index].enabled = true;
    MG_Util::Debug::LogD("Attrib vaos_[%u].attribs[%u].enabled = %d", currentVao_, index, vaos_[currentVao_].attribs[index].enabled);
    return GL_NO_ERROR;
}

GLenum VertexArrayState::DisableAttrib(GLuint index) {
    if (!ValidateAllocatedHandle(currentVao_))
        return GL_INVALID_OPERATION;
    if (index >= GL_MAX_VERTEX_ATTRIBS) return GL_INVALID_VALUE;
    GetCurrentVAO()->attribs[index].enabled = false;
    MG_Util::Debug::LogD("Attrib vaos_[%u].attribs[%u].enabled = %d", currentVao_, index, vaos_[currentVao_].attribs[index].enabled);
    return GL_NO_ERROR;
}

GLenum VertexArrayState::SetAttribPointer(GLuint index, GLint size, GLenum type,
                                          GLboolean normalized, GLsizei stride,
                                          const void* pointer, bool isInteger,
                                          GLuint currentArrayBuffer) {
    if (!ValidateAllocatedHandle(currentVao_))
        return GL_INVALID_OPERATION;
    if (index >= GL_MAX_VERTEX_ATTRIBS) return GL_INVALID_VALUE;

    VertexAttribState state;
    state.size = size;
    state.type = type;
    state.normalized = normalized;
    state.stride = stride;
    state.pointer = pointer;
    state.buffer = currentArrayBuffer;
    state.isInteger = isInteger;
    if (vaos_[currentVao_].attribs.count(index) && vaos_[currentVao_].attribs[index].enabled)
        state.enabled = true;

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