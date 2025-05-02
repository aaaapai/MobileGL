//
// Created by BZLZHH on 2025/5/1.
//

// TODO: Add more gl error check for buffer state manager.
// TODO: Check if the state machine really meets up to OpenGL 3 standard.

#include "ProgramState.h"

GLenum ProgramState::CreateShader(GLenum type, GLuint* shader) {
    if (!shader) return GL_INVALID_VALUE;
    if (type != GL_VERTEX_SHADER && type != GL_FRAGMENT_SHADER)
        return GL_INVALID_ENUM;

    GLuint id = freeShaderIds_.empty() ? ++lastShaderId_ : *freeShaderIds_.begin();
    if (!freeShaderIds_.empty()) freeShaderIds_.erase(freeShaderIds_.begin());

    ShaderObject obj;
    obj.type = type;
    obj.compiled = UncertainBool::False;
    obj.markedForDeletion = false;
    shaders_[id] = obj;
    *shader = id;
    return GL_NO_ERROR;
}

GLenum ProgramState::CreateProgram(GLuint* program) {
    if (!program) return GL_INVALID_VALUE;

    GLuint id = freeProgramIds_.empty() ? ++lastProgramId_ : *freeProgramIds_.begin();
    if (!freeProgramIds_.empty()) freeProgramIds_.erase(freeProgramIds_.begin());

    ProgramObject obj;
    obj.linked = UncertainBool::False;
    obj.markedForDeletion = false;
    programs_[id] = obj;
    *program = id;
    return GL_NO_ERROR;
}

GLenum ProgramState::DeleteShader(GLuint shader) {
    if (!ValidateShader_(shader)) return GL_INVALID_VALUE;

    if (shaders_[shader].markedForDeletion) return GL_NO_ERROR;

    bool inUse = false;
    for (auto& [progId, program] : programs_) {
        auto it = std::find(program.attachedShaders.begin(),
                            program.attachedShaders.end(), shader);
        if (it != program.attachedShaders.end()) {
            inUse = true;
            break;
        }
    }

    if (inUse) {
        shaders_[shader].markedForDeletion = true;
    } else {
        shaders_.erase(shader);
        freeShaderIds_.insert(shader);
    }
    return GL_NO_ERROR;
}

GLenum ProgramState::DeleteProgram(GLuint program) {
    if (!ValidateProgram_(program)) return GL_INVALID_VALUE;

    programs_.erase(program);
    freeProgramIds_.insert(program);
    if (currentProgram_ == program)
        currentProgram_ = 0;
    return GL_NO_ERROR;
}

GLenum ProgramState::LinkShaderToProgram(GLuint program, GLuint shader) {
    if (!ValidateProgram_(program) || !ValidateShader_(shader))
        return GL_INVALID_VALUE;

    auto& prog = programs_[program];
    if (std::find(prog.attachedShaders.begin(),
                  prog.attachedShaders.end(), shader) != prog.attachedShaders.end())
        return GL_INVALID_OPERATION;

    prog.attachedShaders.push_back(shader);
    return GL_NO_ERROR;
}

GLenum ProgramState::UploadShaderSource(GLuint shader, GLsizei count,
                                        const GLchar** string, const GLint* length) {
    if (!ValidateShader_(shader)) return GL_INVALID_VALUE;
    if (count < 0) return GL_INVALID_VALUE;

    std::string source;
    for (GLsizei i = 0; i < count; ++i) {
        const GLchar* str = string[i];
        GLint len = (length && length[i] >= 0) ? length[i] : strlen(str);
        source.append(str, len);
    }
    shaders_[shader].source = std::move(source);
    return GL_NO_ERROR;
}

GLenum ProgramState::BuildShaderStage(GLuint shader) {
    if (!ValidateShader_(shader)) return GL_INVALID_VALUE;

    auto& obj = shaders_[shader];
    // TODO: Add glslang to convert it to SPIR-V first, and then check it in the actual backend.
    obj.compiled = {UncertainBool::Unknown, UncertainBool::True};
    obj.compileStatus = GL_TRUE;
    obj.infoLog.clear();

    return GL_NO_ERROR;
}

GLenum ProgramState::FinalizeProgramPipeline(GLuint program) {
    if (!ValidateProgram_(program)) return GL_INVALID_VALUE;
    auto& prog = programs_[program];

    for (GLuint shader : prog.attachedShaders) {
        if (!shaders_[shader].compiled.toBool())
            return GL_INVALID_OPERATION;
    }

    // TODO: Check the actual program linking status.
    // We might consider using a program emulator (or real backend checking?) or something better in the future.
    prog.linked = {UncertainBool::Unknown, UncertainBool::True};
    prog.linkStatus = GL_TRUE;
    prog.infoLog.clear();

    for (auto& [name, index] : prog.attribBindings) {
        prog.attribLocations[name] = index;
    }
    
    CleanupShaders_();
    return GL_NO_ERROR;
}

GLenum ProgramState::ActivateRenderProgram(GLuint program) {
    if (program != 0 && !ValidateProgram_(program))
        return GL_INVALID_VALUE;

    currentProgram_ = program;
    return GL_NO_ERROR;
}

GLenum ProgramState::DefineProgramAttributeBinding(GLuint program, GLuint index, const GLchar* name) {
    if (!ValidateProgram_(program)) return GL_INVALID_VALUE;
    if (index >= GL_MAX_VERTEX_ATTRIBS) return GL_INVALID_VALUE;

    programs_[program].attribBindings[name] = index;
    return GL_NO_ERROR;
}

GLint ProgramState::QueryProgramAttributeBinding(GLuint program, const GLchar* name) {
    if (!ValidateProgram_(program)) return -1;

    auto& prog = programs_[program];
    if (prog.attribLocations.count(name))
        return prog.attribLocations[name];
    return -1;
}

GLint ProgramState::QueryProgramUniformLocation(GLuint program, const GLchar* name) {
    if (!ValidateProgram_(program)) return -1;

    auto& prog = programs_[program];
    if (prog.uniformLocations.count(name))
        return prog.uniformLocations[name];
    return -1;
}

GLenum ProgramState::QueryProgramStateIntVector(GLuint program, GLenum pname, GLint* params) {
    if (!ValidateProgram_(program)) return GL_INVALID_VALUE;
    auto& prog = programs_[program];

    switch (pname) {
        case GL_LINK_STATUS: *params = prog.linkStatus; break;
        case GL_DELETE_STATUS: *params = prog.markedForDeletion; break;
        case GL_ATTACHED_SHADERS: *params = (GLint)prog.attachedShaders.size(); break;
        default: return GL_INVALID_ENUM;
    }
    return GL_NO_ERROR;
}

GLenum ProgramState::QueryShaderStateIntVector(GLuint shader, GLenum pname, GLint* params) {
    if (!ValidateShader_(shader)) return GL_INVALID_VALUE;
    auto& obj = shaders_[shader];

    switch (pname) {
        case GL_COMPILE_STATUS: *params = obj.compileStatus; break;
        case GL_DELETE_STATUS: *params = obj.markedForDeletion; break;
        case GL_SHADER_TYPE: *params = obj.type; break;
        default: return GL_INVALID_ENUM;
    }
    return GL_NO_ERROR;
}

bool ProgramState::SetProgramStatus(GLuint program, GLboolean status) {
    if (!ValidateProgram_(program)) return false;
    auto& prog = programs_[program];
    if (prog.linked && status != GL_TRUE) {
        return false;
    }
    if (status == GL_TRUE)
        prog.linked = true;
    else
        prog.linked = false;
    prog.linkStatus = status;
    return true;
}

bool ProgramState::SetShaderStatus(GLuint shader, GLboolean status) {
    if (!ValidateShader_(shader)) return false;
    auto& obj = shaders_[shader];
    if (status == GL_TRUE)
        obj.compiled = true;
    else
        obj.compiled = false;
    obj.compileStatus = status;
    return true;
}

bool ProgramState::ValidateShader_(GLuint shader) {
    return shaders_.count(shader) && !shaders_[shader].markedForDeletion;
}

bool ProgramState::ValidateProgram_(GLuint program) {
    return programs_.count(program) && !programs_[program].markedForDeletion;
}

void ProgramState::CleanupShaders_() {
    auto it = shaders_.begin();
    while (it != shaders_.end()) {
        if (it->second.markedForDeletion) {
            bool inUse = false;
            for (auto& [progId, program] : programs_) {
                if (std::find(program.attachedShaders.begin(),
                              program.attachedShaders.end(), it->first)
                    != program.attachedShaders.end()) {
                    inUse = true;
                    break;
                }
            }
            if (!inUse) {
                freeShaderIds_.insert(it->first);
                it = shaders_.erase(it);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
}

GLuint ProgramState::GetCurrentProgram() const {
    return currentProgram_; 
}