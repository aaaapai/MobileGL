//
// Created by BZLZHH on 2025/5/1.
//

#ifndef MOBILEGL_PROGRAMSTATE_H
#define MOBILEGL_PROGRAMSTATE_H
#define MOBILEGL_GLSTATE_H

#include "../../../Includes.h"

struct ShaderObject {
    GLenum type;
    std::string source;
    UncertainBool compiled;
    GLint compileStatus;
    std::string infoLog;
    bool markedForDeletion;
};

struct ProgramObject {
    std::vector<GLuint> attachedShaders;
    UncertainBool linked;
    GLint linkStatus;
    std::string infoLog;
    std::unordered_map<std::string, GLint> attribBindings;
    std::unordered_map<std::string, GLint> attribLocations;
    std::unordered_map<std::string, GLint> uniformLocations;
    bool markedForDeletion;
};

class ProgramState {
private:
    std::unordered_map<GLuint, ShaderObject> shaders_;
    std::unordered_map<GLuint, ProgramObject> programs_;
    std::set<GLuint> freeShaderIds_;
    std::set<GLuint> freeProgramIds_;
    GLuint lastShaderId_ = 0;
    GLuint lastProgramId_ = 0;
    GLuint currentProgram_ = 0;

public:
    // Return: the validity of the operation, according to OpenGL 3 standard
    GLenum CreateShader(GLenum type, GLuint* shader);
    GLenum CreateProgram(GLuint* program);
    GLenum DeleteShader(GLuint shader);
    GLenum DeleteProgram(GLuint program);
    GLenum LinkShaderToProgram(GLuint program, GLuint shader);
    GLenum UploadShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
    GLenum BuildShaderStage(GLuint shader);
    GLenum FinalizeProgramPipeline(GLuint program);
    GLenum ActivateRenderProgram(GLuint program);
    GLenum DefineProgramAttributeBinding(GLuint program, GLuint index, const GLchar* name);
    GLint QueryProgramAttributeBinding(GLuint program, const GLchar* name);
    GLint QueryProgramUniformLocation(GLuint program, const GLchar* name);
    GLenum QueryProgramStateIntVector(GLuint program, GLenum pname, GLint* params);
    GLenum QueryShaderStateIntVector(GLuint shader, GLenum pname, GLint* params);
    
    GLuint GetCurrentProgram() const;
    bool SetProgramStatus(GLuint program, GLboolean status);
    bool SetShaderStatus(GLuint shader, GLboolean status);

private:
    bool ValidateShader_(GLuint shader);
    bool ValidateProgram_(GLuint program);
    void CleanupShaders_();
};

#endif // MOBILEGL_PROGRAMSTATE_H
