//
// Created by BZLZHH on 2025/5/3.
//

#include "ShaderObject.h"

// Re-Include Includes.h, cuz of the absence of Program/DebugTool.h and Program/GLSLTool.cpp.
#undef MOBILEGL_GLSLTOOL_H
#undef MOBILEGL_PROGRAM_DEBUGTOOL_H
#include "../../../Includes.h"

GLenum ProgramState::CreateShader(GLenum type, GLuint* shader) {
    MG_Util::Debug::LogD("MG_State: Program: CreateShader called, type=0x%X, shader ptr=%p", type, shader);
    if (!shader) {
        MG_Util::Debug::LogE("MG_State: Program: CreateShader error: shader pointer is null");
        return GL_INVALID_VALUE;
    }
    if (type != GL_VERTEX_SHADER && type != GL_FRAGMENT_SHADER) {
        MG_Util::Debug::LogE("MG_State: Program: CreateShader error: invalid shader type 0x%X", type);
        return GL_INVALID_ENUM;
    }

    GLuint id = freeShaderIds_.empty() ? ++lastShaderId_ : *freeShaderIds_.begin();
    if (!freeShaderIds_.empty())
        freeShaderIds_.erase(freeShaderIds_.begin());

    ShaderObject obj;
    obj.type = type;
    obj.compiled = {UncertainBool::Unknown, UncertainBool::False};
    obj.compiledSpirv = {};
    obj.markedForDeletion = false;
    obj.compileStatus = GL_FALSE;
    shaders_[id] = obj;
    *shader = id;
    MG_Util::Debug::LogD("MG_State: Program: CreateShader success, new shader id=%u", id);
    return GL_NO_ERROR;
}

GLenum ProgramState::DeleteShader(GLuint shader) {
    MG_Util::Debug::LogD("MG_State: Program: DeleteShader called, shader id=%u", shader);
    if (!ValidateShader_(shader)) {
        MG_Util::Debug::LogE("MG_State: Program: DeleteShader error: invalid shader id %u", shader);
        return GL_INVALID_VALUE;
    }
    if (shaders_[shader].markedForDeletion) {
        MG_Util::Debug::LogW("MG_State: Program: DeleteShader warning: shader %u already marked for deletion", shader);
        return GL_NO_ERROR;
    }

    bool inUse = false;
    for (auto& [progId, program] : programs_) {
        if (std::find(program.attachedShaders.begin(),
                      program.attachedShaders.end(), shader) != program.attachedShaders.end()) {
            inUse = true;
            break;
        }
    }
    if (inUse) {
        shaders_[shader].markedForDeletion = true;
        MG_Util::Debug::LogD("MG_State: Program: DeleteShader info: shader %u marked for deferred deletion (in use)", shader);
    } else {
        shaders_.erase(shader);
        freeShaderIds_.insert(shader);
        MG_Util::Debug::LogD("MG_State: Program: DeleteShader success: shader %u deleted immediately", shader);
    }
    return GL_NO_ERROR;
}

GLenum ProgramState::UploadShaderSource(GLuint shader, GLsizei count,
                                        const GLchar** string, const GLint* length) {
    MG_Util::Debug::LogD("MG_State: Program: UploadShaderSource called, shader id=%u, count=%d", shader, count);
    if (!ValidateShader_(shader)) {
        MG_Util::Debug::LogE("MG_State: Program: UploadShaderSource error: invalid shader id %u", shader);
        return GL_INVALID_VALUE;
    }
    if (count < 0) {
        MG_Util::Debug::LogE("MG_State: Program: UploadShaderSource error: negative count %d", count);
        return GL_INVALID_VALUE;
    }

    std::string source;
    for (GLsizei i = 0; i < count; ++i) {
        const GLchar* str = string[i];
        GLint len = (length && length[i] >= 0) ? length[i] : strlen(str);
        source.append(str, len);
    }
    shaders_[shader].source = std::move(source);
    MG_Util::Debug::LogD("MG_State: Program: UploadShaderSource success: source uploaded (length=%zu)", shaders_[shader].source.size());
    return GL_NO_ERROR;
}

GLenum ProgramState::BuildShaderStage(GLuint shader) {
    MG_Util::Debug::LogD("MG_State: Program: BuildShaderStage called, shader id=%u", shader);
    if (!ValidateShader_(shader)) {
        MG_Util::Debug::LogE("MG_State: Program: BuildShaderStage error: invalid shader id %u", shader);
        return GL_INVALID_VALUE;
    }
    auto& obj = shaders_[shader];

    std::string compilationLog;
    auto spirv = MG_Util::Program::CompileGLSLToSPIRV(obj.type, obj.source, compilationLog);
    if (spirv.empty()) {
        obj.infoLog = "Shader compilation failed: " + compilationLog;
        obj.compiled = UncertainBool::False;
        obj.compileStatus = GL_FALSE;
        MG_Util::Debug::LogE("MG_State: Program: BuildShaderStage error: %s", compilationLog.c_str());
        return GL_INVALID_OPERATION;
    }
    obj.compiledSpirv = spirv;
    obj.compiled = {UncertainBool::Unknown, UncertainBool::True};
    obj.compileStatus = GL_TRUE;
    obj.infoLog.clear();
    MG_Util::Debug::LogD("MG_State: Program: BuildShaderStage success: shader %u compiled to SPIR-V (size=%zu bytes)", shader, spirv.size() * sizeof(unsigned));
    return GL_NO_ERROR;
}

bool ProgramState::SetShaderStatus(GLuint shader, GLboolean status) {
    MG_Util::Debug::LogD("MG_State: Program: SetShaderStatus called, shader id=%u, status=%d", shader, status);
    if (!ValidateShader_(shader)) {
        MG_Util::Debug::LogE("MG_State: Program: SetShaderStatus error: invalid shader id %u", shader);
        return false;
    }
    auto& obj = shaders_[shader];
    if (obj.compiled.getValue() != UncertainBool::Unknown) {
        MG_Util::Debug::LogW("MG_State: Program: SetShaderStatus warning: shader %u status already set", shader);
        return false;
    }
    obj.compiled = (status == GL_TRUE) ? UncertainBool::True : UncertainBool::False;
    obj.compileStatus = status;
    MG_Util::Debug::LogD("MG_State: Program: SetShaderStatus success: shader %u status=%d", shader, status);
    return true;
}

bool ProgramState::ValidateShader_(GLuint shader) {
    bool valid = shaders_.count(shader) && !shaders_[shader].markedForDeletion;
    MG_Util::Debug::LogD("MG_State: Program: ValidateShader_ called, shader id=%u, result=%d", shader, valid);
    return valid;
}
