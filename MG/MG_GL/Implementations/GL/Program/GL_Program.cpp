//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_Program.h"

#undef MOBILEGL_GLSLTOOL_H
#include "../../../../Includes.h"

namespace MG_GL::GL {
    inline bool IsSamplerType(GLenum type) {
        switch (type) {
            case GL_SAMPLER_2D:
            case GL_SAMPLER_3D:
            case GL_SAMPLER_CUBE:
            case GL_SAMPLER_2D_SHADOW:
            case GL_SAMPLER_2D_ARRAY:
            case GL_SAMPLER_2D_ARRAY_SHADOW:
            case GL_SAMPLER_CUBE_SHADOW:
                return true;
            default:
                return false;
        }
    }

    inline size_t GetUniformSize(GLenum type) {
        switch (type) {
            case GL_FLOAT: return sizeof(float);
            case GL_FLOAT_VEC2: return 2 * sizeof(float);
            case GL_FLOAT_VEC3: return 3 * sizeof(float);
            case GL_FLOAT_VEC4: return 4 * sizeof(float);
            case GL_FLOAT_MAT2: return 4 * sizeof(float);
            case GL_FLOAT_MAT3: return 9 * sizeof(float);
            case GL_FLOAT_MAT4: return 16 * sizeof(float);
            case GL_INT:
            case GL_BOOL:
                return sizeof(int);
            case GL_INT_VEC2: return 2 * sizeof(int);
            case GL_INT_VEC3: return 3 * sizeof(int);
            case GL_INT_VEC4: return 4 * sizeof(int);
            case GL_UNSIGNED_INT: return sizeof(uint32_t);
            case GL_UNSIGNED_INT_VEC2: return 2 * sizeof(uint32_t);
            case GL_UNSIGNED_INT_VEC3: return 3 * sizeof(uint32_t);
            case GL_UNSIGNED_INT_VEC4: return 4 * sizeof(uint32_t);
            default: return 0;
        }
    }

    inline size_t AlignSize(size_t size, size_t alignment) {
        return (size + alignment - 1) & ~(alignment - 1);
    }
    
    void CreateDefaultUBO(MG_Diligent::GLProgramInfo& programInfo) {
        MG_Util::Debug::LogD("CreateDefaultUBO for program");
        size_t uboSize = 0;
        std::vector<size_t> uniformSizes;

        for (auto& [name, uniform] : programInfo.programObj.uniformValues) {
            if (IsSamplerType(uniform.type)) continue;

            size_t size = GetUniformSize(uniform.type);
            size_t alignedSize = AlignSize(size, 16);
            uniformSizes.push_back(alignedSize);
            MG_Util::Debug::LogD("  Uniform '%s': size = %zu, alignedSize = %zu",
                                 name.c_str(), size, alignedSize);
            uboSize += alignedSize;
        }

        if (uboSize > 0) {
            Diligent::BufferDesc BuffDesc;
            BuffDesc.Name = "MG_DEFAULT_UBO";
            BuffDesc.Size = uboSize;
            BuffDesc.Usage = Diligent::USAGE_DYNAMIC;
            BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
            BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

            MG_Diligent::g_pDevice->CreateBuffer(BuffDesc, nullptr, &programInfo.pDefaultUBO);
            MG_Util::Debug::LogD("Created default UBO: size = %zu", uboSize);
        } else {
            MG_Util::Debug::LogD("No non-sampler uniforms found, default UBO not created.");
        }
    }

    void RecordUniformOffsets(MG_Diligent::GLProgramInfo& programInfo) {
        MG_Util::Debug::LogD("RecordUniformOffsets for program");
        size_t offset = 0;
        programInfo.uniformOffsets.clear();

        for (auto& [name, uniform] : programInfo.programObj.uniformValues) {
            if (IsSamplerType(uniform.type)) continue;

            size_t size = GetUniformSize(uniform.type);
            size_t alignedSize = AlignSize(size, 16);

            programInfo.uniformOffsets[name] = offset;
            MG_Util::Debug::LogD("  Uniform '%s': offset = %zu, size = %zu, alignedSize = %zu",
                                 name.c_str(), offset, size, alignedSize);
            offset += alignedSize;
        }
    }
    
    Diligent::VALUE_TYPE ConvertGLTypeToDiligent(GLenum type) {
        switch (type) {
            case GL_FLOAT: return Diligent::VT_FLOAT32;
            case GL_FLOAT_VEC2: return Diligent::VT_FLOAT32;
            case GL_FLOAT_VEC3: return Diligent::VT_FLOAT32;
            case GL_FLOAT_VEC4: return Diligent::VT_FLOAT32;
            case GL_SHORT: return Diligent::VT_INT16;
            case GL_UNSIGNED_SHORT: return Diligent::VT_UINT16;
            case GL_BYTE: return Diligent::VT_INT8;
            case GL_UNSIGNED_BYTE: return Diligent::VT_UINT8;
            case GL_INT: return Diligent::VT_INT32;
            case GL_INT_VEC2: return Diligent::VT_INT32;
            case GL_INT_VEC3: return Diligent::VT_INT32;
            case GL_INT_VEC4: return Diligent::VT_INT32;
            case GL_UNSIGNED_INT: return Diligent::VT_UINT32;
            case GL_UNSIGNED_INT_VEC2: return Diligent::VT_UINT32;
            case GL_UNSIGNED_INT_VEC3: return Diligent::VT_UINT32;
            case GL_UNSIGNED_INT_VEC4: return Diligent::VT_UINT32;
            case GL_FLOAT_MAT2: return Diligent::VT_FLOAT32;
            case GL_FLOAT_MAT3: return Diligent::VT_FLOAT32;
            case GL_FLOAT_MAT4: return Diligent::VT_FLOAT32;
            case GL_FLOAT_MAT2x3: return Diligent::VT_FLOAT32;
            case GL_FLOAT_MAT2x4: return Diligent::VT_FLOAT32;
            case GL_FLOAT_MAT3x2: return Diligent::VT_FLOAT32;
            case GL_FLOAT_MAT3x4: return Diligent::VT_FLOAT32;
            case GL_FLOAT_MAT4x2: return Diligent::VT_FLOAT32;
            case GL_FLOAT_MAT4x3: return Diligent::VT_FLOAT32;
            default: return Diligent::VT_UNDEFINED;
        }
    }

    GLint GetGLTypeComponentCount(GLenum type) {
        switch (type) {
            case GL_FLOAT: return 1;
            case GL_FLOAT_VEC2: return 2;
            case GL_FLOAT_VEC3: return 3;
            case GL_FLOAT_VEC4: return 4;
            case GL_INT: return 1;
            case GL_INT_VEC2: return 2;
            case GL_INT_VEC3: return 3;
            case GL_INT_VEC4: return 4;
            case GL_UNSIGNED_INT: return 1;
            case GL_UNSIGNED_INT_VEC2: return 2;
            case GL_UNSIGNED_INT_VEC3: return 3;
            case GL_UNSIGNED_INT_VEC4: return 4;
            case GL_FLOAT_MAT2: return 4;
            case GL_FLOAT_MAT3: return 9;
            case GL_FLOAT_MAT4: return 16; // 4 * 4
            case GL_FLOAT_MAT2x3: return 6; // 2 * 3
            case GL_FLOAT_MAT2x4: return 8; // 2 * 4
            case GL_FLOAT_MAT3x2: return 6; // 3 * 2
            case GL_FLOAT_MAT3x4: return 12; // 3 * 4
            case GL_FLOAT_MAT4x2: return 8; // 4 * 2
            case GL_FLOAT_MAT4x3: return 12; // 4 * 3
            default: return 0;
        }
    }
    
    GLuint CreateShader(GLenum type) {
        MG_Util::Debug::LogD("glCreateShader, type: %s", MG_Util::Debug::GLEnumToString(type));
        GLuint shader;
        GLenum result = MG_State::CreateShader(type, &shader);
        if (result == GL_NO_ERROR) {
            MG_Util::Debug::LogD("Created shader ID: %u", shader);
            MG_Diligent::g_ShaderMap[shader] = nullptr;
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
        if (result == GL_NO_ERROR) {
            auto it = MG_Diligent::g_ShaderMap.find(shader);
            if (it != MG_Diligent::g_ShaderMap.end()) {
                if (it->second) {
                    it->second->Release();
                }
                MG_Diligent::g_ShaderMap.erase(it);
            }
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error deleting shader: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    void DeleteProgram(GLuint program) {
        MG_Util::Debug::LogD("glDeleteProgram, program: %u", program);
        GLenum result = MG_State::DeleteProgram(program);
        if (result == GL_NO_ERROR) {
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error deleting program: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    void AttachShader(GLuint program, GLuint shader) {
        MG_Util::Debug::LogD("glAttachShader, program: %u, shader: %u", program, shader);
        GLenum result = MG_State::LinkShaderToProgram(program, shader);
        if (result == GL_NO_ERROR) {
            return;
        }
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
        // Defer actual compilation to LinkProgram
        GLenum result = MG_State::BuildShaderStage(shader);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error marking shader for compilation: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    void LinkProgram(GLuint program) {
        MG_Util::Debug::LogD("glLinkProgram, program: %u", program);
        GLenum result = MG_State::FinalizeProgramPipeline(program);
        if (result == GL_NO_ERROR) {
            auto& programInfo = MG_Diligent::g_ProgramMap[program];
            auto& programObj = MG_State_T::programState->programs_[program];

            for (const auto& shaderId : programObj.attachedShaders) {
                programInfo.AttachedShadersID.push_back(shaderId);
            }

            MG_Global::unordered_map<GLuint, std::string> shaderSources;
            for (GLuint shaderId : programInfo.AttachedShadersID) {
                auto& shaderObj = MG_State_T::programState->shaders_[shaderId];
                shaderSources[shaderId] = shaderObj.source;
            }

            MG_Util::Program::GenerateDefaultUBOForGLSL_Multi(shaderSources);
            
            // Compile attached shaders
            for (GLuint shaderId : programInfo.AttachedShadersID) {
                auto& shaderObj = MG_State_T::programState->shaders_[shaderId];
                // Compile only if not already compiled in Diligent
                if (MG_Diligent::g_ShaderMap.find(shaderId) == MG_Diligent::g_ShaderMap.end() || MG_Diligent::g_ShaderMap[shaderId] == nullptr) {
                    GLenum shaderType = shaderObj.type;
                    std::string sourceStr = shaderSources[shaderId];
                    MG_Util::Debug::LogD("Shader ID: %u, type: %s\nConverted source:\n%s",
                                         shaderId, MG_Util::Debug::GLEnumToString(shaderType), sourceStr.c_str());

                    Diligent::ShaderCreateInfo ShaderCI;
                    ShaderCI.Source = sourceStr.c_str();
                    ShaderCI.EntryPoint = "main";

                    switch (shaderType) {
                        case GL_VERTEX_SHADER: ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX; break;
                        case GL_FRAGMENT_SHADER: ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL; break;
                        case GL_GEOMETRY_SHADER: ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_GEOMETRY; break;
                        case GL_TESS_CONTROL_SHADER: ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_HULL; break;
                        case GL_TESS_EVALUATION_SHADER: ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_DOMAIN; break;
                        case GL_COMPUTE_SHADER: ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_COMPUTE; break;
                        default: MG_Util::Debug::LogW("Unsupported shader type for compilation: %u", shaderType); continue;
                    }

                    ShaderCI.Desc.Name = ("Shader_" + std::to_string(shaderId)).c_str();
                    ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL_VERBATIM;

                    Diligent::IShader* pShader = nullptr;
                    MG_Diligent::g_pDevice->CreateShader(ShaderCI, &pShader);

                    if (pShader) {
                        MG_Diligent::g_ShaderMap[shaderId] = pShader;
                        programInfo.AttachedShaders.push_back(pShader);
                        shaderObj.compiled = UncertainBool::True;
                        shaderObj.compileStatus = GL_TRUE;
                        MG_Util::Debug::LogD("Successfully compiled shader ID: %u for program %u", shaderId, program);
                    } else {
                        shaderObj.compiled = UncertainBool::False;
                        shaderObj.compileStatus = GL_FALSE;
                        MG_Util::Debug::LogE("Failed to compile shader ID: %u for program %u", shaderId, program);
                    }
                } else {
                    // Shader already compiled, just add to programInfo
                    programInfo.AttachedShaders.push_back(MG_Diligent::g_ShaderMap[shaderId]);
                }
            }

            programInfo.id = program;
            programInfo.inputLayout.clear();

            auto* pVAO = MG_State_T::vertexArrayState->GetCurrentVAO();
            if (!pVAO) return;

            for (const auto& [index, attrib] : pVAO->attribs) {
                if (attrib.enabled) {
                    Diligent::LayoutElement elem;
                    elem.InputIndex = index;
                    elem.BufferSlot = 0;
                    elem.NumComponents = attrib.size;
                    elem.ValueType = ConvertGLTypeToDiligent(attrib.type);
                    elem.IsNormalized = attrib.normalized;
                    elem.RelativeOffset = static_cast<GLuint>(reinterpret_cast<size_t>(attrib.pointer));

                    programInfo.inputLayout.push_back(elem);
                }
            }

            programInfo.psoDirty = true;
            programInfo.psoStateHash = 0;
            programInfo.uniformStages.clear();
            programInfo.programObj = MG_State_T::programState->programs_[program];
            
            if (programInfo.pResourceBinding) {
                programInfo.pResourceBinding->Release();
                programInfo.pResourceBinding = nullptr;
            }

            CreateDefaultUBO(programInfo);
            RecordUniformOffsets(programInfo);

            programObj.linked = true;
            programObj.linkStatus = GL_TRUE;
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error linking program: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    void UseProgram(GLuint program) {
        MG_Util::Debug::LogD("glUseProgram, program: %u", program);
        GLenum result = MG_State::ActivateRenderProgram(program);
        if (result == GL_NO_ERROR) {
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error using program: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    void BindAttribLocation(GLuint program, GLuint index, const GLchar* name) {
        MG_Util::Debug::LogD("glBindAttribLocation, program: %u, index: %u, name: %s", program, index, name);
        GLenum result = MG_State::DefineProgramAttributeLocation(program, index, name);
        if (result == GL_NO_ERROR) return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error binding attribute: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    GLint GetAttribLocation(GLuint program, const GLchar* name) {
        MG_Util::Debug::LogD("glGetAttribLocation, program: %u, name: %s", program, name);
        GLint location = MG_State::QueryProgramAttributeLocation(program, name);
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