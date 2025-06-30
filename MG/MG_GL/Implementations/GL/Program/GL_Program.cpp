//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_Program.h"

#undef MOBILEGL_GLSLTOOL_H
#include "../../../../Includes.h"

namespace MG_GL::GL {
    void CreateDefaultUBO(MG_Diligent::GLProgramInfo& programInfo, size_t uboSize) {
        MG_Util::Debug::LogD("CreateDefaultUBO for program, size = %zu", uboSize);
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
            MG_Util::Debug::LogD("UBO size is 0, default UBO not created.");
        }
    }

    size_t RecordUniformOffsets(MG_Diligent::GLProgramInfo& programInfo, const MG_Global::unordered_map<GLuint, std::string>& shaderSources) {
        MG_Util::Debug::LogD("RecordUniformOffsets for program %u using SPIRV-Cross", programInfo.id);
        size_t uboTotalSize = 0;
        programInfo.uniformOffsets.clear();

        GLuint reflectionShaderId = 0;
        for (auto shaderId : programInfo.AttachedShadersID) {
            auto& shaderObj = MG_State_T::programState->shaders_[shaderId];
            MG_Util::Debug::LogD("  Checking attached shader %u, type: %s", shaderId, MG_Util::Debug::GLEnumToString(shaderObj.type));
            if (shaderObj.type == GL_VERTEX_SHADER) {
                reflectionShaderId = shaderId;
                MG_Util::Debug::LogD("  Found vertex shader %u for reflection.", reflectionShaderId);
                break;
            }
        }
        if (!reflectionShaderId && !programInfo.AttachedShadersID.empty()) {
            reflectionShaderId = programInfo.AttachedShadersID[0];
            MG_Util::Debug::LogD("  No vertex shader found, using first attached shader %u for reflection.", reflectionShaderId);
        }
        if (!reflectionShaderId) {
            MG_Util::Debug::LogW("  No suitable shader found for UBO reflection.");
            return uboTotalSize;
        }

        MG_Util::Debug::LogD("  Using shader %u for UBO reflection.", reflectionShaderId);
        auto itSource = shaderSources.find(reflectionShaderId);
        if (itSource == shaderSources.end()) {
            MG_Util::Debug::LogE("  Could not find source for reflection shader %u.", reflectionShaderId);
            return uboTotalSize;
        }

        MG_Util::Debug::LogD("  Found source for shader %u.", reflectionShaderId);
        MG_Util::Debug::LogD("  Shader source for reflection:\n%s", itSource->second.c_str());
        std::string infoLog;
        auto spirv = MG_Util::Program::CompileGLSLToSPIRV(
                MG_State_T::programState->shaders_[reflectionShaderId].type,
                itSource->second,
                infoLog,
                true
        );
        if (spirv.empty()) {
            MG_Util::Debug::LogE("GLSL to SPIR-V compilation failed for shader %u: %s", reflectionShaderId, infoLog.c_str());
            return uboTotalSize;
        }

        MG_Util::Debug::LogD("  Successfully compiled GLSL to SPIR-V for shader %u.", reflectionShaderId);

        spvc_context context = nullptr;
        spvc_parsed_ir ir = nullptr;
        spvc_compiler compiler = nullptr;
        spvc_resources resources = nullptr;

        if (spvc_context_create(&context) != SPVC_SUCCESS) {
            MG_Util::Debug::LogE("SPIRV-Cross: Failed to create context: %s", spvc_context_get_last_error_string(context));
            return uboTotalSize;
        }

        if (spvc_context_parse_spirv(context, spirv.data(), spirv.size(), &ir) != SPVC_SUCCESS) {
            MG_Util::Debug::LogE("SPIRV-Cross: Failed to parse SPIR-V: %s", spvc_context_get_last_error_string(context));
            spvc_context_destroy(context);
            return uboTotalSize;
        }

        if (spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler) != SPVC_SUCCESS) {
            MG_Util::Debug::LogE("SPIRV-Cross: Failed to create compiler: %s", spvc_context_get_last_error_string(context));
            spvc_context_destroy(context);
            return uboTotalSize;
        }

        MG_Util::Debug::LogD("  spvc_context_create_compiler successful.");

        if (spvc_compiler_create_shader_resources(compiler, &resources) != SPVC_SUCCESS) {
            MG_Util::Debug::LogE("SPIRV-Cross: Failed to create shader resources: %s", spvc_context_get_last_error_string(context));
            spvc_context_destroy(context);
            return uboTotalSize;
        }

        MG_Util::Debug::LogD("  spvc_compiler_create_shader_resources successful.");

        const spvc_reflected_resource* ubo_list = nullptr;
        size_t ubo_count = 0;
        if (spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, &ubo_list, &ubo_count) != SPVC_SUCCESS) {
            MG_Util::Debug::LogE("SPIRV-Cross: Failed to get resource list for UBO: %s", spvc_context_get_last_error_string(context));
            spvc_context_destroy(context);
            return uboTotalSize;
        }

        MG_Util::Debug::LogD("  Found %zu UBO resources.", ubo_count);

        for (size_t i = 0; i < ubo_count; i++) {
            const char* name = ubo_list[i].name;
            if (!name || strcmp(name, "MG_DEFAULT_UBO") != 0) {
                MG_Util::Debug::LogD("  Skipping UBO '%s'.", name ? name : "<unnamed>");
                continue;
            }

            MG_Util::Debug::LogD("  Found UBO 'MG_DEFAULT_UBO'.");
            spvc_type_id ubo_type_id = ubo_list[i].type_id;
            spvc_type ubo_type = spvc_compiler_get_type_handle(compiler, ubo_type_id);

            size_t currentUboSize = 0;
            if (spvc_compiler_get_declared_struct_size(compiler, ubo_type, &currentUboSize) != SPVC_SUCCESS) {
                MG_Util::Debug::LogE("SPIRV-Cross: Failed to get declared struct size: %s", spvc_context_get_last_error_string(context));
                spvc_context_destroy(context);
                return uboTotalSize;
            }

            uboTotalSize = currentUboSize;
            MG_Util::Debug::LogD("  UBO 'MG_DEFAULT_UBO' total size: %zu", uboTotalSize);

            size_t member_count = spvc_type_get_num_member_types(ubo_type);
            for (size_t member_index = 0; member_index < member_count; member_index++) {
                const char* member_name = programInfo.uniformBufferNames[member_index].c_str();

                unsigned offset;
                spvc_compiler_type_struct_member_offset(compiler, ubo_type, member_index, &offset);

                if (member_name) {
                    programInfo.uniformOffsets[member_name] = offset;
                    MG_Util::Debug::LogD(
                            "  Uniform '%s' offset = %u (via SPIRV-Cross)",
                            member_name,
                            offset
                    );
                }
            }
            break;
        }

        spvc_context_destroy(context);
        MG_Util::Debug::LogD("RecordUniformOffsets completed for program %u, UBO total size: %zu", programInfo.id, uboTotalSize);
        return uboTotalSize;
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
            // Do not delete the shader object since it may be used in glDraw*.
            /*
            auto it = MG_Diligent::g_ShaderMap.find(shader);
            if (it != MG_Diligent::g_ShaderMap.end()) {
                if (it->second) {
                    it->second->Release();
                }
                MG_Diligent::g_ShaderMap.erase(it);
            }
            return;
            */
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogW("Error deleting shader: %s", MG_Util::Debug::GLEnumToString(result));
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
        if (count > 0 && string != nullptr && string[0] != nullptr) {
            MG_Util::Debug::LogD("Shader source for shader %u:\n%s", shader, string[0]);
        } else {
            MG_Util::Debug::LogD("Shader source for shader %u is empty or null.", shader);
        }
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

            programInfo.id = program;

            for (const auto& shaderId : programObj.attachedShaders) {
                programInfo.AttachedShadersID.push_back(shaderId);
            }

            MG_Global::unordered_map<GLuint, std::string> shaderSources;
            MG_Global::unordered_map<GLuint, std::string> finalShaderSources;
            for (GLuint shaderId : programObj.attachedShaders) {
                auto it = MG_State_T::programState->shaders_.find(shaderId);
                if (it != MG_State_T::programState->shaders_.end() && !it->second.markedForDeletion) {
                    shaderSources[it->first] = it->second.source;
//                    MG_Util::Program::RenameGLSLBuiltinsForVulkan(shaderSources[it->first]);
                }
            }

            MG_Util::Program::GenerateDefaultUBOForGLSL_Multi(shaderSources, programInfo.uniformBufferNames);

            for (GLuint shaderId : programObj.attachedShaders) {
                auto it = MG_State_T::programState->shaders_.find(shaderId);
                if (it != MG_State_T::programState->shaders_.end() && !it->second.markedForDeletion) {
                    std::string shaderSource;
                    if (it->second.type == GL_VERTEX_SHADER) {
                        std::string infoLog;

                        const char* fragmentShaderSource = nullptr;
                        for (GLuint otherShaderId : programObj.attachedShaders) {
                            if (otherShaderId != shaderId && MG_State_T::programState->shaders_[otherShaderId].type == GL_FRAGMENT_SHADER) {
                                fragmentShaderSource = shaderSources[otherShaderId].c_str();
                                break;
                            }
                        }
                        auto spirv = MG_Util::Program::CompileGLSLToSPIRV(it->second.type,
                                                                          shaderSources[shaderId],
                                                                          infoLog, true, GL_FRAGMENT_SHADER, fragmentShaderSource);
                        if (spirv.empty()) {
                            MG_Util::Debug::LogE("Failed to compile vertex shader %u: %s", shaderId,
                                                 infoLog.c_str());
                            continue;
                        }

                        shaderSource = MG_Util::Program::BindInputLayoutLocationsForGLSL(spirv,
                                                                                         programObj.attribLocations);
                    } else if (it->second.type == GL_FRAGMENT_SHADER) {
                        std::string infoLog;

                        const char* vertexShaderSource = nullptr;
                        for (GLuint otherShaderId : programObj.attachedShaders) {
                            if (otherShaderId != shaderId && MG_State_T::programState->shaders_[otherShaderId].type == GL_VERTEX_SHADER) {
                                vertexShaderSource = shaderSources[otherShaderId].c_str();
                                break;
                            }
                        }

                        auto spirv = MG_Util::Program::CompileGLSLToSPIRV(it->second.type,
                                                                          shaderSources[shaderId],
                                                                          infoLog, true, 
                                                                          GL_VERTEX_SHADER, vertexShaderSource);
                        if (spirv.empty()) {
                            MG_Util::Debug::LogE("Failed to compile fragment shader %u to SPIRV: %s", shaderId,
                                                 infoLog.c_str());
                            continue;
                        }

                        shaderSource = MG_Util::Program::CompileSPIRVToGLSL(spirv, 450, false, true);
                    } else {
                        shaderSource = it->second.source;
                    }
                    finalShaderSources[it->first] = shaderSource;
                }
            }
            
            
            MG_Util::Debug::LogD("Shader sources after UBO generation for program %u:", program);
            for (const auto& [shaderId, source] : finalShaderSources) {
                MG_Util::Debug::LogD("  Program %u Shader %u:\n%s", program, shaderId, source.c_str());
            }

            size_t uboTotalSize = RecordUniformOffsets(programInfo, finalShaderSources);
            CreateDefaultUBO(programInfo, uboTotalSize);
            
            // Compile attached shaders
            for (GLuint shaderId : programInfo.AttachedShadersID) {
                auto& shaderObj = MG_State_T::programState->shaders_[shaderId];
                // Compile only if not already compiled in Diligent
                if (MG_Diligent::g_ShaderMap.find(shaderId) == MG_Diligent::g_ShaderMap.end() || MG_Diligent::g_ShaderMap[shaderId] == nullptr) {
                    GLenum shaderType = shaderObj.type;
                    std::string sourceStr = finalShaderSources[shaderId];
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
                    Diligent::RefCntAutoPtr<Diligent::IDataBlob> pMsg;
                    MG_Diligent::g_pDevice->CreateShader(ShaderCI, &pShader, &pMsg);

                    if (pShader) {
                        MG_Diligent::g_ShaderMap[shaderId] = pShader;
                        programInfo.AttachedShaders.push_back(pShader);
                        shaderObj.compiled = UncertainBool::True;
                        shaderObj.compileStatus = GL_TRUE;
                        MG_Util::Debug::LogD("Successfully compiled shader ID: %u for program %u", shaderId, program);
                    } else {
                        shaderObj.compiled = UncertainBool::False;
                        shaderObj.compileStatus = GL_FALSE;
                        MG_Util::Debug::LogE("Failed to compile shader ID: %u for program %u. \ncompiler info:\n%s", shaderId, program, pMsg->GetConstDataPtr<char>());
                    }
                } else {
                    // Shader already compiled, just add to programInfo
                    programInfo.AttachedShaders.push_back(MG_Diligent::g_ShaderMap[shaderId]);
                }
            }

            programInfo.inputLayout.clear();

            auto* pVAO = MG_State_T::vertexArrayState->GetCurrentVAO();
            if (!pVAO) return;

//            for (const auto& [index, attrib] : pVAO->attribs) {
            for (uint32_t index = 0; index < MG_Constants::VertexArray::MAX_VERTEX_ATTRIBS; ++index) {
                const auto& attrib = pVAO->attribs[index];
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
        MG_Util::Debug::LogW("Error getting program param: %s", MG_Util::Debug::GLEnumToString(result));
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