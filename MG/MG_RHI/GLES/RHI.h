//
// Created by Swung 0x48 on 2025/5/6.
//

#ifndef MOBILEGL_RHI_H
#define MOBILEGL_RHI_H

#include "../../Includes.h"

namespace MG_RHI::GLES {
    template <typename K, typename V>
    using unordered_map = ankerl::unordered_dense::map<K, V>;

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
        unordered_map<GLuint, GLuint> textureMap_;
        unordered_map<GLuint, GLuint> bufferMap_;
        unordered_map<GLuint, GLuint> vaoMap_;
        unordered_map<GLuint, GLuint> programMap_;
        unordered_map<GLuint, GLuint> framebufferMap_;

        unordered_map<GLuint, unordered_map<GLint, bool>> textureLevelUploaded_;
        std::array<GLuint, 32> lastBoundTextures { 0 };

        GLuint lastBoundVAO = 0;
        GLuint lastBoundProgram = 0;
        GLuint lastBoundFBO = 0;
    };
}

#define MGL_TRY(operation) MG_Util::Debug::LogD("GLES call: %s", #operation); operation MG_RHI::GLES::RHI::CheckError();

#endif //MOBILEGL_RHI_H
