//
// Created by Swung 0x48 on 2025/5/6.
//

#ifndef MOBILEGL_RHI_H
#define MOBILEGL_RHI_H

#include "../../Includes.h"

namespace MG_RHI::GLES {
    enum class State {
        None,
        DrawArrays,
        DrawElements
    };

    class RHI {
    public:
        void TransitionTo(State toState);

        static inline RHI& GetInstance() {
            static RHI s_rhi;
            return s_rhi;
        }

        static void CheckError();

        void SyncAllTexturesToGLES(TextureState* textureState);
        void SyncAllBuffersToGLES(BufferState* bufferState);
    private:

        State currentState_ = State::None;
        std::unordered_map<GLuint, GLuint> textureMap_;
        std::unordered_map<GLuint, std::unordered_map<GLint, bool>> textureLevelUploaded_;
        std::unordered_map<GLuint, GLuint> bufferMap_;
        std::array<GLuint, 32> lastBoundTextures { 0 };
        std::unordered_map<GLuint, GLuint> vaoMap_;

        std::unordered_map<GLuint, GLuint> programMap_;
        std::unordered_map<GLuint, GLuint> framebufferMap_;

        GLuint lastBoundVAO = 0;
        GLuint lastBoundProgram = 0;
        GLuint lastBoundFBO = 0;
    };
}

#define MGL_TRY(operation) MG_Util::Debug::LogD("GLES call: %s", #operation); operation MG_RHI::GLES::RHI::CheckError();

#endif //MOBILEGL_RHI_H
