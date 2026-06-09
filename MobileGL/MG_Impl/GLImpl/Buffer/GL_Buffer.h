// MobileGL - MobileGL/MG_Impl/GLImpl/Buffer/GL_Buffer.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MobileGL::MG_Impl::GLImpl {
    /* @INSERTION_POINT:FUNCTION_DECLARATION@ */
    void GetBufferParameteriv(GLenum target, GLenum pname, GLint* params);
    void GetBufferParameteri64v(GLenum target, GLenum pname, GLint64* params);
    void GetBufferPointerv(GLenum target, GLenum pname, void** params);
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
    void BindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);

} // namespace MobileGL::MG_Impl::GLImpl
