//
// Created by BZLZHH on 2025/3/15.
//

#ifndef MOBILEGL_GL_TEXTURE_H
#define MOBILEGL_GL_TEXTURE_H

#include "../../../../Includes.h"

namespace MG_GL::GL {
    void ActiveTexture(GLenum texture);
    void BindTexture(GLenum target, GLuint texture);
    void DeleteTextures(GLsizei n, const GLuint* textures);
    void GenTextures(GLsizei n, GLuint* textures);
    void TexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* data);
    void TexParameterf(GLenum target, GLenum pname, GLfloat param);
    void TexParameteri(GLenum target, GLenum pname, GLint param);
    void TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
    void GetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params);
}

#endif //MOBILEGL_GL_TEXTURE_H
