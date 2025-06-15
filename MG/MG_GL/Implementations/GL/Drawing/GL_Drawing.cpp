//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_Drawing.h"

namespace MG_GL::GL {
    void CreateRenderPassAndFramebuffer(GLuint framebuffer,
                                        const FramebufferObject& fbo,
                                        MG_Diligent::GLFramebufferInfo& fbInfo);

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
                // case GL_DOUBLE: return sizeof(double); // Not supported
                // case GL_DOUBLE_VEC2: return 2 * sizeof(double); // Not supported
                // case GL_DOUBLE_VEC3: return 3 * sizeof(double); // Not supported
                // case GL_DOUBLE_VEC4: return 4 * sizeof(double); // Not supported
            default: return 0;
        }
    }

    inline size_t AlignSize(size_t size, size_t alignment) {
        return (size + alignment - 1) & ~(alignment - 1);
    }

    inline Diligent::SHADER_TYPE GetDiligentShaderType(GLenum shaderType) {
        switch (shaderType) {
            case GL_VERTEX_SHADER:
                return Diligent::SHADER_TYPE_VERTEX;
            case GL_FRAGMENT_SHADER:
                return Diligent::SHADER_TYPE_PIXEL;
            case GL_GEOMETRY_SHADER:
                return Diligent::SHADER_TYPE_GEOMETRY;
            case GL_COMPUTE_SHADER:
                return Diligent::SHADER_TYPE_COMPUTE;
            case GL_TESS_CONTROL_SHADER:
                return Diligent::SHADER_TYPE_DOMAIN;
            case GL_TESS_EVALUATION_SHADER:
                return Diligent::SHADER_TYPE_HULL;
            default:
                return Diligent::SHADER_TYPE_UNKNOWN;
        }
    }

    Diligent::SHADER_TYPE GetShaderStageForUniform(GLuint program, const std::string& name, 
                                                   Diligent::PipelineResourceLayoutDesc desc) {
        auto& programInfo = MG_Diligent::g_ProgramMap[program];
        auto& programObj = MG_State_T::programState->programs_[program];

        auto it = programInfo.uniformStages.find(name);
        if (it != programInfo.uniformStages.end()) {
            return it->second;
        }

        Diligent::SHADER_TYPE stage = Diligent::SHADER_TYPE_ALL;

        // Try to get it from the PipelineResourceLayoutDesc first.
        if (desc.DefaultVariableType != Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC) {
            for (Diligent::Uint32 i = 0; i < desc.NumVariables; ++i) {
                if (desc.Variables[i].Name == name) {
                    stage = desc.Variables[i].ShaderStages;
                    programInfo.uniformStages[name] = stage;
                    return stage;
                }
            }
        }
        
        for (Diligent::Uint32 i = 0; i < desc.NumImmutableSamplers; ++i) {
            if (desc.ImmutableSamplers[i].ShaderStages != Diligent::SHADER_TYPE_UNKNOWN &&
                strcmp(desc.ImmutableSamplers[i].SamplerOrTextureName, name.c_str()) == 0) {
                stage = desc.ImmutableSamplers[i].ShaderStages;
                programInfo.uniformStages[name] = stage;
                return stage;
            }
        }

        // The code below is useless, we should remove it.
        for (auto shader : programInfo.AttachedShadersID) {
            auto& shaderObj = MG_State_T::programState->shaders_[shader];
            if (shaderObj.compiledSpirv.empty()) continue;

            spvc_context context = nullptr;
            spvc_parsed_ir parsed_ir = nullptr;
            spvc_compiler compiler = nullptr;
            spvc_resources resources = nullptr;

            spvc_result result = spvc_context_create(&context);
            if (result != SPVC_SUCCESS) continue;

            result = spvc_context_parse_spirv(context,
                                              reinterpret_cast<const SpvId*>(shaderObj.compiledSpirv.data()),
                                              shaderObj.compiledSpirv.size() / sizeof(SpvId),
                                              &parsed_ir);

            if (result != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }

            result = spvc_context_create_compiler(context, SPVC_BACKEND_GLSL,
                                                  parsed_ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler);

            if (result != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }

            result = spvc_compiler_create_shader_resources(compiler, &resources);
            if (result != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }

            const spvc_reflected_resource* resourceList = nullptr;
            size_t resourceCount = 0;

            spvc_resources_get_resource_list_for_type(
                    resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER,
                    &resourceList, &resourceCount);

            for (size_t i = 0; i < resourceCount; i++) {
                if (strcmp(resourceList[i].name, name.c_str()) == 0) {
                    stage = GetDiligentShaderType(shaderObj.type);
                    break;
                }
            }

            spvc_resources_get_resource_list_for_type(
                    resources, SPVC_RESOURCE_TYPE_SAMPLED_IMAGE,
                    &resourceList, &resourceCount);

            for (size_t i = 0; i < resourceCount; i++) {
                if (strcmp(resourceList[i].name, name.c_str()) == 0) {
                    stage = GetDiligentShaderType(shaderObj.type);
                    break;
                }
            }

            spvc_resources_get_resource_list_for_type(
                    resources, SPVC_RESOURCE_TYPE_STORAGE_IMAGE,
                    &resourceList, &resourceCount);

            for (size_t i = 0; i < resourceCount; i++) {
                if (strcmp(resourceList[i].name, name.c_str()) == 0) {
                    stage = GetDiligentShaderType(shaderObj.type);
                    break;
                }
            }

            spvc_resources_get_resource_list_for_type(
                    resources, SPVC_RESOURCE_TYPE_SEPARATE_SAMPLERS,
                    &resourceList, &resourceCount);

            for (size_t i = 0; i < resourceCount; i++) {
                if (strcmp(resourceList[i].name, name.c_str()) == 0) {
                    stage = GetDiligentShaderType(shaderObj.type);
                    break;
                }
            }

            spvc_context_destroy(context);
        }
        if (stage == Diligent::SHADER_TYPE_ALL)
            stage = Diligent::SHADER_TYPE_PIXEL;
        programInfo.uniformStages[name] = stage;

        return stage;
    }

    void UpdateSamplerAndTextureUniforms(GLuint program, Diligent::IShaderResourceBinding& pSRB,
                                         Diligent::PipelineResourceLayoutDesc desc) {
        MG_Util::Debug::LogD("Updating sampler and texture uniforms for program %u", program);
        auto& programObj = MG_State_T::programState->programs_[program];

        for (auto& [name, uniform] : programObj.uniformValues) {
            if (!IsSamplerType(uniform.type)) continue;

            GLuint textureID = 0;
            if (!uniform.intData.empty()) {
                auto textureUnit = static_cast<GLuint>(uniform.intData[0]);
                auto unitState = &MG_State_T::textureState->textureUnits_[textureUnit];
                textureID = unitState->GetBoundTexture(unitState->activeTarget);
                MG_Util::Debug::LogD("Sampler '%s' (type: %X) uses texture ID: %u", name.c_str(), uniform.type, textureID);
            }

            Diligent::ITextureView* pTextureView = nullptr;
            if (textureID != 0) {
                auto it = MG_Diligent::g_TextureViewMap.find(textureID);
                if (it != MG_Diligent::g_TextureViewMap.end()) {
                    pTextureView = it->second;
                    MG_Util::Debug::LogD("Found ITextureView for texture ID %u.", textureID);
                } else {
                    MG_Util::Debug::LogW("ITextureView not found for texture ID %u (sampler '%s').", textureID, name.c_str());
                }
            }

            if (pTextureView) {
                Diligent::SHADER_TYPE stage = GetShaderStageForUniform(program, name, desc);
                MG_Util::Debug::LogD("Setting texture view for sampler '%s' in stage %d.", name.c_str(), stage);
                auto* pVar = pSRB.GetVariableByName(stage, name.c_str());
                if (pVar) {
                    pVar->Set(pTextureView, Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
                } else {
                    MG_Util::Debug::LogE("Failed to get shader variable for sampler '%s' in stage %d. Skipping.", name.c_str(), stage);
                }

            } else {
                MG_Util::Debug::LogW("No ITextureView available for sampler '%s' (texture ID: %u). Skipping.", name.c_str(), textureID);
            }
        }
        MG_Util::Debug::LogD("Finished updating sampler and texture uniforms for program %u.", program);
    }

    void UpdateUniformsToDefaultUBO(GLuint program, Diligent::IShaderResourceBinding& pSRB) {
        MG_Util::Debug::LogD("Updating uniforms to default UBO for program %u", program);
        auto& programInfo = MG_Diligent::g_ProgramMap[program];
        auto& programObj = MG_State_T::programState->programs_[program];

        if (programObj.uniformValues.empty() || !programInfo.pDefaultUBO) {
            return;
        }

        void * mapped;
        MG_Util::Debug::LogD("Mapping UBO for program %u", program);
        MG_Diligent::g_pContext->MapBuffer(programInfo.pDefaultUBO, Diligent::MAP_WRITE,
                                           Diligent::MAP_FLAG_DISCARD, mapped);

        if (mapped) {
            MG_Util::Debug::LogD("UBO mapped successfully for program %u. Updating uniform values.", program);
            uint8_t* uboData = static_cast<uint8_t*>(mapped);

            for (auto& name: programInfo.uniformBufferNames) {
                auto& uniform = programObj.uniformValues[name];
                if (IsSamplerType(uniform.type)) continue;

                auto it = programInfo.uniformOffsets.find(name);
                if (it != programInfo.uniformOffsets.end()) {
                    size_t offset = it->second;
                    MG_Util::Debug::LogD("Updating uniform '%s' at offset %zu for program %u", name.c_str(), offset, program);

                    size_t dataSize = 0;
                    const void* dataPtr = nullptr;

                    if (!uniform.floatData.empty()) {
                        dataSize = uniform.floatData.size() * sizeof(float);
                        dataPtr = uniform.floatData.data();
                    } else if (!uniform.intData.empty()) {
                        dataSize = uniform.intData.size() * sizeof(int);
                        dataPtr = uniform.intData.data();
                    } else if (!uniform.uintData.empty()) {
                        dataSize = uniform.uintData.size() * sizeof(uint32_t);
                        dataPtr = uniform.uintData.data();
                    }

                    if (dataPtr && dataSize > 0 && dataSize <= GetUniformSize(uniform.type)) {
                         memcpy(uboData + offset, dataPtr, dataSize);
                    } else {
                        if (!dataPtr || dataSize == 0) {
                            MG_Util::Debug::LogW("Cannot copy data for uniform '%s': data is null or size is zero.", name.c_str());
                        } else if (dataSize > GetUniformSize(uniform.type)) {
                            MG_Util::Debug::LogW("Cannot copy data for uniform '%s': provided data size (%zu) exceeds uniform size (%zu).", name.c_str(), dataSize, GetUniformSize(uniform.type));
                        }
                        // Fallback to individual assignments if memcpy is not suitable or data is malformed
                        switch (uniform.type) {
                            case GL_FLOAT:
                                if (!uniform.floatData.empty()) *reinterpret_cast<float*>(uboData + offset) = uniform.floatData[0];
                                break;
                            case GL_FLOAT_VEC2:
                            case GL_FLOAT_VEC3:
                            case GL_FLOAT_VEC4:
                            case GL_FLOAT_MAT2:
                            case GL_FLOAT_MAT3:
                            case GL_FLOAT_MAT4:
                                // memcpy handled above if data is valid
                                break;
                            case GL_INT:
                            case GL_BOOL:
                                if (!uniform.intData.empty()) *reinterpret_cast<int*>(uboData + offset) = uniform.intData[0];
                                break;
                            case GL_INT_VEC2:
                            case GL_INT_VEC3:
                            case GL_INT_VEC4:
                                // memcpy handled above if data is valid
                                break;
                            case GL_UNSIGNED_INT:
                                if (!uniform.uintData.empty()) *reinterpret_cast<uint32_t*>(uboData + offset) = uniform.uintData[0];
                                break;
                            case GL_UNSIGNED_INT_VEC2:
                            case GL_UNSIGNED_INT_VEC3:
                            case GL_UNSIGNED_INT_VEC4:
                                // memcpy handled above if data is valid
                                break;
                        }
                    }
                }
            }
            MG_Util::Debug::LogD("Finished updating uniform values in UBO for program %u.", program);
        } else {
            MG_Util::Debug::LogE("Failed to map UBO for program %u.", program);
        }

        MG_Util::Debug::LogD("Unmapping UBO for program %u", program);
        MG_Diligent::g_pContext->UnmapBuffer(programInfo.pDefaultUBO, Diligent::MAP_WRITE);
        pSRB.GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "MG_DEFAULT_UBO")->Set(programInfo.pDefaultUBO,
                                                                                    Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
        pSRB.GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "MG_DEFAULT_UBO")->Set(programInfo.pDefaultUBO,
                                                                                   Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
    }

    void EnsureRenderPassActive() {
        MG_Util::Debug::LogD("EnsureRenderPassActive called.");
        if (MG_Diligent::IsInRenderPass) {
            MG_Util::Debug::LogD("Render pass is already active.");
            return;
        }

        MG_Util::Debug::LogD("Render pass is not active, attempting to begin one.");
        GLuint drawFB = MG_State_T::framebufferState->currentBindings_[GL_DRAW_FRAMEBUFFER];
        if (drawFB == 0) {
            MG_Util::Debug::LogD("drawFB is 0, using default framebuffer (0).");
            drawFB = 0;
        }

        auto it = MG_Diligent::g_FramebufferMap.find(drawFB);
        if (it == MG_Diligent::g_FramebufferMap.end()) {
            MG_Util::Debug::LogE("Framebuffer %u not found in g_FramebufferMap.", drawFB);
            return;
        }

        MG_Util::Debug::LogD("Found framebuffer %u in g_FramebufferMap.", drawFB);
        MG_Diligent::GLFramebufferInfo& fbInfo = it->second;

        if (!fbInfo.pRenderPass || !fbInfo.pFramebuffer) {
            MG_Util::Debug::LogD("RenderPass or Framebuffer not yet created for FBO %u. Creating now.", drawFB);
            FramebufferObject* pFBO = MG_State_T::framebufferState->GetCurrentFBO(GL_DRAW_FRAMEBUFFER);
            if (!pFBO) {
                MG_Util::Debug::LogE("No current FBO found for GL_DRAW_FRAMEBUFFER when trying to create RenderPass/Framebuffer.");
                return;
            }
            CreateRenderPassAndFramebuffer(drawFB, *pFBO, fbInfo);
            if (!fbInfo.pRenderPass || !fbInfo.pFramebuffer) {
                MG_Util::Debug::LogE("Failed to create RenderPass or Framebuffer for FBO %u.", drawFB);
                return;
            }
            MG_Util::Debug::LogD("Successfully created RenderPass and Framebuffer for FBO %u.", drawFB);
        }

        MG_Util::Debug::LogD("Proceeding to begin render pass for FBO %u.", drawFB);
        if (fbInfo.pRenderPass && fbInfo.pFramebuffer) {
            Diligent::BeginRenderPassAttribs beginRenderPassAttribs;
            beginRenderPassAttribs.pFramebuffer = fbInfo.pFramebuffer;
            beginRenderPassAttribs.pRenderPass = fbInfo.pRenderPass;
            beginRenderPassAttribs.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
            beginRenderPassAttribs.ClearValueCount = fbInfo.ClearValues.size();
            beginRenderPassAttribs.pClearValues = fbInfo.ClearValues.data();

            MG_Diligent::g_pContext->BeginRenderPass(beginRenderPassAttribs);
            MG_Diligent::IsInRenderPass = true;
        } else {
            MG_Util::Debug::LogE("RenderPass or Framebuffer not yet created for FBO %u.", drawFB);
        }
    }
    
    void PrepareForDraw() {
        if (MG_Diligent::IsInRenderPass) {
            MG_Util::Debug::LogD("Ending current render pass.");
            MG_Diligent::g_pContext->EndRenderPass();
            MG_Diligent::IsInRenderPass = false;
            MG_Util::Debug::LogD("Current render pass ended.");
        } else {
            MG_Util::Debug::LogD("No active render pass to end.");
        }
        GLuint program = MG_State::GetCurrentProgram();
       
        if (program == 0) {
            MG_Util::Debug::LogE("No active program for DrawElements");
            return;
        }

        auto& programInfo = MG_Diligent::g_ProgramMap[program];

        GLuint drawFB = MG_State_T::framebufferState->currentBindings_[GL_DRAW_FRAMEBUFFER];
        if (drawFB == 0) drawFB = 0;

        auto itFB = MG_Diligent::g_FramebufferMap.find(drawFB);
        if (itFB == MG_Diligent::g_FramebufferMap.end()) {
            MG_Util::Debug::LogE("Framebuffer not found: %u", drawFB);
            return;
        }
        MG_Diligent::GLFramebufferInfo& fbInfo = itFB->second;

        MG_Util::Debug::LogD("Fetching PSO for program %u", program);
        programInfo.pPipelineState = MG_Diligent::g_PSOManager.GetOrCreatePSO(
                program,
                programInfo,
                *MG_State_T::commonState,
                *MG_State_T::vertexArrayState,
                fbInfo
        );

        if (!programInfo.pPipelineState) {
            MG_Util::Debug::LogE("Failed to get or create PSO for program %u", program);
            return;
        }
        MG_Util::Debug::LogD("Successfully obtained PSO for program %u", program);

        if (!programInfo.pResourceBinding) {
            MG_Util::Debug::LogD("Creating ShaderResourceBinding for program %u", program);
            programInfo.pPipelineState->CreateShaderResourceBinding(
                    &programInfo.pResourceBinding, true);
            if (!programInfo.pResourceBinding) {
                MG_Util::Debug::LogE("Failed to create ShaderResourceBinding for program %u", program);
                return;
            }

            MG_Util::Debug::LogD("Successfully created ShaderResourceBinding for program %u", program);
        }

        MG_Diligent::g_pContext->SetPipelineState(programInfo.pPipelineState);

        auto* pVAO = MG_State_T::vertexArrayState->GetCurrentVAO();
        if (!pVAO) {
            MG_Util::Debug::LogE("No current VAO found. A VAO must be bound before drawing.");
            return;
        }

        std::unordered_map<GLuint, Diligent::IBuffer*> createdBuffers;

        for (const auto& [attribIndex, attrib] : pVAO->attribs) {
            if (!attrib.enabled || attrib.buffer == 0) continue;

            GLuint buffer = attrib.buffer;
            auto& bufferObj = MG_State_T::bufferState->buffers_[buffer];

            Diligent::IBuffer *&pBuffer = MG_Diligent::g_BufferMap[buffer];
            if (bufferObj.isDynamic && !pBuffer) {
                Diligent::BufferDesc BuffDesc;
                std::string bufferName = "Dynamic VBO " + std::to_string(buffer);
                BuffDesc.Name = bufferName.c_str();
                BuffDesc.Size = bufferObj.data.size();
                BuffDesc.Usage = Diligent::USAGE_DYNAMIC;
                BuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
                BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

                MG_Diligent::g_pDevice->CreateBuffer(BuffDesc, nullptr, &pBuffer);

                if (!pBuffer) {
                    MG_Util::Debug::LogE("Failed to create dynamic vertex buffer %u", buffer);
                    continue;
                }
            }
        }

        if (pVAO->elementBuffer != 0) {
            GLuint buffer = pVAO->elementBuffer;
            auto& bufferObj = MG_State_T::bufferState->buffers_[buffer];

            Diligent::IBuffer*& pBuffer = MG_Diligent::g_BufferMap[buffer];
            if (bufferObj.isDynamic && !pBuffer) {
                Diligent::BufferDesc BuffDesc;
                std::string bufferName = "Dynamic IBO " + std::to_string(buffer);
                BuffDesc.Name = bufferName.c_str();
                BuffDesc.Size = bufferObj.data.size();
                BuffDesc.Usage = Diligent::USAGE_DYNAMIC;
                BuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
                BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

                if (pBuffer) {
                    pBuffer->Release();
                }

                MG_Diligent::g_pDevice->CreateBuffer(BuffDesc, nullptr, &pBuffer);

                if (!pBuffer) {
                    MG_Util::Debug::LogE("Failed to create dynamic index buffer %u", buffer);
                }
            }
        }

        // Update data for all dynamic buffers
        for (auto& [bufferID, pBuffer] : MG_Diligent::g_BufferMap) {
            auto& bufferObj = MG_State_T::bufferState->buffers_[bufferID];
            if (pBuffer && bufferObj.isDynamic && !bufferObj.data.empty()) {
                void* pMappedData = nullptr;
                MG_Util::Debug::LogD("Mapping dynamic buffer %u for data update.", bufferID);
                MG_Diligent::g_pContext->MapBuffer(
                        pBuffer,
                        Diligent::MAP_WRITE,
                        Diligent::MAP_FLAG_DISCARD,
                        pMappedData);

                if (pMappedData) {
                    memcpy(pMappedData, bufferObj.data.data(), bufferObj.data.size());
                    MG_Diligent::g_pContext->UnmapBuffer(pBuffer, Diligent::MAP_WRITE);
                    bufferObj.dirty = false;
                    MG_Util::Debug::LogD("Successfully updated data for dynamic buffer %u.", bufferID);
                    createdBuffers[bufferID] = pBuffer;
                } else {
                    MG_Util::Debug::LogE("Failed to map dynamic buffer %u for data update.", bufferID);
                }
            }
        }

        std::vector<Diligent::IBuffer*> vertexBuffers;
        std::vector<Diligent::Uint64> offsets;

        for (const auto& [attribIndex, attrib] : pVAO->attribs) {
            if (!attrib.enabled || attrib.buffer == 0) continue;

            GLuint buffer = attrib.buffer;
            auto it = MG_Diligent::g_BufferMap.find(buffer);
            if (it == MG_Diligent::g_BufferMap.end() || !it->second) continue;

            vertexBuffers.push_back(it->second);
            offsets.push_back(static_cast<Diligent::Uint64>(reinterpret_cast<size_t>(attrib.pointer)));
        }

        if (!vertexBuffers.empty()) {
            MG_Util::Debug::LogD("Setting Vertex Buffers:");
            MG_Util::Debug::LogD("  NumBuffers: %u", static_cast<Diligent::Uint32>(vertexBuffers.size()));
            for (size_t i = 0; i < vertexBuffers.size(); ++i) {
                MG_Util::Debug::LogD("  Buffer %zu:", i);
                if (vertexBuffers[i]) {
                    const auto& desc = vertexBuffers[i]->GetDesc();
                    MG_Util::Debug::LogD("    Name: %s", desc.Name ? desc.Name : "N/A");
                    MG_Util::Debug::LogD("    Size: %llu", desc.Size);
                    MG_Util::Debug::LogD("    Usage: %d", desc.Usage);
                    MG_Util::Debug::LogD("    BindFlags: %u", desc.BindFlags);
                    MG_Util::Debug::LogD("    CPUAccessFlags: %u", desc.CPUAccessFlags);
                } else {
                    MG_Util::Debug::LogD("    <null buffer>");
                }
                MG_Util::Debug::LogD("    Offset: %llu", offsets[i]);
            }

            MG_Diligent::g_pContext->SetVertexBuffers(
                    0,
                    static_cast<Diligent::Uint32>(vertexBuffers.size()),
                    vertexBuffers.data(),
                    offsets.data(),
                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                    Diligent::SET_VERTEX_BUFFERS_FLAG_NONE
            );
        }

        MG_Util::Debug::LogD("Updating uniforms for program %u", program);
        UpdateUniformsToDefaultUBO(program, *programInfo.pResourceBinding);
        UpdateSamplerAndTextureUniforms(program, *programInfo.pResourceBinding,
                                        programInfo.pPipelineState->GetDesc().ResourceLayout);

        MG_Util::Debug::LogD("Committing shader resources for program %u", program);
        MG_Diligent::g_pContext->CommitShaderResources(
                programInfo.pResourceBinding,
                Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
        );
    }

    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
        PrepareForDraw();
        auto* pVAO = MG_State_T::vertexArrayState->GetCurrentVAO();
        if (pVAO->elementBuffer != 0) {
            GLuint buffer = pVAO->elementBuffer;
            auto it = MG_Diligent::g_BufferMap.find(buffer);
            if (it != MG_Diligent::g_BufferMap.end() && it->second) {
                auto offset = reinterpret_cast<uintptr_t>(indices);

                MG_Diligent::g_pContext->SetIndexBuffer(
                        it->second,
                        offset,
                        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
                );
            }
        }
        
        Diligent::DrawIndexedAttribs drawAttrs;
        drawAttrs.NumIndices = count;
        drawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
        drawAttrs.IndexType = ConvertGLTypeToDiligent(type);
        MG_Util::Debug::LogD("DrawIndexedAttribs Dump:");
        MG_Util::Debug::LogD("  NumIndices: %u", drawAttrs.NumIndices);
        MG_Util::Debug::LogD("  IndexType: %d", drawAttrs.IndexType);
        MG_Util::Debug::LogD("  Flags: %u", drawAttrs.Flags);
        MG_Util::Debug::LogD("  NumInstances: %u", drawAttrs.NumInstances);
        MG_Util::Debug::LogD("  BaseVertex: %u", drawAttrs.BaseVertex);
        MG_Util::Debug::LogD("  FirstIndexLocation: %u", drawAttrs.FirstIndexLocation);
        MG_Util::Debug::LogD("  FirstInstanceLocation: %u", drawAttrs.FirstInstanceLocation);

        EnsureRenderPassActive();
        MG_Diligent::g_pContext->DrawIndexed(drawAttrs);
        MG_Diligent::g_pContext->EndRenderPass();
        MG_Diligent::IsInRenderPass = false;
        MG_Util::Debug::LogD("DrawIndexed completed.");
    }

    void DrawArrays(GLenum mode, GLint first, GLsizei count) {
        // TODO
    }

    void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex) {
        auto* pVAO = MG_State_T::vertexArrayState->GetCurrentVAO();
        if (pVAO->elementBuffer != 0) {
            GLuint buffer = pVAO->elementBuffer;
            auto it = MG_Diligent::g_BufferMap.find(buffer);
            if (it != MG_Diligent::g_BufferMap.end() && it->second) {
                auto offset = reinterpret_cast<uintptr_t>(indices);

                MG_Diligent::g_pContext->SetIndexBuffer(
                        it->second,
                        offset,
                        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
                );
            }
        }

        Diligent::DrawIndexedAttribs drawAttrs;
        drawAttrs.NumIndices = count;
        drawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
        drawAttrs.IndexType = ConvertGLTypeToDiligent(type);
        drawAttrs.BaseVertex = basevertex; // Apply basevertex
        MG_Util::Debug::LogD("DrawIndexedAttribs Dump (BaseVertex):");
        MG_Util::Debug::LogD("  NumIndices: %u", drawAttrs.NumIndices);
        MG_Util::Debug::LogD("  IndexType: %d", drawAttrs.IndexType);
        MG_Util::Debug::LogD("  Flags: %u", drawAttrs.Flags);
        MG_Util::Debug::LogD("  NumInstances: %u", drawAttrs.NumInstances);
        MG_Util::Debug::LogD("  BaseVertex: %u", drawAttrs.BaseVertex);
        MG_Util::Debug::LogD("  FirstIndexLocation: %u", drawAttrs.FirstIndexLocation);
        MG_Util::Debug::LogD("  FirstInstanceLocation: %u", drawAttrs.FirstInstanceLocation);

        EnsureRenderPassActive();
        MG_Diligent::g_pContext->DrawIndexed(drawAttrs);
        MG_Diligent::g_pContext->EndRenderPass();
        MG_Diligent::IsInRenderPass = false;
        MG_Util::Debug::LogD("DrawIndexed (BaseVertex) completed.");
    }

    void MultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const GLvoid *const *indices, GLsizei drawcount) {
        // TODO
    }

    void MultiDrawElementsBaseVertex(GLenum mode, const GLsizei *count, GLenum type, const GLvoid *const *indices, GLsizei drawcount, const GLint *basevertex) {
        PrepareForDraw();

        auto* pVAO = MG_State_T::vertexArrayState->GetCurrentVAO();
        Diligent::IBuffer* pIndexBuffer = nullptr;
        if (pVAO->elementBuffer != 0) {
            auto it = MG_Diligent::g_BufferMap.find(pVAO->elementBuffer);
            if (it != MG_Diligent::g_BufferMap.end() && it->second) {
                pIndexBuffer = it->second;
            }
        }

        if (!pIndexBuffer) {
            MG_Util::Debug::LogE("No valid index buffer bound for multi-draw");
            return;
        }

        std::vector<Diligent::MultiDrawIndexedItem> drawItems;
        drawItems.reserve(drawcount);

        Diligent::VALUE_TYPE indexType = ConvertGLTypeToDiligent(type);
        Diligent::Uint32 indexSize = 0;
        switch (indexType) {
            case Diligent::VT_UINT8:  indexSize = 1; break;
            case Diligent::VT_UINT16: indexSize = 2; break;
            case Diligent::VT_UINT32: indexSize = 4; break;
            default:
                MG_Util::Debug::LogE("Unsupported index type for multi-draw: %d", type);
                return;
        }

        for (GLsizei i = 0; i < drawcount; i++) {
            if (count[i] <= 0) continue;
            Diligent::MultiDrawIndexedItem item;
            item.NumIndices = static_cast<Diligent::Uint32>(count[i]);
            Diligent::Uint64 byteOffset = reinterpret_cast<Diligent::Uint64>(indices[i]);
            item.FirstIndexLocation = static_cast<Diligent::Uint32>(byteOffset / indexSize);
            item.BaseVertex = static_cast<Diligent::Int32>(basevertex[i]);
            drawItems.push_back(item);
        }

        if (drawItems.empty()) {
            MG_Util::Debug::LogW("No valid draw items in multi-draw");
            return;
        }

        Diligent::MultiDrawIndexedAttribs drawAttrs;
        drawAttrs.DrawCount = static_cast<Diligent::Uint32>(drawItems.size());
        drawAttrs.pDrawItems = drawItems.data();
        drawAttrs.IndexType = indexType;
        drawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
        drawAttrs.NumInstances = 1;
        drawAttrs.FirstInstanceLocation = 0;

        MG_Util::Debug::LogD("MultiDrawIndexedAttribs Dump:");
        MG_Util::Debug::LogD("  DrawCount: %u", drawAttrs.DrawCount);
        MG_Util::Debug::LogD("  IndexType: %d", drawAttrs.IndexType);
        MG_Util::Debug::LogD("  Flags: %u", drawAttrs.Flags);
        MG_Util::Debug::LogD("  NumInstances: %u", drawAttrs.NumInstances);
        MG_Util::Debug::LogD("  FirstInstanceLocation: %u", drawAttrs.FirstInstanceLocation);

        for (Diligent::Uint32 i = 0; i < drawAttrs.DrawCount; i++) {
            const auto& item = drawAttrs.pDrawItems[i];
            MG_Util::Debug::LogD("  DrawItem[%u]:", i);
            MG_Util::Debug::LogD("    NumIndices: %u", item.NumIndices);
            MG_Util::Debug::LogD("    FirstIndexLocation: %u", item.FirstIndexLocation);
            MG_Util::Debug::LogD("    BaseVertex: %d", item.BaseVertex);
        }

        EnsureRenderPassActive();
        MG_Diligent::g_pContext->MultiDrawIndexed(drawAttrs);
        MG_Diligent::g_pContext->EndRenderPass();
        MG_Diligent::IsInRenderPass = false;
        MG_Util::Debug::LogD("MultiDrawIndexed completed.");
    }

    void DrawBuffer(GLenum buf) {
        // TODO
    }
}