#pragma once
#include <Includes.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        void AttachShader(GLuint program, GLuint shader);
        void BindAttribLocation(GLuint program, GLuint index, const GLchar* name);
        void CompileShader(GLuint shader);
        GLuint CreateProgram(void);
        GLuint CreateShader(GLenum type);
        void DeleteProgram(GLuint program);
        void DeleteShader(GLuint shader);
        void DetachShader(GLuint program, GLuint shader);
        void GetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type,
                             GLchar* name);
        void GetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type,
                              GLchar* name);
        void GetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders);
        GLint GetAttribLocation(GLuint program, const GLchar* name);
        void GetProgramiv(GLuint program, GLenum pname, GLint* params);
        void GetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
        void GetShaderiv(GLuint shader, GLenum pname, GLint* params);
        void GetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
        void GetShaderSource(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source);
        GLint GetUniformLocation(GLuint program, const GLchar* name);
        void GetUniformfv(GLuint program, GLint location, GLfloat* params);
        void GetUniformiv(GLuint program, GLint location, GLint* params);
        GLboolean IsProgram(GLuint program);
        GLboolean IsShader(GLuint shader);
        void LinkProgram(GLuint program);
        void ShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
        void UseProgram(GLuint program);
        void Uniform1f(GLint location, GLfloat v0);
        void Uniform2f(GLint location, GLfloat v0, GLfloat v1);
        void Uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
        void Uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
        void Uniform1i(GLint location, GLint v0);
        void Uniform2i(GLint location, GLint v0, GLint v1);
        void Uniform3i(GLint location, GLint v0, GLint v1, GLint v2);
        void Uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
        void Uniform1fv(GLint location, GLsizei count, const GLfloat* value);
        void Uniform2fv(GLint location, GLsizei count, const GLfloat* value);
        void Uniform3fv(GLint location, GLsizei count, const GLfloat* value);
        void Uniform4fv(GLint location, GLsizei count, const GLfloat* value);
        void Uniform1iv(GLint location, GLsizei count, const GLint* value);
        void Uniform2iv(GLint location, GLsizei count, const GLint* value);
        void Uniform3iv(GLint location, GLsizei count, const GLint* value);
        void Uniform4iv(GLint location, GLsizei count, const GLint* value);
        void UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
        void UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
        void UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
        void ValidateProgram(GLuint program);
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
