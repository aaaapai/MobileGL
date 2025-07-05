//
// Created by BZLZHH on 2025/5/1.

// TODO: Add more gl error check for buffer state manager.
// TODO: Check if the state machine really meets up to OpenGL 3 standard.

#include "ProgramState.h"

// Re-Include Includes.h, cuz of the absence of Program/DebugTool.h and Program/GLSLTool.cpp.
#undef MOBILEGL_GLSLTOOL_H
#undef MOBILEGL_PROGRAM_DEBUGTOOL_H
#include "../../../Includes.h"

ProgramState::ProgramState() {
    glslang::InitializeProcess();
}

ProgramState::~ProgramState() {
    glslang::FinalizeProcess();
}

GLenum ProgramState::CreateProgram(GLuint* program) {
    MG_Util::Debug::LogD("MG_State: Program: CreateProgram called, program ptr=%p", program);
    if (!program) {
        MG_Util::Debug::LogE("MG_State: Program: CreateProgram error: program pointer is null");
        return GL_INVALID_VALUE;
    }

    GLuint id = freeProgramIds_.empty() ? ++lastProgramId_ : *freeProgramIds_.begin();
    if (!freeProgramIds_.empty())
        freeProgramIds_.erase(freeProgramIds_.begin());

    ProgramObject obj;
    obj.linked = {UncertainBool::Unknown, UncertainBool::False};
    obj.linkStatus = GL_FALSE;
    obj.markedForDeletion = false;
    programs_[id] = obj;
    *program = id;
    MG_Util::Debug::LogD("MG_State: Program: CreateProgram success, new id=%u", id);
    return GL_NO_ERROR;
}

GLenum ProgramState::DeleteProgram(GLuint program) {
    MG_Util::Debug::LogD("MG_State: Program: DeleteProgram called, id=%u", program);
    if (!ValidateProgram_(program)) {
        MG_Util::Debug::LogE("MG_State: Program: DeleteProgram error: invalid program id %u", program);
        return GL_INVALID_VALUE;
    }

    programs_.erase(program);
    //freeProgramIds_.insert(program);
    if (currentProgram_ == program)
        currentProgram_ = 0;
    MG_Util::Debug::LogD("MG_State: Program: DeleteProgram success, id=%u", program);
    return GL_NO_ERROR;
}

GLenum ProgramState::LinkShaderToProgram(GLuint program, GLuint shader) {
    MG_Util::Debug::LogD("MG_State: Program: LinkShaderToProgram called, program=%u, shader=%u",
                         program, shader);
    if (!ValidateProgram_(program) || !ValidateShader_(shader)) {
        MG_Util::Debug::LogE("MG_State: Program: LinkShaderToProgram error: invalid ids program=%u shader=%u",
                             program, shader);
        return GL_INVALID_VALUE;
    }

    auto& prog = programs_[program];
    if (std::find(prog.attachedShaders.begin(),
                  prog.attachedShaders.end(), shader) != prog.attachedShaders.end()) {
        MG_Util::Debug::LogW("MG_State: Program: LinkShaderToProgram warning: shader %u already attached to program %u",
                             shader, program);
        return GL_INVALID_OPERATION;
    }

    prog.attachedShaders.push_back(shader);
    MG_Util::Debug::LogD("MG_State: Program: LinkShaderToProgram success: shader %u attached to program %u",
                         shader, program);
    return GL_NO_ERROR;
}

GLenum ProgramState::FinalizeProgramPipeline(GLuint program) {
    MG_Util::Debug::LogD("MG_State: Program: FinalizeProgramPipeline called, id=%u", program);
    if (!ValidateProgram_(program)) {
        MG_Util::Debug::LogE("MG_State: Program: FinalizeProgramPipeline error: invalid program id %u", program);
        return GL_INVALID_VALUE;
    }
    auto& prog = programs_[program];

    for (GLuint shader : prog.attachedShaders) {
        if (!shaders_[shader].compiled.toBool()) {
            MG_Util::Debug::LogE("MG_State: Program: FinalizeProgramPipeline error: shader %u not compiled", shader);
            return GL_INVALID_OPERATION;
        }
    }

    prog.infoLog.clear();
    
    for (GLuint shaderId : prog.attachedShaders) {
        auto& shader = shaders_[shaderId];
        if (!shader.compiled.toBool() || shader.compiledSpirv.empty()) {
            prog.infoLog = "Program linking failed: \n" +
                           MG_Util::Program::GetShaderTypeName(shader.type) + " is not compiled.";
            prog.linked = UncertainBool::False;
            prog.linkStatus = GL_FALSE;
            MG_Util::Debug::LogE("MG_State: Program: FinalizeProgramPipeline error: %s", prog.infoLog.c_str());
            return GL_INVALID_OPERATION;
        }
    }

    std::vector<std::vector<unsigned>> allSpirv = 
            MG_Util::Program::CompileMultipleShadersToSPIRV(*this, prog, prog.infoLog);
    
    if (!prog.infoLog.empty() || allSpirv.empty()) {
        prog.linked = UncertainBool::False;
        prog.linkStatus = GL_FALSE;
        prog.infoLog = "Program linking failed during SPIR-V compilation: " + prog.infoLog;
        MG_Util::Debug::LogE("MG_State: Program: FinalizeProgramPipeline error: %s", prog.infoLog.c_str());
        return GL_INVALID_OPERATION;
    }
    
    MG_Util::Program::ReflectSPIRVUniforms(allSpirv, prog, prog.infoLog);
    
    for (size_t i = 0; i < prog.attachedShaders.size(); ++i) {
        GLuint shaderId = prog.attachedShaders[i];
        shaders_[shaderId].source = MG_Util::Program::CompileSPIRVToGLSL(allSpirv[i], 450, false, false);
    }
    
    if (!prog.infoLog.empty()) {
        prog.linked = UncertainBool::False;
        prog.linkStatus = GL_FALSE;
        prog.infoLog = "Program linking failed: " + prog.infoLog;
        MG_Util::Debug::LogE("MG_State: Program: FinalizeProgramPipeline error: %s", prog.infoLog.c_str());
        return GL_INVALID_OPERATION;
    }
    
    // TODO: Support explicitly assign attrib locations in the shaders.

    prog.linked = {UncertainBool::Unknown, UncertainBool::True};
    prog.linkStatus = GL_TRUE;

    CleanupShaders_();

    MG_Util::Program::DumpUniforms(*this, program);
    MG_Util::Debug::LogD("MG_State: Program: FinalizeProgramPipeline success, id=%u", program);
    return GL_NO_ERROR;
}

GLenum ProgramState::ActivateRenderProgram(GLuint program) {
    MG_Util::Debug::LogD("MG_State: Program: ActivateRenderProgram called, id=%u", program);
    if (program != 0 && !ValidateProgram_(program)) {
        MG_Util::Debug::LogE("MG_State: Program: ActivateRenderProgram error: invalid program id %u", program);
        return GL_INVALID_VALUE;
    }

    MG_Util::Program::DumpCurrentUniforms(*this);
    currentProgram_ = program;
    MG_Util::Program::DumpCurrentUniforms(*this);
    MG_Util::Debug::LogD("MG_State: Program: ActivateRenderProgram success, current program=%u", currentProgram_);
    return GL_NO_ERROR;
}

GLenum ProgramState::DefineProgramAttributeLocation(GLuint program, GLuint index, const GLchar* name) {
    MG_Util::Debug::LogD("MG_State: Program: DefineProgramAttributeLocation called, program=%u, index=%u, name=%s",
                         program, index, name);
    if (!ValidateProgram_(program)) {
        MG_Util::Debug::LogE("MG_State: Program: DefineProgramAttributeLocation error: invalid program id %u", program);
        return GL_INVALID_VALUE;
    }
    if (index >= GL_MAX_VERTEX_ATTRIBS) {
        MG_Util::Debug::LogE("MG_State: Program: DefineProgramAttributeLocation error: index %u out of range", index);
        return GL_INVALID_VALUE;
    }

    programs_[program].attribLocations[name] = index;
    MG_Util::Debug::LogD("MG_State: Program: DefineProgramAttributeLocation success: %s -> %u", name, index);
    return GL_NO_ERROR;
}

GLint ProgramState::QueryProgramAttributeLocation(GLuint program, const GLchar* name) {
    MG_Util::Debug::LogD("MG_State: Program: QueryProgramAttributeLocation called, program=%u, name=%s",
                         program, name);
    if (!ValidateProgram_(program)) {
        MG_Util::Debug::LogE("MG_State: Program: QueryProgramAttributeLocation error: invalid program id %u", program);
        return -1;
    }

    auto& prog = programs_[program];
    auto it = prog.attribLocations.find(name);
    if (it != prog.attribLocations.end()) {
        GLint loc = it->second;
        MG_Util::Debug::LogD("MG_State: Program: QueryProgramAttributeLocation success: %s -> %d", name, loc);
        return loc;
    } else {
        MG_Util::Debug::LogW("MG_State: Program: QueryProgramAttributeLocation warning: name '%s' not found, generating new one...", name);
        // Find the next available location
        GLint loc = (GLint)prog.attribLocations.size();
        bool locInUse;
        do {
            locInUse = false;
            for (const auto& pair : prog.attribLocations) {
                if (pair.second == loc) {
                    locInUse = true;
                    loc++;
                    break;
                }
            }
        } while (locInUse);
        prog.attribLocations[name] = loc;
        MG_Util::Debug::LogD("MG_State: Program: QueryProgramAttributeLocation success: %s -> %d", name, loc);
        return prog.attribLocations[name];
    }
}

GLenum ProgramState::QueryProgramStateIntVector(GLuint program, GLenum pname, GLint* params) {
    MG_Util::Debug::LogD("MG_State: Program: QueryProgramStateIntVector called, program=%u, pname=0x%X",
                         program, pname);
    if (!ValidateProgram_(program)) {
        MG_Util::Debug::LogE("MG_State: Program: QueryProgramStateIntVector error: invalid program id %u", program);
        return GL_INVALID_VALUE;
    }
    auto& prog = programs_[program];

    switch (pname) {
        case GL_LINK_STATUS:     *params = prog.linkStatus; break;
        case GL_DELETE_STATUS:   *params = prog.markedForDeletion; break;
        case GL_ATTACHED_SHADERS:*params = (GLint)prog.attachedShaders.size(); break;
        default:
            MG_Util::Debug::LogE("MG_State: Program: QueryProgramStateIntVector error: invalid pname 0x%X", pname);
            return GL_INVALID_ENUM;
    }
    MG_Util::Debug::LogD("MG_State: Program: QueryProgramStateIntVector success: value=%d", *params);
    return GL_NO_ERROR;
}

GLenum ProgramState::QueryShaderStateIntVector(GLuint shader, GLenum pname, GLint* params) {
    MG_Util::Debug::LogD("MG_State: Program: QueryShaderStateIntVector called, shader=%u, pname=0x%X",
                         shader, pname);
    if (!ValidateShader_(shader)) {
        MG_Util::Debug::LogE("MG_State: Program: QueryShaderStateIntVector error: invalid shader id %u", shader);
        return GL_INVALID_VALUE;
    }
    auto& obj = shaders_[shader];

    switch (pname) {
        case GL_COMPILE_STATUS: *params = obj.compileStatus; break;
        case GL_DELETE_STATUS:  *params = obj.markedForDeletion; break;
        case GL_SHADER_TYPE:    *params = obj.type; break;
        default:
            MG_Util::Debug::LogE("MG_State: Program: QueryShaderStateIntVector error: invalid pname 0x%X", pname);
            return GL_INVALID_ENUM;
    }
    MG_Util::Debug::LogD("MG_State: Program: QueryShaderStateIntVector success: value=%d", *params);
    return GL_NO_ERROR;
}

template<typename T>
GLenum ProgramState::SetUniform(GLuint program, GLint location, GLsizei count, const T* value, GLenum type) {
    MG_Util::Debug::LogD("MG_State: Program: SetUniform called, program=%u, location=%d, count=%d, type=0x%X",
                         program, location, count, type);
    if (!ValidateProgram_(program)) {
        MG_Util::Debug::LogE("MG_State: Program: SetUniform error: invalid program id %u", program);
        return GL_INVALID_VALUE;
    }
    if (location < -1) {
        MG_Util::Debug::LogE("MG_State: Program: SetUniform error: invalid location %d", location);
        return GL_INVALID_VALUE;
    }
    if (count < 0) {
        MG_Util::Debug::LogE("MG_State: Program: SetUniform error: invalid count %d", count);
        return GL_INVALID_VALUE;
    }
    if (location == -1) {
        MG_Util::Debug::LogW("MG_State: Program: SetUniform warning: location -1, no-op");
        return GL_NO_ERROR;
    }

    ProgramObject& prog = programs_[program];
    std::string uniformName;
    for (const auto& pair : prog.uniformLocations) {
        if (pair.second == location) {
            uniformName = pair.first;
            break;
        }
    }
    if (uniformName.empty()) {
        MG_Util::Debug::LogE("MG_State: Program: SetUniform error: no uniform at location %d in program %u",
                             location, program);
        return GL_INVALID_OPERATION;
    }

    UniformValue& uniform = prog.uniformValues[uniformName];
    
    uniform.setData(type, count, value);
    MG_Util::Debug::LogD("MG_State: Program: SetUniform success: %s set", uniformName.c_str());
    return GL_NO_ERROR;
}

template<typename T>
GLenum ProgramState::SetUniformMatrix(GLuint program, GLint location, GLsizei count, GLboolean transpose, const T* value, GLenum matrixType) {
    MG_Util::Debug::LogD("MG_State: Program: SetUniformMatrix called, program=%u, location=%d, count=%d, matrixType=0x%X",
                         program, location, count, matrixType);
    (void)transpose;
    if (!ValidateProgram_(program)) {
        MG_Util::Debug::LogE("MG_State: Program: SetUniformMatrix error: invalid program id %u", program);
        return GL_INVALID_VALUE;
    }
    if (location < -1) {
        MG_Util::Debug::LogE("MG_State: Program: SetUniformMatrix error: invalid location %d", location);
        return GL_INVALID_VALUE;
    }
    if (count < 0) {
        MG_Util::Debug::LogE("MG_State: Program: SetUniformMatrix error: invalid count %d", count);
        return GL_INVALID_VALUE;
    }
    if (location == -1) {
        MG_Util::Debug::LogW("MG_State: Program: SetUniformMatrix warning: location -1, no-op");
        return GL_NO_ERROR;
    }

    ProgramObject& prog = programs_[program];
    std::string uniformName;
    for (const auto& pair : prog.uniformLocations) {
        if (pair.second == location) {
            uniformName = pair.first;
            break;
        }
    }
    if (uniformName.empty()) {
        MG_Util::Debug::LogE("MG_State: Program: SetUniformMatrix error: no uniform at location %d in program %u",
                             location, program);
        return GL_INVALID_OPERATION;
    }

    UniformValue& uniform = prog.uniformValues[uniformName];
    uniform.setData(matrixType, count, value);
    MG_Util::Debug::LogD("MG_State: Program: SetUniformMatrix success: %s set", uniformName.c_str());
    return GL_NO_ERROR;
}

#define IMPLEMENT_UNIFORM_FUNCTIONS(type, suffix, vecType)                               \
void ProgramState::UpdateUniform##suffix##1(GLint location, type v0) {                   \
    MG_Util::Debug::LogD("MG_State: Program: UpdateUniform" #suffix "1 called, location=%d", location); \
    SetUniform<type>(currentProgram_, location, 1, &v0, vecType);                        \
}                                                                                        \
void ProgramState::UpdateUniform##suffix##2(GLint location, type v0, type v1) {          \
    type values[2] = {v0, v1};                                                           \
    MG_Util::Debug::LogD("MG_State: Program: UpdateUniform" #suffix "2 called, location=%d", location); \
    SetUniform<type>(currentProgram_, location, 1, values, vecType);                     \
}                                                                                        \
void ProgramState::UpdateUniform##suffix##3(GLint location, type v0, type v1, type v2) { \
    type values[3] = {v0, v1, v2};                                                       \
    MG_Util::Debug::LogD("MG_State: Program: UpdateUniform" #suffix "3 called, location=%d", location); \
    SetUniform<type>(currentProgram_, location, 1, values, vecType);                     \
}                                                                                        \
void ProgramState::UpdateUniform##suffix##4(GLint location, type v0, type v1, type v2, type v3) { \
    type values[4] = {v0, v1, v2, v3};                                                   \
    MG_Util::Debug::LogD("MG_State: Program: UpdateUniform" #suffix "4 called, location=%d", location); \
    SetUniform<type>(currentProgram_, location, 1, values, vecType);                     \
    } \
void ProgramState::UpdateUniform##suffix##Vector1(GLint location, GLsizei count, const type* value) { \
    MG_Util::Debug::LogD("MG_State: Program: UpdateUniform" #suffix "Vector1 called, location=%d, count=%d", location, count); \
    SetUniform<type>(currentProgram_, location, count, value, vecType);                  \
    } \
void ProgramState::UpdateUniform##suffix##Vector2(GLint location, GLsizei count, const type* value) { \
    MG_Util::Debug::LogD("MG_State: Program: UpdateUniform" #suffix "Vector2 called, location=%d, count=%d", location, count); \
    SetUniform<type>(currentProgram_, location, count, value, vecType);              \
    } \
void ProgramState::UpdateUniform##suffix##Vector3(GLint location, GLsizei count, const type* value) { \
    MG_Util::Debug::LogD("MG_State: Program: UpdateUniform" #suffix "Vector3 called, location=%d, count=%d", location, count); \
    SetUniform<type>(currentProgram_, location, count, value, vecType);              \
    } \
void ProgramState::UpdateUniform##suffix##Vector4(GLint location, GLsizei count, const type* value) { \
    MG_Util::Debug::LogD("MG_State: Program: UpdateUniform" #suffix "Vector4 called, location=%d, count=%d", location, count); \
    SetUniform<type>(currentProgram_, location, count, value, vecType);              \
    }

IMPLEMENT_UNIFORM_FUNCTIONS(GLfloat, Float, GL_FLOAT_VEC4)
IMPLEMENT_UNIFORM_FUNCTIONS(GLint, Int, GL_INT_VEC4)
IMPLEMENT_UNIFORM_FUNCTIONS(GLuint, UInt, GL_UNSIGNED_INT_VEC4)
IMPLEMENT_UNIFORM_FUNCTIONS(GLboolean, Bool, GL_BOOL_VEC4)
#undef IMPLEMENT_UNIFORM_FUNCTIONS

#define IMPLEMENT_MATRIX_FUNCTIONS(suffix, matrixType) \
void ProgramState::UpdateUniformMatrix##suffix##Vector(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { \
    MG_Util::Debug::LogD("MG_State: Program: UpdateUniformMatrix" #suffix "Vector called, location=%d, count=%d", location, count); \
    SetUniformMatrix<GLfloat>(currentProgram_, location, count, transpose, value, matrixType); \
}

IMPLEMENT_MATRIX_FUNCTIONS(2x2, GL_FLOAT_MAT2)
IMPLEMENT_MATRIX_FUNCTIONS(3x3, GL_FLOAT_MAT3)
IMPLEMENT_MATRIX_FUNCTIONS(4x4, GL_FLOAT_MAT4)
IMPLEMENT_MATRIX_FUNCTIONS(2x3, GL_FLOAT_MAT2x3)
IMPLEMENT_MATRIX_FUNCTIONS(2x4, GL_FLOAT_MAT2x4)
IMPLEMENT_MATRIX_FUNCTIONS(3x2, GL_FLOAT_MAT3x2)
IMPLEMENT_MATRIX_FUNCTIONS(3x4, GL_FLOAT_MAT3x4)
IMPLEMENT_MATRIX_FUNCTIONS(4x2, GL_FLOAT_MAT4x2)
IMPLEMENT_MATRIX_FUNCTIONS(4x3, GL_FLOAT_MAT4x3)
#undef IMPLEMENT_MATRIX_FUNCTIONS

GLint ProgramState::QueryProgramUniformLocation(GLuint program, const GLchar* name) {
    MG_Util::Debug::LogD("MG_State: Program: QueryProgramUniformLocation called, program=%u, name=%s", program, name);
    if (!ValidateProgram_(program)) {
        MG_Util::Debug::LogE("MG_State: Program: QueryProgramUniformLocation error: invalid program id %u", program);
        return -1;
    }
    auto& prog = programs_[program];
    auto it = prog.uniformLocations.find(name);
    if (it != prog.uniformLocations.end()) {
        GLint loc = it->second;
        MG_Util::Debug::LogD("MG_State: Program: QueryProgramUniformLocation success: %s -> %d", name, loc);
        return loc;
    }
    MG_Util::Debug::LogW("MG_State: Program: QueryProgramUniformLocation warning: name '%s' not found", name);
    return -1;
}

bool ProgramState::SetProgramStatus(GLuint program, GLboolean status) {
    MG_Util::Debug::LogD("MG_State: Program: SetProgramStatus called, program=%u, status=%d", program, status);
    if (!ValidateProgram_(program)) {
        MG_Util::Debug::LogE("MG_State: Program: SetProgramStatus error: invalid program id %u", program);
        return false;
    }
    auto& prog = programs_[program];
    if (prog.linked.getValue() != UncertainBool::Unknown) {
        MG_Util::Debug::LogW("MG_State: Program: SetProgramStatus warning: link state already known");
        return false;
    }
    prog.linked = (status == GL_TRUE) ? UncertainBool::True : UncertainBool::False;
    prog.linkStatus = status;
    MG_Util::Debug::LogD("MG_State: Program: SetProgramStatus success, status=%d", status);
    return true;
}

GLuint ProgramState::GetCurrentProgram() const {
    MG_Util::Debug::LogD("MG_State: Program: GetCurrentProgram called, returning %u", currentProgram_);
    return currentProgram_;
}

const ShaderObject* ProgramState::GetShaderObject(GLuint shader) const {
    MG_Util::Debug::LogD("MG_State: Program: GetShaderObject called, shader=%u", shader);
    auto it = shaders_.find(shader);
    if (it == shaders_.end()) {
        MG_Util::Debug::LogW("MG_State: Program: GetShaderObject warning: invalid shader id %u", shader);
        return nullptr;
    }
    MG_Util::Debug::LogD("MG_State: Program: GetShaderObject success, returning object");
    return &(it->second);
}

const ProgramObject* ProgramState::GetProgramObject(GLuint program) const {
    MG_Util::Debug::LogD("MG_State: Program: GetProgramObject called, program=%u", program);
    auto it = programs_.find(program);
    if (it == programs_.end()) {
        MG_Util::Debug::LogW("MG_State: Program: GetProgramObject warning: invalid program id %u", program);
        return nullptr;
    }
    MG_Util::Debug::LogD("MG_State: Program: GetProgramObject success, returning object");
    return &(it->second);
}

bool ProgramState::ValidateProgram_(GLuint program) {
    bool ok = programs_.count(program) && !programs_[program].markedForDeletion;
    MG_Util::Debug::LogD("MG_State: Program: ValidateProgram_ called, program=%u, result=%d", program, ok);
    return ok;
}

void ProgramState::CleanupShaders_() {
    MG_Util::Debug::LogD("MG_State: Program: CleanupShaders_ start");
    auto it = shaders_.begin();
    while (it != shaders_.end()) {
        if (it->second.markedForDeletion) {
            bool inUse = false;
            for (auto& [progId, program] : programs_) {
                if (std::find(program.attachedShaders.begin(),
                              program.attachedShaders.end(), it->first) != program.attachedShaders.end()) {
                    inUse = true;
                    break;
                }
            }
            if (!inUse) {
                MG_Util::Debug::LogD("MG_State: Program: CleanupShaders_ deleting shader %u", it->first);
                // freeShaderIds_.insert(it->first);
                it = shaders_.erase(it);
                continue;
            }
        }
        ++it;
    }
    MG_Util::Debug::LogD("MG_State: Program: CleanupShaders_ end");
}

// UniformValue
template<typename T>
void UniformValue::setData(GLenum uniformType, GLsizei count_, const T* values) {
    MG_Util::Debug::LogD("MG_State: Program: UniformValue::setData called, type=0x%X, count=%d", uniformType, count_);
    //type = uniformType;
    count = count_;

    floatData.clear();
    intData.clear();
    uintData.clear();
    boolData.clear();

    switch (type) {
        case GL_FLOAT:                           numElements = count_ * 1; break;
        case GL_FLOAT_VEC2:                      numElements = count_ * 2; break;
        case GL_FLOAT_VEC3:                      numElements = count_ * 3; break;
        case GL_FLOAT_VEC4:                      numElements = count_ * 4; break;
        case GL_INT:                             numElements = count_ * 1; break;
        case GL_INT_VEC2:                        numElements = count_ * 2; break;
        case GL_INT_VEC3:                        numElements = count_ * 3; break;
        case GL_INT_VEC4:                        numElements = count_ * 4; break;
        case GL_UNSIGNED_INT:                    numElements = count_ * 1; break;
        case GL_UNSIGNED_INT_VEC2:               numElements = count_ * 2; break;
        case GL_UNSIGNED_INT_VEC3:               numElements = count_ * 3; break;
        case GL_UNSIGNED_INT_VEC4:               numElements = count_ * 4; break;
        case GL_BOOL:                            numElements = count_ * 1; break;
        case GL_BOOL_VEC2:                       numElements = count_ * 2; break;
        case GL_BOOL_VEC3:                       numElements = count_ * 3; break;
        case GL_BOOL_VEC4:                       numElements = count_ * 4; break;
        case GL_FLOAT_MAT2:                      numElements = count_ * 4; break; // 2x2
        case GL_FLOAT_MAT3:                      numElements = count_ * 9; break; // 3x3
        case GL_FLOAT_MAT4:                      numElements = count_ * 16; break; // 4x4
        case GL_FLOAT_MAT2x3:                    numElements = count_ * 6; break; // 2x3
        case GL_FLOAT_MAT2x4:                    numElements = count_ * 8; break; // 2x4
        case GL_FLOAT_MAT3x2:                    numElements = count_ * 6; break; // 3x2
        case GL_FLOAT_MAT3x4:                    numElements = count_ * 12; break; // 3x4
        case GL_FLOAT_MAT4x2:                    numElements = count_ * 8; break; // 4x2
        case GL_FLOAT_MAT4x3:                    numElements = count_ * 12; break; // 4x3
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_BUFFER:
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:    numElements = count_ * 1; break;
        default:
            MG_Util::Debug::LogW("MG_State: Program: UniformValue::setData warning: unsupported type 0x%X for numElements", uniformType);
            numElements = 1;
            return;
    }

    for (GLsizei i = 0; i < numElements; ++i) {
        switch (uniformType) {
            case GL_FLOAT:
            case GL_FLOAT_VEC2:
            case GL_FLOAT_VEC3:
            case GL_FLOAT_VEC4:
            case GL_FLOAT_MAT2:
            case GL_FLOAT_MAT3:
            case GL_FLOAT_MAT4:
            case GL_FLOAT_MAT2x3:
            case GL_FLOAT_MAT2x4:
            case GL_FLOAT_MAT3x2:
            case GL_FLOAT_MAT3x4:
            case GL_FLOAT_MAT4x2:
            case GL_FLOAT_MAT4x3:
                floatData.push_back(static_cast<GLfloat>(values[i]));
                break;
            case GL_INT:
            case GL_INT_VEC2:
            case GL_INT_VEC3:
            case GL_INT_VEC4:
            case GL_SAMPLER_1D:
            case GL_SAMPLER_2D:
            case GL_SAMPLER_3D:
            case GL_SAMPLER_CUBE:
            case GL_SAMPLER_1D_SHADOW:
            case GL_SAMPLER_2D_SHADOW:
            case GL_SAMPLER_CUBE_SHADOW:
            case GL_SAMPLER_1D_ARRAY:
            case GL_SAMPLER_2D_ARRAY:
            case GL_SAMPLER_1D_ARRAY_SHADOW:
            case GL_SAMPLER_2D_ARRAY_SHADOW:
            case GL_SAMPLER_BUFFER:
            case GL_SAMPLER_2D_MULTISAMPLE:
            case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
                intData.push_back(static_cast<GLint>(values[i]));
                break;
            case GL_UNSIGNED_INT:
            case GL_UNSIGNED_INT_VEC2:
            case GL_UNSIGNED_INT_VEC3:
            case GL_UNSIGNED_INT_VEC4:
                uintData.push_back(static_cast<GLuint>(values[i]));
                break;
            case GL_BOOL:
            case GL_BOOL_VEC2:
            case GL_BOOL_VEC3:
            case GL_BOOL_VEC4:
                boolData.push_back(values[i] ? GL_TRUE : GL_FALSE);
                break;
            default:
                MG_Util::Debug::LogW("MG_State: Program: UniformValue::setData warning: unsupported type 0x%X", uniformType);
                break;
        }
    }
    MG_Util::Debug::LogD("MG_State: Program: UniformValue::setData end");
}
