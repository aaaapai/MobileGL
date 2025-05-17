//
// Created by BZLZHH on 2025/3/15.
//

#ifndef MOBILEGL_GL_DRAWING_H
#define MOBILEGL_GL_DRAWING_H
#include "../../../../Includes.h"

namespace MG_GL::GL {
    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
    void DrawArrays(GLenum mode, GLint first, GLsizei count);
    void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex);
    void MultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const GLvoid *const *indices, GLsizei drawcount);
    void MultiDrawElementsBaseVertex(GLenum mode, const GLsizei *count, GLenum type, const GLvoid *const *indices, GLsizei drawcount, const GLint *basevertex);
    void DrawBuffer(GLenum buf);
}

#endif //MOBILEGL_GL_DRAWING_H
