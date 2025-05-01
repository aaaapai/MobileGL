//
// Created by BZLZHH on 2025/3/15.
//

#ifndef MOBILEGL_GL_VERTEXARRAY_H
#define MOBILEGL_GL_VERTEXARRAY_H

#include "../../../../Includes.h"

namespace MG_GL::GL {
    void GenVertexArrays(GLsizei n, GLuint* arrays);
    void BindVertexArray(GLuint array);
    void EnableVertexAttribArray(GLuint index);
    void VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
    void VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);
}

#endif //MOBILEGL_GL_VERTEXARRAY_H
