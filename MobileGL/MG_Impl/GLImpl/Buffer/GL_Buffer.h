#pragma once
#include <Includes.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        /* @INSERTION_POINT:FUNCTION_DECLARATION@ */
        void GetBufferParameteriv(GLenum target, GLenum pname, GLint* params);
        GLboolean IsBuffer(GLuint buffer);
        void DeleteBuffers(GLsizei n, const GLuint* buffers);
        void FlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length);
        GLboolean UnmapBuffer(GLenum target);
        void* MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
        void* MapBuffer(GLenum target, GLenum access);
        void CopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset,
                               GLsizeiptr size);
        void BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
        void BufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
        void BindBuffer(GLenum target, GLuint buffer);
        void GenBuffers(GLsizei n, GLuint* buffers);
        void BindBufferBase(GLenum target, GLuint index, GLuint buffer);
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
