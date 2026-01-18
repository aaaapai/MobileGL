// MobileGL - MobileGL/MG_Impl/GLImpl/VertexArray/GL_VertexArray.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
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
