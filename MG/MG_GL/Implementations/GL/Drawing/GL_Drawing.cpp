//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_Drawing.h"
#include "../../../../MG_RHI/GLES/RHI.h"

namespace MG_GL::GL {
    // This is a test function that directly draws textures.
    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices) {
//        ::GLES::glViewport(0,0,3200,1440);
//        MG_RHI::GLES::Test::DrawAllTextures();
        VertexArrayState* vaState = MG_State_T::vertexArrayState;
        VertexArrayObject* vao = &vaState->vaos_[vaState->currentVao_];

        auto& rhi = MG_RHI::GLES::RHI::GetInstance();
        rhi.TransitionTo(MG_RHI::GLES::State::DrawElements);

        if (vao->elementBuffer != 0 && indices != nullptr) {
            static GLuint dynamicIBO = 0;
            if (dynamicIBO == 0) {
                MGL_TRY(::GLES::glGenBuffers(1, &dynamicIBO);)
            }

            size_t typeSize = 0;
            switch(type) {
                case GL_UNSIGNED_BYTE:  typeSize = sizeof(GLubyte); break;
                case GL_UNSIGNED_SHORT: typeSize = sizeof(GLushort); break;
                case GL_UNSIGNED_INT:   typeSize = sizeof(GLuint); break;
            }
            size_t dataSize = count * typeSize;

            MGL_TRY(::GLES::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dynamicIBO);)
            MGL_TRY(::GLES::glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataSize, indices, GL_STREAM_DRAW);)
        }

        if (vao->elementBuffer != 0 || indices != nullptr) {
            MGL_TRY(::GLES::glDrawElements(
                    mode,
                    count,
                    type,
                    indices
            );)
        }
    };
    void DrawArrays(GLenum mode, GLint first, GLsizei count) {
        ::GLES::glViewport(0,0,3200,1440);
        MG_RHI::GLES::Test::DrawAllTextures();
    };
}