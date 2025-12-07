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
