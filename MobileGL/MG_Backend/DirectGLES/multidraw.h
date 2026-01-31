#pragma once
#include <Includes.h>

namespace MobileGL::MG_Backend::DirectGLES {

void MultiDrawElementsBaseVertex_indirect(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei drawcount, const GLint* basevertex);
void MultiDrawElementsBaseVertex_compute(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei drawcount, const GLint* basevertex);

void MultiDrawElements_indirect(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei drawcount);
void MultiDrawElements_compute(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei drawcount);

}

MOBILEGL_GL_API void glMultiDrawElements_indirect(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei drawcount);

MOBILEGL_GL_API void glMultiDrawElements_compute(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei drawcount);

MOBILEGL_GL_API void glMultiDrawElementsBaseVertex_indirect(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei drawcount, const GLint* basevertex);

MOBILEGL_GL_API void glMultiDrawElementsBaseVertex_compute(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei drawcount, const GLint* basevertex);
