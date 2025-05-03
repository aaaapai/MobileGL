//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_Program.h"

namespace MG_GL::GL {
    GLuint CreateShader(GLenum type) {
        MG_Util::Debug::LogD("glCreateShader, type: %s", MG_Util::Debug::GLEnumToString(type));
        GLuint shader;
        GLenum result = MG_State::CreateShader(type, &shader);
        if (result == GL_NO_ERROR) {
            MG_Util::Debug::LogD("Created shader ID: %u", shader);
            return shader;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error creating shader: %s", MG_Util::Debug::GLEnumToString(result));
        return 0;
    }

    GLuint CreateProgram() {
        MG_Util::Debug::LogD("glCreateProgram");
        GLuint program;
        GLenum result = MG_State::CreateProgram(&program);
        if (result == GL_NO_ERROR) {
            MG_Util::Debug::LogD("Created program ID: %u", program);
            return program;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error creating program: %s", MG_Util::Debug::GLEnumToString(result));
        return 0;
    }
    
    void DeleteShader(GLuint shader) {
        MG_Util::Debug::LogD("glDeleteShader, shader: %u", shader);
        GLenum result = MG_State::DeleteShader(shader);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error deleting shader: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    void DeleteProgram(GLuint program) {
        MG_Util::Debug::LogD("glDeleteProgram, program: %u", program);
        GLenum result = MG_State::DeleteProgram(program);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error deleting program: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    void AttachShader(GLuint program, GLuint shader) {
        MG_Util::Debug::LogD("glAttachShader, program: %u, shader: %u", program, shader);
        GLenum result = MG_State::LinkShaderToProgram(program, shader);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error attaching shader: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    void ShaderSource(GLuint shader, GLsizei count, const GLchar *const*string, const GLint* length) {
        MG_Util::Debug::LogD("glShaderSource, shader: %u, count: %d", shader, count);
        GLenum result = MG_State::UploadShaderSource(shader, count, (const GLchar **)string, length);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting shader source: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    void CompileShader(GLuint shader) {
        MG_Util::Debug::LogD("glCompileShader, shader: %u", shader);
        GLenum result = MG_State::BuildShaderStage(shader);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error compiling shader: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    void LinkProgram(GLuint program) {
        MG_Util::Debug::LogD("glLinkProgram, program: %u", program);
        GLenum result = MG_State::FinalizeProgramPipeline(program);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error linking program: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    void UseProgram(GLuint program) {
        MG_Util::Debug::LogD("glUseProgram, program: %u", program);
        GLenum result = MG_State::ActivateRenderProgram(program);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error using program: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    void BindAttribLocation(GLuint program, GLuint index, const GLchar* name) {
        MG_Util::Debug::LogD("glBindAttribLocation, program: %u, index: %u, name: %s", program, index, name);
        GLenum result = MG_State::DefineProgramAttributeBinding(program, index, name);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error binding attribute: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    GLint GetAttribLocation(GLuint program, const GLchar* name) {
        MG_Util::Debug::LogD("glGetAttribLocation, program: %u, name: %s", program, name);
        GLint location = MG_State::QueryProgramAttributeBinding(program, name);
        if (location != -1) return location;
        MG_State::SetError(GL_INVALID_OPERATION);
        MG_Util::Debug::LogE("Attribute not found: %s", name);
        return -1;
    }
    
    GLint GetUniformLocation(GLuint program, const GLchar* name) {
        MG_Util::Debug::LogD("glGetUniformLocation, program: %u, name: %s", program, name);
        GLint location = MG_State::QueryProgramUniformLocation(program, name);
        if (location != -1) return location;
        MG_State::SetError(GL_INVALID_OPERATION);
        MG_Util::Debug::LogE("Uniform not found: %s", name);
        return -1;
    }
    
    void GetProgramiv(GLuint program, GLenum pname, GLint* params) {
        MG_Util::Debug::LogD("glGetProgramiv, program: %u, pname: %s",
                             program, MG_Util::Debug::GLEnumToString(pname));
        GLenum result = MG_State::QueryProgramStateIntVector(program, pname, params);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error getting program param: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    void GetShaderiv(GLuint shader, GLenum pname, GLint* params) {
        MG_Util::Debug::LogD("glGetShaderiv, shader: %u, pname: %s",
                             shader, MG_Util::Debug::GLEnumToString(pname));
        GLenum result = MG_State::QueryShaderStateIntVector(shader, pname, params);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error getting shader param: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void Uniform1f(GLint location, GLfloat v0) {
        MG_Util::Debug::LogD("glUniform1f, location: %d, v0: %f", location, v0);
        MG_State::UpdateProgramUniformFloat1(location, v0);
    }

    void Uniform2f(GLint location, GLfloat v0, GLfloat v1) {
        MG_Util::Debug::LogD("glUniform2f, location: %d, v0: %f, v1: %f", location, v0, v1);
        MG_State::UpdateProgramUniformFloat2(location, v0, v1);
    }

    void Uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
        MG_Util::Debug::LogD("glUniform3f, location: %d, v0: %f, v1: %f, v2: %f", location, v0, v1, v2);
        MG_State::UpdateProgramUniformFloat3(location, v0, v1, v2);
    }

    void Uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
        MG_Util::Debug::LogD("glUniform4f, location: %d, v0: %f, v1: %f, v2: %f, v3: %f", location, v0, v1, v2, v3);
        MG_State::UpdateProgramUniformFloat4(location, v0, v1, v2, v3);
    }

    void Uniform1fv(GLint location, GLsizei count, const GLfloat* value) {
        MG_Util::Debug::LogD("glUniform1fv, location: %d, count: %d", location, count);
        MG_State::UpdateProgramUniformFloatVector1(location, count, value);
    }

    void Uniform2fv(GLint location, GLsizei count, const GLfloat* value) {
        MG_Util::Debug::LogD("glUniform2fv, location: %d, count: %d", location, count);
        MG_State::UpdateProgramUniformFloatVector2(location, count, value);
    }

    void Uniform3fv(GLint location, GLsizei count, const GLfloat* value) {
        MG_Util::Debug::LogD("glUniform3fv, location: %d, count: %d", location, count);
        MG_State::UpdateProgramUniformFloatVector3(location, count, value);
    }

    void Uniform4fv(GLint location, GLsizei count, const GLfloat* value) {
        MG_Util::Debug::LogD("glUniform4fv, location: %d, count: %d", location, count);
        MG_State::UpdateProgramUniformFloatVector4(location, count, value);
    }

    void Uniform1i(GLint location, GLint v0) {
        MG_Util::Debug::LogD("glUniform1i, location: %d, v0: %d", location, v0);
        MG_State::UpdateProgramUniformInt1(location, v0);
    }

    void Uniform2i(GLint location, GLint v0, GLint v1) {
        MG_Util::Debug::LogD("glUniform2i, location: %d, v0: %d, v1: %d", location, v0, v1);
        MG_State::UpdateProgramUniformInt2(location, v0, v1);
    }

    void Uniform3i(GLint location, GLint v0, GLint v1, GLint v2) {
        MG_Util::Debug::LogD("glUniform3i, location: %d, v0: %d, v1: %d, v2: %d", location, v0, v1, v2);
        MG_State::UpdateProgramUniformInt3(location, v0, v1, v2);
    }

    void Uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
        MG_Util::Debug::LogD("glUniform4i, location: %d, v0: %d, v1: %d, v2: %d, v3: %d", location, v0, v1, v2, v3);
        MG_State::UpdateProgramUniformInt4(location, v0, v1, v2, v3);
    }

    void Uniform1iv(GLint location, GLsizei count, const GLint* value) {
        MG_Util::Debug::LogD("glUniform1iv, location: %d, count: %d", location, count);
        MG_State::UpdateProgramUniformIntVector1(location, count, value);
    }

    void Uniform2iv(GLint location, GLsizei count, const GLint* value) {
        MG_Util::Debug::LogD("glUniform2iv, location: %d, count: %d", location, count);
        MG_State::UpdateProgramUniformIntVector2(location, count, value);
    }

    void Uniform3iv(GLint location, GLsizei count, const GLint* value) {
        MG_Util::Debug::LogD("glUniform3iv, location: %d, count: %d", location, count);
        MG_State::UpdateProgramUniformIntVector3(location, count, value);
    }

    void Uniform4iv(GLint location, GLsizei count, const GLint* value) {
        MG_Util::Debug::LogD("glUniform4iv, location: %d, count: %d", location, count);
        MG_State::UpdateProgramUniformIntVector4(location, count, value);
    }

    void Uniform1ui(GLint location, GLuint v0) {
        MG_Util::Debug::LogD("glUniform1ui, location: %d, v0: %u", location, v0);
        MG_State::UpdateProgramUniformUInt1(location, v0);
    }

    void Uniform2ui(GLint location, GLuint v0, GLuint v1) {
        MG_Util::Debug::LogD("glUniform2ui, location: %d, v0: %u, v1: %u", location, v0, v1);
        MG_State::UpdateProgramUniformUInt2(location, v0, v1);
    }

    void Uniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2) {
        MG_Util::Debug::LogD("glUniform3ui, location: %d, v0: %u, v1: %u, v2: %u", location, v0, v1, v2);
        MG_State::UpdateProgramUniformUInt3(location, v0, v1, v2);
    }

    void Uniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {
        MG_Util::Debug::LogD("glUniform4ui, location: %d, v0: %u, v1: %u, v2: %u, v3: %u", location, v0, v1, v2, v3);
        MG_State::UpdateProgramUniformUInt4(location, v0, v1, v2, v3);
    }

    void Uniform1uiv(GLint location, GLsizei count, const GLuint* value) {
        MG_Util::Debug::LogD("glUniform1uiv, location: %d, count: %d", location, count);
        MG_State::UpdateProgramUniformUIntVector1(location, count, value);
    }

    void Uniform2uiv(GLint location, GLsizei count, const GLuint* value) {
        MG_Util::Debug::LogD("glUniform2uiv, location: %d, count: %d", location, count);
        MG_State::UpdateProgramUniformUIntVector2(location, count, value);
    }

    void Uniform3uiv(GLint location, GLsizei count, const GLuint* value) {
        MG_Util::Debug::LogD("glUniform3uiv, location: %d, count: %d", location, count);
        MG_State::UpdateProgramUniformUIntVector3(location, count, value);
    }

    void Uniform4uiv(GLint location, GLsizei count, const GLuint* value) {
        MG_Util::Debug::LogD("glUniform4uiv, location: %d, count: %d", location, count);
        MG_State::UpdateProgramUniformUIntVector4(location, count, value);
    }

    void UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
        MG_Util::Debug::LogD("glUniformMatrix2fv, location: %d, count: %d, transpose: %d", location, count, transpose);
        MG_State::UpdateProgramUniformMatrix2x2Vector(location, count, transpose, value);
    }

    void UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
        MG_Util::Debug::LogD("glUniformMatrix3fv, location: %d, count: %d, transpose: %d", location, count, transpose);
        MG_State::UpdateProgramUniformMatrix3x3Vector(location, count, transpose, value);
    }

    void UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
        MG_Util::Debug::LogD("glUniformMatrix4fv, location: %d, count: %d, transpose: %d", location, count, transpose);
        MG_State::UpdateProgramUniformMatrix4x4Vector(location, count, transpose, value);
    }

    void UniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
        MG_Util::Debug::LogD("glUniformMatrix2x3fv, location: %d, count: %d, transpose: %d", location, count, transpose);
        MG_State::UpdateProgramUniformMatrix2x3Vector(location, count, transpose, value);
    }

    void UniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
        MG_Util::Debug::LogD("glUniformMatrix2x4fv, location: %d, count: %d, transpose: %d", location, count, transpose);
        MG_State::UpdateProgramUniformMatrix2x4Vector(location, count, transpose, value);
    }

    void UniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
        MG_Util::Debug::LogD("glUniformMatrix3x2fv, location: %d, count: %d, transpose: %d", location, count, transpose);
        MG_State::UpdateProgramUniformMatrix3x2Vector(location, count, transpose, value);
    }

    void UniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
        MG_Util::Debug::LogD("glUniformMatrix3x4fv, location: %d, count: %d, transpose: %d", location, count, transpose);
        MG_State::UpdateProgramUniformMatrix3x4Vector(location, count, transpose, value);
    }

    void UniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
        MG_Util::Debug::LogD("glUniformMatrix4x2fv, location: %d, count: %d, transpose: %d", location, count, transpose);
        MG_State::UpdateProgramUniformMatrix4x2Vector(location, count, transpose, value);
    }

    void UniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
        MG_Util::Debug::LogD("glUniformMatrix4x3fv, location: %d, count: %d, transpose: %d", location, count, transpose);
        MG_State::UpdateProgramUniformMatrix4x3Vector(location, count, transpose, value);
    }
}