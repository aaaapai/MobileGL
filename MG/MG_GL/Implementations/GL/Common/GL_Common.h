//
// Created by BZLZHH on 2025/3/15.
//

#ifndef MOBILEGL_GL_COMMON_H
#define MOBILEGL_GL_COMMON_H

#include "../../../../Includes.h"

namespace MG_GL::GL {
    void ClearDepth(::GLdouble depth);
    void ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void Clear(GLbitfield mask);
    void PixelStorei(GLenum pname, GLint param);
}

#endif //MOBILEGL_GL_COMMON_H
