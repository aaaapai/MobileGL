//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_Drawing.h"

namespace MG_GL::GL {
    // This is a test function that directly draws textures.
    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices) {
        ::GLES::glViewport(0,0,3200,1440);
        MG_RHI::GLES::Test::DrawAllTextures();
    };
    void DrawArrays(GLenum mode, GLint first, GLsizei count) {
        ::GLES::glViewport(0,0,3200,1440);
        MG_RHI::GLES::Test::DrawAllTextures();
    };
}