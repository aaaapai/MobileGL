//
// Created by BZLZHH on 2025/3/15.
//

#ifndef MOBILEGL_GL_COMMON_H
#define MOBILEGL_GL_COMMON_H

namespace MG_GL::GL {
    void ClearDepth(GLdouble depth);
    void ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void Clear(GLbitfield mask);
    void PixelStorei(GLenum pname, GLint param);
    void Enable(GLenum cap);
    void Disable(GLenum cap);
    void BlendFunc(GLenum sfactor, GLenum dfactor);
    void BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
    void ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    void DepthFunc(GLenum func);
    void DepthMask(GLboolean flag);
    void Viewport(GLint x, GLint y, GLsizei width, GLsizei height);
}

#endif //MOBILEGL_GL_COMMON_H
