//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_Buffer.h"

namespace MG_GL::GL {
    void* MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length,
                         GLbitfield access) {
        MG_Util::Debug::LogD("glMapBufferRange, target: %s, offset: %lld, length: %lld, access: 0x%X",
                             MG_Util::Debug::GLEnumToString(target),
                             static_cast<long long>(offset),
                             static_cast<long long>(length),
                             access);
        void* mappedPointer = nullptr;
        GLenum result = MG_State::AcquireBufferMemoryRange(target, offset, length, access, &mappedPointer);

        if (result == GL_NO_ERROR) {
            MG_Util::Debug::LogD("Mapped pointer: %p", mappedPointer);
            return mappedPointer;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error mapping buffer range: %s", MG_Util::Debug::GLEnumToString(result));
        return nullptr;
    }

    void FlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length) {
        MG_Util::Debug::LogD("glFlushMappedBufferRange, target: %s, offset: %lld, length: %lld",
                             MG_Util::Debug::GLEnumToString(target),
                             static_cast<long long>(offset),
                             static_cast<long long>(length));
        GLenum result = MG_State::SyncBufferMemory(target, offset, length);
        if (result == GL_NO_ERROR) {
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error flushing mapped buffer range: %s",
                             MG_Util::Debug::GLEnumToString(result));
    }

    void CopyBufferSubData(GLenum readTarget, GLenum writeTarget,
                           GLintptr readOffset, GLintptr writeOffset,
                           GLsizeiptr size) {
        MG_Util::Debug::LogD("glCopyBufferSubData, readTarget: %s, writeTarget: %s, "
                             "readOffset: %lld, writeOffset: %lld, size: %lld",
                             MG_Util::Debug::GLEnumToString(readTarget),
                             MG_Util::Debug::GLEnumToString(writeTarget),
                             static_cast<long long>(readOffset),
                             static_cast<long long>(writeOffset),
                             static_cast<long long>(size));
        GLenum result = MG_State::CopyBufferRange(readTarget, writeTarget, readOffset, writeOffset, size);
        if (result == GL_NO_ERROR) {
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error copying buffer subdata: %s",
                             MG_Util::Debug::GLEnumToString(result));
    }
    
    void* MapBuffer(GLenum target, GLenum access) {
        MG_Util::Debug::LogD("glMapBuffer(target=0x%x, access=0x%x)", target, access);
        void* mappedPtr = nullptr;
        GLenum err = MG_State::AcquireBufferMemory(target, access, &mappedPtr);
        if (err != GL_NO_ERROR) {
            MG_State::SetError(err);
            MG_Util::Debug::LogE("glMapBuffer failed: %s", MG_Util::Debug::GLEnumToString(err));
            return nullptr;
        }
        MG_Util::Debug::LogD("glMapBuffer returns %p", mappedPtr);
        return mappedPtr;
    }
    
    GLboolean UnmapBuffer(GLenum target) {
        MG_Util::Debug::LogD("glUnmapBuffer(target=0x%x)", target);
        GLenum err = MG_State::ReleaseBufferMemory(target);
        if (err != GL_NO_ERROR) {
            MG_State::SetError(err);
            MG_Util::Debug::LogE("glUnmapBuffer failed: %s", MG_Util::Debug::GLEnumToString(err));
            return GL_FALSE;
        }
        MG_Util::Debug::LogD("glUnmapBuffer succeeded");
        return GL_TRUE;
    }

    void BindBuffer(GLenum target, GLuint buffer) {
        MG_Util::Debug::LogD("glBindBuffer, target: %s, buffer: %d", MG_Util::Debug::GLEnumToString(target), buffer);
        if (buffer != 0 &&
            MG_State::ValidateGeneratedName(buffer) &&
            !MG_State::ValidateAllocatedBufferHandle(buffer)) {
            MG_Util::Debug::LogD("Actually creating buffer: %u", buffer);
            GLenum result = MG_State::CreateBuffer(buffer);
            if (result != GL_NO_ERROR) {
                MG_State::SetError(result);
                MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
                return;
            }
        }

        if (buffer != 0 && !MG_State::ValidateAllocatedBufferHandle(buffer)) {
            MG_State::SetError(GL_INVALID_VALUE);
            MG_Util::Debug::LogE("Invalid buffer handle: %u", buffer);
            return;
        }
        GLenum result = MG_State::BindBuffer(target, buffer);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void BufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
        MG_Util::Debug::LogD("glBufferData, target: %s, size: %zd, data: %p, usage: %s",
                             MG_Util::Debug::GLEnumToString(target), size, data, MG_Util::Debug::GLEnumToString(usage));
        GLenum result = MG_State::CommitBufferStorage(target, size, data, usage);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void GetBufferParameteriv(GLenum target, GLenum pname, GLint* params) {
        MG_Util::Debug::LogD("glGetBufferParameteriv, target: %d, pname: %d, params: %p",
                             target, pname, params);
        GLenum result = MG_State::QueryBufferPropertyIntVector(target, pname, params);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("GetBufferParameteriv not implemented");
    }
    
    void GenBuffers(GLsizei n, GLuint* buffers) {
        MG_Util::Debug::LogD("glGenBuffers, n: %d, buffers: %p", n, buffers);

        if (n < 0) {
            MG_State::SetError(GL_INVALID_VALUE);
            MG_Util::Debug::LogE("Invalid buffer count: %d", n);
            return;
        }

        GLenum result = MG_State::GenBufferNames(n, buffers);
        if (result == GL_NO_ERROR) {
            MG_Util::Debug::LogD("Generated buffer names:");
            for (GLsizei i = 0; i < n; ++i) {
                MG_Util::Debug::LogD("  Buffer[%d] = %u", i, buffers[i]);
            }
            return;
        }

        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    GLboolean IsBuffer(GLuint buffer) {
        MG_Util::Debug::LogD("glIsBuffer, buffer: %u", buffer);

        if (buffer == 0) {
            return GL_FALSE;
        }

        bool isValid = MG_State::ValidateAllocatedBufferHandle(buffer);
        // Should we report gl error here or in MG_State? 
        MG_Util::Debug::LogD("Buffer %u is %s", buffer, isValid ? "valid" : "invalid");
        return isValid ? GL_TRUE : GL_FALSE;
    }

    void BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) {
        MG_Util::Debug::LogD("glBufferSubData, target: %s, offset: %lld, size: %lld, data: %p",
                             MG_Util::Debug::GLEnumToString(target),
                             static_cast<long long>(offset),
                             static_cast<long long>(size),
                             data);
        GLenum result = MG_State::CommitBufferStorageRegion(target, offset, size, data);
        if (result != GL_NO_ERROR) {
            MG_State::SetError(result);
            MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
        }
    }
    void DeleteBuffers(GLsizei n, const GLuint *buffers) {
        MG_Util::Debug::LogD("glDeleteBuffers, n: %d, buffers: %p", n, buffers);

        GLenum result = MG_State::DeleteBuffers(n, buffers);
        if (result == GL_NO_ERROR)
            return;

        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }
}