#include "GL_Drawing.h"
#include <Config.h>
#include <MG_State/GLState/Core.h>
#include <MG_Backend/DirectGLES/DirectGLES.h>
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        void Clear_Backend(GLbitfield mask) {
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::Clear(mask);
#endif
        }

        void DrawElements_Backend(GLenum mode, GLsizei count, GLenum type, const void* indices) {
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::DrawElements(mode, count, type, indices);
#endif
        }

        void MultiDrawElements_Backend(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices,
                                       GLsizei drawcount) {
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::MultiDrawElements(mode, count, type, indices, drawcount);
#endif
        }

        void MultiDrawElementsBaseVertex_Backend(GLenum mode, const GLsizei* count, GLenum type,
                                                 const void* const* indices, GLsizei drawcount,
                                                 const GLint* basevertex) {
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::MultiDrawElementsBaseVertex(mode, count, type, indices, drawcount, basevertex);
#endif
        }

        /* @INSERTION_POINT:FUNCTION_IMPLEMENTATION@ */
        void MultiDrawElements(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices,
                               GLsizei drawcount) {
            MultiDrawElements_Backend(mode, count, type, indices, drawcount);
        }

        void MultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices,
                                         GLsizei drawcount, const GLint* basevertex) {
            MultiDrawElementsBaseVertex_Backend(mode, count, type, indices, drawcount, basevertex);
        }

        void Clear(GLbitfield mask) {
            Clear_Backend(mask);
        }

        void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
            DrawElements_Backend(mode, count, type, indices);
        }

    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
