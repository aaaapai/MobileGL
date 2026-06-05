// MobileGL - MobileGL/MG_Impl/GLImpl/Program/GL_Program.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MobileGL::MG_Impl::GLImpl {
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
    GLuint GetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName);
    void UniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
    void GetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
    void GetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length,
                                   GLchar* uniformBlockName);
    void BindFragDataLocation(GLuint program, GLuint colorNumber, const char* name);
    GLint GetFragDataLocation(GLuint program, const char* name);
    void GetProgramInterfaceiv(GLuint program, GLenum programInterface, GLenum pname, GLint* params);
    GLuint GetProgramResourceIndex(GLuint program, GLenum programInterface, const GLchar* name);
    void GetProgramResourceName(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize,
                                GLsizei* length, GLchar* name);
    void GetProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount,
                              const GLenum* props, GLsizei bufSize, GLsizei* length, GLint* params);
    GLint GetProgramResourceLocation(GLuint program, GLenum programInterface, const GLchar* name);
    GLint GetProgramResourceLocationIndex(GLuint program, GLenum programInterface, const GLchar* name);
    void ShaderStorageBlockBinding(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);
    void ValidateProgram(GLuint program);
} // namespace MobileGL::MG_Impl::GLImpl
