//
// Created by BZLZHH on 2025/3/15.
//

#ifndef MOBILEGL_GL_PROGRAM_H
#define MOBILEGL_GL_PROGRAM_H

#include "../../../../Includes.h"

namespace MG_GL::GL {
    Diligent::VALUE_TYPE ConvertGLTypeToDiligent(GLenum type);
    
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
    void Uniform1f(GLint location, GLfloat v0);
    void Uniform2f(GLint location, GLfloat v0, GLfloat v1);
    void Uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    void Uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void Uniform1fv(GLint location, GLsizei count, const GLfloat* value);
    void Uniform2fv(GLint location, GLsizei count, const GLfloat* value);
    void Uniform3fv(GLint location, GLsizei count, const GLfloat* value);
    void Uniform4fv(GLint location, GLsizei count, const GLfloat* value);
    void Uniform1i(GLint location, GLint v0);
    void Uniform2i(GLint location, GLint v0, GLint v1);
    void Uniform3i(GLint location, GLint v0, GLint v1, GLint v2);
    void Uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    void Uniform1iv(GLint location, GLsizei count, const GLint* value);
    void Uniform2iv(GLint location, GLsizei count, const GLint* value);
    void Uniform3iv(GLint location, GLsizei count, const GLint* value);
    void Uniform4iv(GLint location, GLsizei count, const GLint* value);
    void Uniform1ui(GLint location, GLuint v0);
    void Uniform2ui(GLint location, GLuint v0, GLuint v1);
    void Uniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2);
    void Uniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
    void Uniform1uiv(GLint location, GLsizei count, const GLuint* value);
    void Uniform2uiv(GLint location, GLsizei count, const GLuint* value);
    void Uniform3uiv(GLint location, GLsizei count, const GLuint* value);
    void Uniform4uiv(GLint location, GLsizei count, const GLuint* value);
    void UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void UniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void UniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void UniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void UniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void UniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void UniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
}

#endif //MOBILEGL_GL_PROGRAM_H
