#pragma once
#include <Includes.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        /* @INSERTION_POINT:FUNCTION_DECLARATION@ */
        void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex);
        void DrawArrays(GLenum mode, GLint first, GLsizei count);
        void MultiDrawElements(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices,
                               GLsizei drawcount);
        void MultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices,
                                         GLsizei drawcount, const GLint* basevertex);
        void Clear(GLbitfield mask);
        void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
