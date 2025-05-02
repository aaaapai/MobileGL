//
// Created by BZLZHH on 2025/3/15.
//

#ifndef MOBILEGL_GL_PROGRAM_H
#define MOBILEGL_GL_PROGRAM_H

#include "../../../../Includes.h"

namespace MG_GL::GL {
    GLuint CreateShader(GLenum type);
    GLuint CreateProgram();
    void DeleteShader(GLuint shader);
    void DeleteProgram(GLuint program);
    void AttachShader(GLuint program, GLuint shader);
    void ShaderSource(GLuint shader, GLsizei count, const GLchar *const*string, const GLint* length);
    void CompileShader(GLuint shader);
    void LinkProgram(GLuint program);
    void UseProgram(GLuint program);
    void BindAttribLocation(GLuint program, GLuint index, const GLchar* name);
    GLint GetAttribLocation(GLuint program, const GLchar* name);
    GLint GetUniformLocation(GLuint program, const GLchar* name);
    void GetProgramiv(GLuint program, GLenum pname, GLint* params);
    void GetShaderiv(GLuint shader, GLenum pname, GLint* params);
}

#endif //MOBILEGL_GL_PROGRAM_H
