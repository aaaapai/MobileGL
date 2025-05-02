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
}