#pragma once
#include <Includes.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        /* @INSERTION_POINT:FUNCTION_DECLARATION@ */
        void Clear(GLbitfield mask);
        void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
