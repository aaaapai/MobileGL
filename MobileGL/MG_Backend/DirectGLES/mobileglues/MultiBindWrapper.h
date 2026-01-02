#pragma once
#define GL_GLEXT_PROTOTYPES
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>
#include <Includes.h>

extern "C" {
    GLAPI void glBindTextures(GLuint first, GLsizei count, const GLuint* textures);
    GLAPI void glBindSamplers(GLuint first, GLsizei count, const GLuint* samplers);
    GLAPI void glBindImageTextures(GLuint first, GLsizei count, const GLuint* textures);
    GLAPI void glBindVertexBuffers(GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides);
}
