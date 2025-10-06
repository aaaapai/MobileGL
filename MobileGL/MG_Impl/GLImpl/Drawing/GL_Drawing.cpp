#include "GL_Drawing.h"
#include <Config.h>
#include <MG_State/GLState/Core.h>
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        void Clear_Backend(GLbitfield mask) {

#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            auto clearColor = MG_State::pGLContext->GetClearColor();
            MG_External::GLES::glClearColor(clearColor.r(), clearColor.g(), clearColor.b(), clearColor.a());
            auto clearDepth = MG_State::pGLContext->GetClearDepth();
            MG_External::GLES::glClearDepthf(clearDepth);
            MG_External::GLES::glClear(mask);
#endif
        }

        void DrawElements_Backend(GLenum mode, GLsizei count, GLenum type, const void* indices) {
            // TODO
        }

        /* @INSERTION_POINT:FUNCTION_IMPLEMENTATION@ */
        void Clear(GLbitfield mask) {
            Clear_Backend(mask);
        }

        void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
            DrawElements_Backend(mode, count, type, indices);
        }

    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
