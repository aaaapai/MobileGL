#pragma once
#include <Includes.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        GLsync FenceSync(GLenum condition, GLbitfield flags);
        GLenum ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
        void DeleteSync(GLsync sync);
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
