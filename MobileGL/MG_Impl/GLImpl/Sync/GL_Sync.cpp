#include "GL_Sync.h"

#include "MG_State/GLState/Core.h"

#include <MG_Util/BackendLoaders/OpenGL/Loader.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        GLsync FenceSync_Backend(GLenum condition, GLbitfield flags) {
            return 0;
        }

        GLenum ClientWaitSync_Backend(GLsync sync, GLbitfield flags, GLuint64 timeout) {
            return 0;
        }

        void DeleteSync_Backend(GLsync sync) {}

        GLsync FenceSync_State(GLenum condition, GLbitfield flags) {
            return 0;
        }

        GLenum ClientWaitSync_State(GLsync sync, GLbitfield flags, GLuint64 timeout) {
            return 0;
        }

        void DeleteSync_State(GLsync sync) {}

        GLsync FenceSync(GLenum condition, GLbitfield flags) {
            return MG_External::GLES::glFenceSync(condition, flags);
        }

        GLenum ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
            return MG_External::GLES::glClientWaitSync(sync, flags, timeout);
        }

        void DeleteSync(GLsync sync) {
            MG_External::GLES::glDeleteSync(sync);
        }

    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
