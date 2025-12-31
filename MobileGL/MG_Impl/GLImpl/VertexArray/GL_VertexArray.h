// MobileGL - MobileGL/MG_Impl/GLImpl/VertexArray/GL_VertexArray.h
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        /* @INSERTION_POINT:FUNCTION_DECLARATION@ */
        void VertexAttribDivisor(GLuint index, GLuint divisor);
        GLboolean IsVertexArray(GLuint array);
        void DisableVertexAttribArray(GLuint index);
        void EnableVertexAttribArray(GLuint index);
        void VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);
        void VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
                                 const void* pointer);
        void BindVertexArray(GLuint array);
        void DeleteVertexArrays(GLsizei n, const GLuint* arrays);
        void GenVertexArrays(GLsizei n, GLuint* arrays);
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
