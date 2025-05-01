//
// Created by BZLZHH on 2025/5/1.
//

// TODO: Add more gl error check for buffer state manager.

#include "BufferState.h"

GLenum BufferState::Create(GLuint* buffer) {
    MG_Util::Debug::LogD("MG_State: Buffer: Create called");
    if (!buffer) return GL_INVALID_VALUE;

    GLuint id = 0;
    if (!freeIds_.empty()) {
        id = *freeIds_.begin();
        freeIds_.erase(freeIds_.begin());
    } else {
        id = ++lastId_;
    }
    *buffer = id;
    BufferObject& obj = buffers_[id];
    MG_Util::Debug::LogD("MG_State: Buffer: Create created buffer %d", id);
    obj.generated = true;

    return GL_NO_ERROR;
}

GLenum BufferState::CreateN(GLsizei n, GLuint* buffers) {
    MG_Util::Debug::LogD("MG_State: Buffer: CreateN called with n=%d", n);
    if (n < 0) return GL_INVALID_VALUE;

    for (GLsizei i = 0; i < n; ++i) {
        GLenum result = Create(&buffers[i]);
        if (result != GL_NO_ERROR) {
            MG_Util::Debug::LogE("MG_State: Buffer: CreateN create buffer failed with error 0x%x", result);
            return result;
        }
    }
    MG_Util::Debug::LogD("MG_State: Buffer: CreateN created buffers successfully");
    return GL_NO_ERROR;
}

GLenum BufferState::Bind(GLenum target, GLuint buffer) {
    if (!IsValidTarget_(target)) return GL_INVALID_ENUM;
    MG_Util::Debug::LogD("MG_State: Buffer: Bind called with target=0x%x, buffer=%u", target, buffer);

    if (buffer != 0) {
        auto it = buffers_.find(buffer);
        if (it == buffers_.end()) {
            buffers_[buffer];
        } else {
            BufferObject& obj = it->second;
            if (obj.target != 0 && obj.target != target) {
                MG_Util::Debug::LogE("MG_State: Buffer: Bind can not bind a buffer to different target 0x%x, current bind target is 0x%x", target, obj.target);
                return GL_INVALID_OPERATION;
            }
        }
        buffers_[buffer].target = target;
    }

    currentBindings_[target] = buffer;
    MG_Util::Debug::LogD("MG_State: Buffer: Bind succeed bind buffer %u to target 0x%x", buffer, target);
    return GL_NO_ERROR;
}

GLenum BufferState::CommitStorage(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
    MG_Util::Debug::LogD("MG_State: Buffer: CommitStorage called on target 0x%x",target);
    auto it = currentBindings_.find(target);
    if (it == currentBindings_.end() || it->second == 0)
        return GL_INVALID_OPERATION;

    BufferObject& obj = buffers_[it->second];
    MG_Util::Debug::LogD("MG_State: Buffer: CommitStorage get buffer object %u at target 0x%x",it->second,target);
    obj.usage = usage;
    obj.data.resize(size);
    if (data) memcpy(obj.data.data(), data, size);
    MG_Util::Debug::LogD("MG_State: Buffer: CommitStorage buffer at target 0x%x committed storage, size = %zu, usage=0x%x", target, size, usage);

    return GL_NO_ERROR;
}

GLenum BufferState::AcquireBufferMemory(GLenum target, GLenum access, void** mappedPointer) {
    if (!IsValidTarget_(target)) return GL_INVALID_ENUM;
    if (MG_Constants::Buffer::VALID_ACCESS.find(access) == MG_Constants::Buffer::VALID_ACCESS.end()) {
        return GL_INVALID_ENUM;
        MG_Util::Debug::LogD("MG_State: Buffer: AcquireBufferMemory buffer at target 0x%x acquired mapped memory, access mode = 0x%x", target, access);
    }
    
    auto it = currentBindings_.find(target);
    if (it == currentBindings_.end() || it->second == 0)
        return GL_INVALID_OPERATION;

    BufferObject& obj = buffers_[it->second];
    if (obj.isMapped) return GL_INVALID_OPERATION;

    obj.isMapped = true;
    *mappedPointer = obj.data.data();
    obj.isMapped = true;
    obj.accessMode = access;

    return GL_NO_ERROR;
}

GLenum BufferState::ReleaseBufferMemory(GLenum target) {
    MG_Util::Debug::LogD("MG_State: Buffer: ReleaseBufferMemory called on target 0x%x",target);
    if (!IsValidTarget_(target)) return GL_INVALID_ENUM;

    
    auto it = currentBindings_.find(target);
    if (it == currentBindings_.end() || it->second == 0)
        return GL_INVALID_OPERATION;

    BufferObject& obj = buffers_[it->second];
    if (!obj.isMapped) return GL_INVALID_OPERATION;

    obj.isMapped = false;
    obj.accessMode = GL_READ_WRITE;
    MG_Util::Debug::LogD("MG_State: Buffer: ReleaseBufferMemory buffer at target 0x%x released mapped memory", target);
    return GL_NO_ERROR;
}

bool BufferState::ValidateHandle(GLuint buffer) {
    bool isvalid = buffers_.count(buffer) > 0;
    MG_Util::Debug::LogD("MG_State: Buffer: ValidateHandle called on buffer %u returns %d", buffer, isvalid);
    return isvalid;
}

void BufferState::Delete(GLuint buffer) {
    if (buffers_.erase(buffer)) {
        freeIds_.insert(buffer);
        for (auto& [target, id] : currentBindings_) {
            if (id == buffer) id = 0;
        }
    }
    MG_Util::Debug::LogD("MG_State: Buffer: Delete buffer %u", buffer);
}

bool BufferState::IsValidTarget_(GLenum target) {
    MG_Util::Debug::LogD("MG_State: Buffer: IsValidTarget_ called with target=0x%x,result=%d", target, !(MG_Constants::Buffer::VALID_TARGETS.find(target) == MG_Constants::Buffer::VALID_TARGETS.end()));
    return !(MG_Constants::Buffer::VALID_TARGETS.find(target) == MG_Constants::Buffer::VALID_TARGETS.end());
}

GLenum BufferState::QueryPropertyIntVector(GLenum target, GLenum pname, GLint* params) const {
    MG_Util::Debug::LogD("MG_State: Buffer: QueryPropertyIntVector called at target 0x%x,param name = 0x%x",target,pname);
    static const std::set<GLenum> validTargets = {
            GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER,
            GL_PIXEL_PACK_BUFFER, GL_PIXEL_UNPACK_BUFFER
    };
    if (validTargets.find(target) == validTargets.end()) {
        return GL_INVALID_ENUM;
    }
    
    if (MG_Constants::Buffer::VALID_PARAM_NAMES.find(pname) == MG_Constants::Buffer::VALID_PARAM_NAMES.end()) {
        return GL_INVALID_ENUM;
    }
    
    auto bindingIt = currentBindings_.find(target);
    if (bindingIt == currentBindings_.end() || bindingIt->second == 0) {
        return GL_INVALID_OPERATION;
    }
    GLuint bufferId = bindingIt->second;
    auto bufferIt = buffers_.find(bufferId);
    if (bufferIt == buffers_.end()) {
        return GL_INVALID_OPERATION;
    }
    
    const BufferObject& buffer = bufferIt->second;
    
    switch (pname) {
        case GL_BUFFER_ACCESS:
            *params = static_cast<GLint>(buffer.accessMode);
            break;
        case GL_BUFFER_MAPPED:
            *params = buffer.isMapped ? GL_TRUE : GL_FALSE;
            break;
        case GL_BUFFER_SIZE:
            *params = static_cast<GLint>(buffer.data.size());
            break;
        case GL_BUFFER_USAGE:
            *params = static_cast<GLint>(buffer.usage);
            break;
        MG_Util::Debug::LogD("MG_State: Buffer: QueryPropertyIntVector Query info about buffer %u succeed",buffer.target);
        default:
            return GL_INVALID_ENUM;
    }
    return GL_NO_ERROR;
}


GLuint BufferState::GetCurrentBinding(GLenum target) const {
    auto it = currentBindings_.find(target);
    if (it != currentBindings_.end()) {
        return it->second;
    }
    return 0;
}