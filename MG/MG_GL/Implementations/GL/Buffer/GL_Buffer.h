//
// Created by BZLZHH on 2025/3/15.
//

#ifndef MOBILEGL_GL_BUFFER_H
#define MOBILEGL_GL_BUFFER_H

#include "../../../../Includes.h"

namespace MG_GL::GL {
    void* MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
    void FlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length);
    void CopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    void* MapBuffer(GLenum target, GLenum access);
    GLboolean UnmapBuffer(GLenum target);
    void BindBuffer(GLenum target, GLuint buffer);
    void BufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
    void BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
    void GetBufferParameteriv(GLenum target, GLenum pname, GLint* params);
    void GenBuffers(GLsizei n, GLuint* buffers);
    void DeleteBuffers(GLsizei n, const GLuint *buffers);
    GLboolean IsBuffer(GLuint buffer);
}

#endif //MOBILEGL_GL_BUFFER_H
