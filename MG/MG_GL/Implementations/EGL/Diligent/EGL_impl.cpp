//
// Created by Swung 0x48 on 2025-05-18.
//

#include "EGL_impl.h"

#undef MOBILEGL_GLSLTOOL_H
#undef MOBILEGL_PROGRAM_DEBUGTOOL_H
#include "../../../../Includes.h"

typedef Diligent::IEngineFactoryVk* (*Diligent_GetEngineFactoryVk_t)();
typedef Diligent::IEngineFactoryOpenGL* (*Diligent_GetEngineFactoryOpenGL_t)();

namespace MG_Diligent {
    Diligent::IEngineFactory*     g_pFactory;
    Diligent::IRenderDevice*      g_pDevice;
    Diligent::IDeviceContext*     g_pContext;
    Diligent::ISwapChain*         g_pSwapChain;
    MG_Global::unordered_map<GLuint, Diligent::IBuffer*>         g_BufferMap;
    MG_Global::unordered_map<GLuint, Diligent::ITexture*>        g_TextureMap;
    MG_Global::unordered_map<GLuint, Diligent::ITextureView*>    g_TextureViewMap;
    MG_Global::unordered_map<GLuint, GLFramebufferInfo>          g_FramebufferMap;
    MG_Global::unordered_map<GLuint, Diligent::IShader*>         g_ShaderMap;
    MG_Global::unordered_map<GLuint, GLProgramInfo>              g_ProgramMap;
    MG_Global::unordered_map<GLuint, Diligent::ISampler*>        g_SamplerMap;
    MG_Global::unordered_map<GLuint, Diligent::IBuffer*>         g_UniformBufferMap;
    Diligent::IBuffer* g_TriangleFanIndexBuffer = nullptr;
    GLuint g_NextResourceId = 0;
//    bool IsInRenderPass = false;
    bool initialized = false;

    void BuildInputLayout(GLuint program, VertexArrayState& vaState,
                          std::vector<Diligent::LayoutElement>& inputLayout) {
        inputLayout.clear();

        auto& programObj = MG_State_T::programState->programs_[program];

//        for (const auto& [attribIndex, attrib] : vaState.vaos_[vaState.currentVao_].attribs) {
        for (uint32_t attribIndex = 0; attribIndex < MG_Constants::VertexArray::MAX_VERTEX_ATTRIBS; ++attribIndex) {
            const auto& attrib = vaState.vaos_[vaState.currentVao_].attribs[attribIndex];
            if (!attrib.enabled) continue;

            std::string attribName;
            for (auto [name, loc] : programObj.attribLocations) {
                if (loc == attribIndex) {
                    attribName = name;
                    break;
                }
            }
            
            if (attribName.empty()) {
                continue; 
                // This can happen if an attribute is bound but not used by the shader program.
            }
            
            MG_State::QueryProgramAttributeLocation(program, attribName.c_str());
            if (attribIndex == -1) {
                continue;
            }

            Diligent::LayoutElement element;
            element.InputIndex = attribIndex;
            element.BufferSlot = 0;
            element.NumComponents = attrib.size;

            switch (attrib.type) {
                case GL_BYTE:           element.ValueType = Diligent::VT_INT8; break;
                case GL_UNSIGNED_BYTE:  element.ValueType = Diligent::VT_UINT8; break;
                case GL_SHORT:          element.ValueType = Diligent::VT_INT16; break;
                case GL_UNSIGNED_SHORT: element.ValueType = Diligent::VT_UINT16; break;
                case GL_INT:            element.ValueType = Diligent::VT_INT32; break;
                case GL_UNSIGNED_INT:   element.ValueType = Diligent::VT_UINT32; break;
                case GL_FLOAT:          element.ValueType = Diligent::VT_FLOAT32; break;
                case GL_HALF_FLOAT:     element.ValueType = Diligent::VT_FLOAT16; break;
                case GL_DOUBLE:         element.ValueType = Diligent::VT_FLOAT64; break;
                default:                element.ValueType = Diligent::VT_UNDEFINED; break;
            }

            element.IsNormalized = attrib.normalized;
            element.Stride = attrib.stride; // Set stride by attrib.stride
            
            element.RelativeOffset = static_cast<Diligent::Uint32>(reinterpret_cast<uintptr_t>(attrib.pointer));

            inputLayout.push_back(element);

            MG_Util::Debug::LogD("Built LayoutElement for attrib '%s' (location %d): "
                                 "NumComponents=%d, ValueType=%d, IsNormalized=%s, RelativeOffset=%u, Stride=%u",
                                 attribName.c_str(), attribIndex,
                                 element.NumComponents, element.ValueType,
                                 element.IsNormalized ? "true" : "false",
                                 element.RelativeOffset, element.Stride);
        }
    }

    namespace {
        template <typename T>
        inline void hash_combine(std::size_t& seed, const T& v) {
            std::hash<T> hasher;
            seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
    }

    uint64_t PipelineStateManager::CalculateStateHash(
            CommonState &commonState,
            VertexArrayState &vaState,
            GLFramebufferInfo& fbInfo,
            GLenum primitiveTopology) {
        size_t hash = 0;

        auto& capabilities = commonState.capabilities;

        hash_combine(hash, commonState.blendSrcRGB);
        hash_combine(hash, commonState.blendDstRGB);
        hash_combine(hash, commonState.blendSrcAlpha);
        hash_combine(hash, commonState.blendDstAlpha);
        uint32_t colorMask = (commonState.colorMask[0] ? 1 : 0) |
                             (commonState.colorMask[1] ? 2 : 0) |
                             (commonState.colorMask[2] ? 4 : 0) |
                             (commonState.colorMask[3] ? 8 : 0);
        hash_combine(hash, colorMask);
        hash_combine(hash, capabilities[GL_BLEND]);

        hash_combine(hash, commonState.depthFunc);
        hash_combine(hash, commonState.depthMask);
        hash_combine(hash, capabilities[GL_DEPTH_TEST]);

        hash_combine(hash, primitiveTopology);
        hash_combine(hash, 0); // TODO: Cull Face Mode
        hash_combine(hash, capabilities[GL_CULL_FACE]);
        hash_combine(hash, capabilities[GL_STENCIL_TEST]);

        auto *pVAO = vaState.GetCurrentVAO();

        for (uint32_t i = 0; i < MG_Constants::VertexArray::MAX_VERTEX_ATTRIBS; ++i) {
            const auto& attrib = pVAO->attribs[i];
            if (attrib.enabled) {
                hash_combine(hash, i);
                hash_combine(hash, attrib.size);
                hash_combine(hash, attrib.type);
                hash_combine(hash, attrib.normalized);
            }
        }

        hash_combine(hash, fbInfo.ColorRTVs.size());
//        if (!fbInfo.pRenderPass) {
            for (const auto& rtv : fbInfo.ColorRTVs) {
                if (rtv) {
                    hash_combine(hash, rtv->GetDesc().Format);
                }
            }
//        }
        hash_combine(hash, fbInfo.DepthStencilFormat);
        auto& programObj = MG_State_T::programState->programs_[MG_State_T::programState->currentProgram_];

        for (auto& [name, uniform] : programObj.uniformValues) {
            if (!MG_GL::GL::IsSamplerType(uniform.type)) continue;

            std::string textureIDStr;
            if (!uniform.intData.empty()) {
                auto textureUnit = static_cast<GLuint>(uniform.intData[0]);
                auto unitState = &MG_State_T::textureState->textureUnits_[textureUnit];
                textureIDStr = "tex" + std::to_string(unitState->GetBoundTexture(unitState->activeTarget));
                hash_combine(hash, textureIDStr);
                MG_Util::Debug::LogD("Hashing sampler: %s, texture: %s", name.c_str(), textureIDStr.c_str());
            }
        }
        return hash;
    }

    void PipelineStateManager::ConfigurePSO(
            Diligent::GraphicsPipelineStateCreateInfo &PSOCreateInfo,
            GLProgramInfo &programInfo,
            CommonState &commonState,
            VertexArrayState &vaState,
            GLFramebufferInfo& fbInfo,
            GLenum primitiveTopology) {
        PSOCreateInfo.PSODesc.Name = "Program_PSO";
        PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

        MG_Util::Debug::LogD("Num AttachedShaders: %zu", programInfo.AttachedShaders.size());
        for (auto shader: programInfo.AttachedShaders) {
            MG_Util::Debug::LogD("AttachedShader: %p, Type: %d", shader, shader->GetDesc().ShaderType);
            switch (shader->GetDesc().ShaderType) {
                case Diligent::SHADER_TYPE_VERTEX:
                    PSOCreateInfo.pVS = shader;
                    break;
                case Diligent::SHADER_TYPE_PIXEL:
                    PSOCreateInfo.pPS = shader;
                    break;
                case Diligent::SHADER_TYPE_GEOMETRY:
                    PSOCreateInfo.pGS = shader;
                    break;
                default:
                    break;
            }
        }
        
        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = programInfo.inputLayout.data();
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = programInfo.inputLayout.size();

//        if (fbInfo.pRenderPass) {
//            PSOCreateInfo.GraphicsPipeline.pRenderPass = fbInfo.pRenderPass;
//        } else {
//            PSOCreateInfo.GraphicsPipeline.pRenderPass = g_FramebufferMap[0].pRenderPass; // Default render pass
//        }

        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = GLPrimitiveTopologyToDiligent(primitiveTopology);

//        if (fbInfo.pRenderPass) {
//            PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 0;
//        } else
        {
            PSOCreateInfo.GraphicsPipeline.NumRenderTargets = fbInfo.ColorRTVs.size();
            if (PSOCreateInfo.GraphicsPipeline.NumRenderTargets == 0) {
                PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
                PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
            } else {
                for (size_t i = 0; i < fbInfo.ColorRTVs.size(); ++i) {
                    if (fbInfo.ColorRTVs[i]) {
                        PSOCreateInfo.GraphicsPipeline.RTVFormats[i] =
                                fbInfo.ColorRTVs[i]->GetDesc().Format;
                    } else {
                        PSOCreateInfo.GraphicsPipeline.RTVFormats[i] = Diligent::TEX_FORMAT_RGBA8_UNORM;
                    }
                }
            }
        }

        PSOCreateInfo.GraphicsPipeline.DSVFormat = fbInfo.DepthStencilFormat;

        Diligent::BlendStateDesc &blendDesc = PSOCreateInfo.GraphicsPipeline.BlendDesc;
        blendDesc.IndependentBlendEnable = false;
        blendDesc.RenderTargets[0].BlendEnable = commonState.capabilities[GL_BLEND];
        blendDesc.RenderTargets[0].SrcBlend = ConvertGLBlendFactor(commonState.blendSrcRGB);
        blendDesc.RenderTargets[0].DestBlend = ConvertGLBlendFactor(commonState.blendDstRGB);
        blendDesc.RenderTargets[0].BlendOp = Diligent::BLEND_OPERATION_ADD;
        blendDesc.RenderTargets[0].SrcBlendAlpha = ConvertGLBlendFactor(
                commonState.blendSrcAlpha);
        blendDesc.RenderTargets[0].DestBlendAlpha = ConvertGLBlendFactor(
                commonState.blendDstAlpha);
        blendDesc.RenderTargets[0].BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;
        blendDesc.RenderTargets[0].RenderTargetWriteMask = (Diligent::COLOR_MASK)(
                (commonState.colorMask[0] ? Diligent::COLOR_MASK_RED : 0) |
                (commonState.colorMask[1] ? Diligent::COLOR_MASK_GREEN : 0) |
                (commonState.colorMask[2] ? Diligent::COLOR_MASK_BLUE : 0) |
                (commonState.colorMask[3] ? Diligent::COLOR_MASK_ALPHA : 0));

        MG_Util::Debug::LogD("Configured Blend State:");
//        MG_Util::Debug::LogD("  Using explicit render pass: %s", fbInfo.pRenderPass ? "true" : "false");
        MG_Util::Debug::LogD("  NumRenderTargets: %u", PSOCreateInfo.GraphicsPipeline.NumRenderTargets);
        MG_Util::Debug::LogD("  IndependentBlendEnable: %s", blendDesc.IndependentBlendEnable ? "true" : "false");
        MG_Util::Debug::LogD("  RenderTargets[0].BlendEnable: %s", blendDesc.RenderTargets[0].BlendEnable ? "true" : "false");
        MG_Util::Debug::LogD("  RenderTargets[0].SrcBlend: %d (GL: %s)", 
                             blendDesc.RenderTargets[0].SrcBlend, 
                             MG_Util::Debug::GLEnumToString(commonState.blendSrcRGB));
        MG_Util::Debug::LogD("  RenderTargets[0].DestBlend: %d (GL: %s)", 
                             blendDesc.RenderTargets[0].DestBlend, 
                             MG_Util::Debug::GLEnumToString(commonState.blendDstRGB));
        MG_Util::Debug::LogD("  RenderTargets[0].BlendOp: %d", blendDesc.RenderTargets[0].BlendOp);
        MG_Util::Debug::LogD("  RenderTargets[0].SrcBlendAlpha: %d (GL: %s)", 
                             blendDesc.RenderTargets[0].SrcBlendAlpha, 
                             MG_Util::Debug::GLEnumToString(commonState.blendSrcAlpha));
        MG_Util::Debug::LogD("  RenderTargets[0].DestBlendAlpha: %d (GL: %s)", 
                             blendDesc.RenderTargets[0].DestBlendAlpha, 
                             MG_Util::Debug::GLEnumToString(commonState.blendDstAlpha));
        MG_Util::Debug::LogD("  RenderTargets[0].BlendOpAlpha: %d", blendDesc.RenderTargets[0].BlendOpAlpha);
        MG_Util::Debug::LogD("  RenderTargets[0].RenderTargetWriteMask: %u", blendDesc.RenderTargets[0].RenderTargetWriteMask);

        Diligent::DepthStencilStateDesc &depthStencilDesc = PSOCreateInfo.GraphicsPipeline.DepthStencilDesc;
        depthStencilDesc.DepthEnable = commonState.capabilities[GL_DEPTH_TEST];
        depthStencilDesc.DepthWriteEnable = commonState.depthMask;
        depthStencilDesc.DepthFunc = ConvertGLDepthFunc(commonState.depthFunc);

        Diligent::RasterizerStateDesc &rasterizerDesc = PSOCreateInfo.GraphicsPipeline.RasterizerDesc;
        rasterizerDesc.CullMode = commonState.capabilities[GL_CULL_FACE] ?
                                  Diligent::CULL_MODE_BACK : Diligent::CULL_MODE_NONE;
        rasterizerDesc.FrontCounterClockwise = false;
        // TODO: Get correct FrontCounterClockwise state and re-enable backface culling.

        if (!programInfo.inputLayout.empty()) {
            PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = programInfo.inputLayout.data();
            PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = 
                    static_cast<Diligent::Uint32>(programInfo.inputLayout.size());

            MG_Util::Debug::LogD("Configured InputLayout with %zu elements",
                                 programInfo.inputLayout.size());
        } else {
            MG_Util::Debug::LogW("No input layout elements for PSO");
            PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = nullptr;
            PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = 0;
        }


        ConfigureResourceLayout(PSOCreateInfo, programInfo);
    }

    void PipelineStateManager::ReleasePSO(GLuint program) {
        if (program == 0)
            return;
        auto &programInfo = g_ProgramMap[program];
        if (programInfo.pPipelineState) {
            MG_Util::Debug::LogD("Releasing PSO for program %u", program);
            programInfo.pPipelineState->Release();
            programInfo.pPipelineState = nullptr;
            MG_Util::Debug::LogD("PSO released for program %u", program);
        }
        programInfo.psoDirty = true;
    }

    void PipelineStateManager::MarkPSODirty(GLuint program) {
        if (program == 0)
            return;
        auto &programInfo = g_ProgramMap[program];
        programInfo.psoDirty = true;
        MG_Util::Debug::LogD("Marked PSO dirty for program %u", program);
    }

    Diligent::IPipelineState *PipelineStateManager::GetOrCreatePSO(
            GLuint program,
            GLProgramInfo &programInfo,
            CommonState &commonState,
            VertexArrayState &vaState,
            GLFramebufferInfo& fbInfo,
            GLenum primitiveTopology) {
        uint64_t currentStateHash = CalculateStateHash(commonState, vaState, fbInfo, primitiveTopology);

        if (programInfo.pPipelineState &&
            programInfo.psoStateHash == currentStateHash &&
            !programInfo.psoDirty) {
            MG_Util::Debug::LogD("PSO unchanged");
            return programInfo.pPipelineState;
        }

        MG_Util::Debug::LogD("commonState - Blend states: ");
        MG_Util::Debug::LogD("  SrcBlend: %s", MG_Util::Debug::GLEnumToString(commonState.blendSrcRGB));
        MG_Util::Debug::LogD("  DestBlend: %s", MG_Util::Debug::GLEnumToString(commonState.blendDstRGB));
        MG_Util::Debug::LogD("  SrcBlendAlpha: %s", MG_Util::Debug::GLEnumToString(commonState.blendSrcAlpha));
        MG_Util::Debug::LogD("  DestBlendAlpha: %s", MG_Util::Debug::GLEnumToString(commonState.blendDstAlpha));

        PSOKey key{program, currentStateHash};

        auto it = psoCache.find(key);
        if (it != psoCache.end() && it->second) {
            MG_Util::Debug::LogD("Found cached PSO: %04X%04X", it->first.programHash, it->first.stateHash);

            auto& desc = it->second->GetGraphicsPipelineDesc();
            MG_Util::Debug::LogD("PSO state - Blend states: ");
            MG_Util::Debug::LogD("  SrcBlend: %s", MG_Util::Debug::DiligentBlendFactorToString(desc.BlendDesc.RenderTargets[0].SrcBlend));
            MG_Util::Debug::LogD("  DestBlend: %s", MG_Util::Debug::DiligentBlendFactorToString(desc.BlendDesc.RenderTargets[0].DestBlend));
            MG_Util::Debug::LogD("  SrcBlendAlpha: %s", MG_Util::Debug::DiligentBlendFactorToString(desc.BlendDesc.RenderTargets[0].SrcBlendAlpha));
            MG_Util::Debug::LogD("  DestBlendAlpha: %s", MG_Util::Debug::DiligentBlendFactorToString(desc.BlendDesc.RenderTargets[0].DestBlendAlpha));
            // These asserts can fail
            assert(ConvertGLBlendFactor(commonState.blendSrcRGB) == desc.BlendDesc.RenderTargets[0].SrcBlend);
            assert(ConvertGLBlendFactor(commonState.blendDstRGB) == desc.BlendDesc.RenderTargets[0].DestBlend);
            assert(ConvertGLBlendFactor(commonState.blendSrcAlpha) == desc.BlendDesc.RenderTargets[0].SrcBlendAlpha);
            assert(ConvertGLBlendFactor(commonState.blendDstAlpha) == desc.BlendDesc.RenderTargets[0].DestBlendAlpha);

            assert(ConvertGLDepthFunc(commonState.depthFunc) == desc.DepthStencilDesc.DepthFunc);
//            assert(commonState.capabilities[GL_DEPTH_TEST] == desc.DepthStencilDesc.DepthEnable);
//            if ((ConvertGLBlendFactor(commonState.blendSrcRGB) == desc.BlendDesc.RenderTargets[0].SrcBlend) &&
//            (ConvertGLBlendFactor(commonState.blendDstRGB) == desc.BlendDesc.RenderTargets[0].DestBlend) &&
//            (ConvertGLBlendFactor(commonState.blendSrcAlpha) == desc.BlendDesc.RenderTargets[0].SrcBlendAlpha) &&
//            (ConvertGLBlendFactor(commonState.blendDstAlpha) == desc.BlendDesc.RenderTargets[0].DestBlendAlpha))
                return it->second;
        }
        
        MG_Diligent::BuildInputLayout(program, vaState, programInfo.inputLayout);
        
        Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;
        ConfigurePSO(PSOCreateInfo, programInfo, commonState, vaState, fbInfo, primitiveTopology);

        MG_Util::Debug::LogD("Dumping PSOCreateInfo for program %u:", program);
        MG_Util::Debug::LogD("  PSODesc.Name: %s", PSOCreateInfo.PSODesc.Name);
        MG_Util::Debug::LogD("  PSODesc.PipelineType: %d", PSOCreateInfo.PSODesc.PipelineType);
        MG_Util::Debug::LogD("  PSODesc.ResourceLayout.DefaultVariableType: %d", PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType);
        MG_Util::Debug::LogD("  PSODesc.ResourceLayout.NumVariables: %u", PSOCreateInfo.PSODesc.ResourceLayout.NumVariables);
        if (PSOCreateInfo.PSODesc.ResourceLayout.Variables) {
            for (Diligent::Uint32 i = 0; i < PSOCreateInfo.PSODesc.ResourceLayout.NumVariables; ++i) {
                MG_Util::Debug::LogD("    Variable %u:", i);
                MG_Util::Debug::LogD("      Name: %s", PSOCreateInfo.PSODesc.ResourceLayout.Variables[i].Name);
                MG_Util::Debug::LogD("      ShaderStages: %u", PSOCreateInfo.PSODesc.ResourceLayout.Variables[i].ShaderStages);
                MG_Util::Debug::LogD("      Type: %d", PSOCreateInfo.PSODesc.ResourceLayout.Variables[i].Type);
            }
        }
        MG_Util::Debug::LogD("  PSODesc.ResourceLayout.NumImmutableSamplers: %u", PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers);
        if (PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers) {
            for (Diligent::Uint32 i = 0; i < PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers; ++i) {
                MG_Util::Debug::LogD("    ImmutableSampler %u:", i);
                MG_Util::Debug::LogD("      ShaderStages: %u", PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers[i].ShaderStages);
                MG_Util::Debug::LogD("      SamplerOrTextureName: %s", PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers[i].SamplerOrTextureName);
                // SamplerDesc
                MG_Util::Debug::LogD("      Desc.MinFilter: %d", PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers[i].Desc.MinFilter);
                MG_Util::Debug::LogD("      Desc.MagFilter: %d", PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers[i].Desc.MagFilter);
                MG_Util::Debug::LogD("      Desc.MipFilter: %d", PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers[i].Desc.MipFilter);
                MG_Util::Debug::LogD("      Desc.AddressU: %d", PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers[i].Desc.AddressU);
                MG_Util::Debug::LogD("      Desc.AddressV: %d", PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers[i].Desc.AddressV);
                MG_Util::Debug::LogD("      Desc.AddressW: %d", PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers[i].Desc.AddressW);
            }
        }
        MG_Util::Debug::LogD("  pVS: %p (Shader Type: %d)", PSOCreateInfo.pVS, PSOCreateInfo.pVS ? PSOCreateInfo.pVS->GetDesc().ShaderType : -1);
        MG_Util::Debug::LogD("  pPS: %p (Shader Type: %d)", PSOCreateInfo.pPS, PSOCreateInfo.pPS ? PSOCreateInfo.pPS->GetDesc().ShaderType : -1);
        MG_Util::Debug::LogD("  pGS: %p (Shader Type: %d)", PSOCreateInfo.pGS, PSOCreateInfo.pGS ? PSOCreateInfo.pGS->GetDesc().ShaderType : -1);
        // GraphicsPipeline
        MG_Util::Debug::LogD("  GraphicsPipeline.InputLayout.NumElements: %u", PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements);
        if (PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements) {
            for (Diligent::Uint32 i = 0; i < PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements; ++i) {
                MG_Util::Debug::LogD("    LayoutElement %u:", i);
                MG_Util::Debug::LogD("      InputIndex: %u", PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements[i].InputIndex);
                MG_Util::Debug::LogD("      BufferSlot: %u", PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements[i].BufferSlot);
                MG_Util::Debug::LogD("      NumComponents: %u", PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements[i].NumComponents);
                MG_Util::Debug::LogD("      ValueType: %d", PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements[i].ValueType);
                MG_Util::Debug::LogD("      IsNormalized: %s", PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements[i].IsNormalized ? "true" : "false");
            }
        MG_Util::Debug::LogD("  GraphicsPipeline.pRenderPass: %p", PSOCreateInfo.GraphicsPipeline.pRenderPass);
        }
        MG_Util::Debug::LogD("  GraphicsPipeline.PrimitiveTopology: %d", PSOCreateInfo.GraphicsPipeline.PrimitiveTopology);
        MG_Util::Debug::LogD("  GraphicsPipeline.NumRenderTargets: %u", PSOCreateInfo.GraphicsPipeline.NumRenderTargets);
        for (Diligent::Uint8 i = 0; i < PSOCreateInfo.GraphicsPipeline.NumRenderTargets; ++i) {
            MG_Util::Debug::LogD("    RTVFormats[%u]: %d", i, PSOCreateInfo.GraphicsPipeline.RTVFormats[i]);
        }
        MG_Util::Debug::LogD("  GraphicsPipeline.DSVFormat: %d", PSOCreateInfo.GraphicsPipeline.DSVFormat);
        MG_Util::Debug::LogD("  GraphicsPipeline.BlendDesc.IndependentBlendEnable: %s", PSOCreateInfo.GraphicsPipeline.BlendDesc.IndependentBlendEnable ? "true" : "false");
        MG_Util::Debug::LogD("  GraphicsPipeline.DepthStencilDesc.DepthEnable: %s", PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable ? "true" : "false");
        MG_Util::Debug::LogD("  GraphicsPipeline.DepthStencilDesc.DepthFunc: 0x%x", PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthFunc);
        MG_Util::Debug::LogD("  GraphicsPipeline.RasterizerDesc.CullMode: %d", PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode);

        MG_Util::Debug::LogD("Blend states: ");
        MG_Util::Debug::LogD("  GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable: %s", PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable ? "true" : "false");
        MG_Util::Debug::LogD("  GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend: %s", MG_Util::Debug::GLEnumToString(commonState.blendSrcRGB));
        MG_Util::Debug::LogD("  GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend: %s", MG_Util::Debug::GLEnumToString(commonState.blendDstRGB));
        MG_Util::Debug::LogD("  GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlendAlpha: %s", MG_Util::Debug::GLEnumToString(commonState.blendSrcAlpha));
        MG_Util::Debug::LogD("  GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlendAlpha: %s", MG_Util::Debug::GLEnumToString(commonState.blendDstAlpha));

        Diligent::IPipelineState *pNewPSO = nullptr;
        MG_Util::Debug::LogD("Creating new PSO for program %u", program);
        g_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &pNewPSO);
        if (pNewPSO) {
            MG_Util::Debug::LogD("PSO created successfully for program %u: %p, key = %04X%04X", program, pNewPSO, key.programHash, key.stateHash);
        } else {
            MG_Util::Debug::LogE("Failed to create PSO for program %u", program);
        }

        psoCache[key] = pNewPSO;

        if constexpr (MG_Global::Common::LogLevel >= MG_Constants::Common::LOG_LEVEL_DEBUG) {
            MG_Util::Debug::LogD("PSO list = [");
            for (auto& pso: psoCache) {
                MG_Util::Debug::LogD("    %04X%04X,", pso.first.programHash, pso.first.stateHash);
            }
            MG_Util::Debug::LogD("] // PSO list end");
        }

        return pNewPSO;
    }

    void PipelineStateManager::ConfigureResourceLayout(
            Diligent::GraphicsPipelineStateCreateInfo& PSOCreateInfo,
            GLProgramInfo& programInfo)
    {
        MG_Util::Debug::LogD("Begin configuring resource layout for pipeline");

        auto& ResourceLayout = PSOCreateInfo.PSODesc.ResourceLayout;
        ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

        static std::vector<Diligent::ShaderResourceVariableDesc> Variables;
        Variables.clear();

        MG_Global::unordered_map<GLuint, std::string> shaderSourcesMap;

        ProgramObject programObj = MG_State_T::programState->programs_[programInfo.id];

        for (GLuint shaderId : programObj.attachedShaders) {
            auto it = MG_State_T::programState->shaders_.find(shaderId);
            if (it != MG_State_T::programState->shaders_.end()) {
                std::string shaderSource;
                if (it->second.type == GL_VERTEX_SHADER) {
                    std::string infoLog;
                    auto spirv =
                            MG_Util::Program::CompileGLSLToSPIRV(it->second.type,
                                                                 it->second.source,
                                                                 infoLog);
                    if (spirv.empty()) {
                        MG_Util::Debug::LogE("Shader Source for %u:\n%s", shaderId,
                                             it->second.source.c_str());
                        MG_Util::Debug::LogE("Failed to compile shader %u: %s", shaderId,
                                             infoLog.c_str());
                        continue;
                    }

                    shaderSource = MG_Util::Program::BindInputLayoutLocationsForGLSL(spirv,
                                                                                     programObj.attribLocations);
                    // MG_Util::Program::RenameGLSLBuiltinsForVulkan(shaderSource);
                } else {
                    shaderSource = it->second.source;
                }
                shaderSourcesMap[it->first] = shaderSource;
            }
        }

        MG_Util::Program::GenerateDefaultUBOForGLSL_Multi(shaderSourcesMap, programInfo.uniformBufferNames);

        for (auto shaderId : programInfo.AttachedShadersID) {
            auto shader = MG_Diligent::g_ShaderMap[shaderId];
            MG_Util::Debug::LogD("Processing shader ID: %u", shaderId);

            auto& shaderObj = MG_State_T::programState->shaders_[shaderId];
            MG_Util::Debug::LogD("Shader source for %u before SPIR-V compilation:\n%s", shaderId,
                                 shaderSourcesMap[shaderId].c_str());

            std::string compilationLog;
            auto spirv =
                    MG_Util::Program::CompileGLSLToSPIRV(shaderObj.type,
                                                         shaderSourcesMap[shaderId],
                                                         compilationLog);
            if (spirv.empty()) {
                MG_Util::Debug::LogE("Failed to compile shader %u: %s", shaderId, compilationLog.c_str());
                continue;
            } else {
                MG_Util::Debug::LogD("Shader %u (modified) compiled to SPIR-V successfully", shaderId);
            }

            spvc_context context = nullptr;
            spvc_parsed_ir parsed_ir = nullptr;
            spvc_compiler compiler = nullptr;
            spvc_resources resources = nullptr;

            spvc_result result = spvc_context_create(&context);
            if (result != SPVC_SUCCESS) {
                MG_Util::Debug::LogE("spvc_context_create failed for shader %u", shaderId);
                continue;
            }

            result = spvc_context_parse_spirv(context,
                                              spirv.data(),
                                              spirv.size(),
                                              &parsed_ir);

            if (result != SPVC_SUCCESS) {
                MG_Util::Debug::LogE("spvc_context_parse_spirv failed for shader %u", shaderId);
                spvc_context_destroy(context);
                continue;
            }

            result = spvc_context_create_compiler(context, SPVC_BACKEND_GLSL,
                                                  parsed_ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler);

            if (result != SPVC_SUCCESS) {
                MG_Util::Debug::LogE("spvc_context_create_compiler failed for shader %u", shaderId);
                spvc_context_destroy(context);
                continue;
            }

            result = spvc_compiler_create_shader_resources(compiler, &resources);
            if (result != SPVC_SUCCESS) {
                MG_Util::Debug::LogE("spvc_compiler_create_shader_resources failed for shader %u", shaderId);
                spvc_context_destroy(context);
                continue;
            }

            Diligent::SHADER_TYPE shaderType;
            switch (shaderObj.type) {
                case GL_VERTEX_SHADER:
                    shaderType = Diligent::SHADER_TYPE_VERTEX;
                    break;
                case GL_FRAGMENT_SHADER:
                    shaderType = Diligent::SHADER_TYPE_PIXEL;
                    break;
                case GL_GEOMETRY_SHADER:
                    shaderType = Diligent::SHADER_TYPE_GEOMETRY;
                    break;
                case GL_COMPUTE_SHADER:
                    shaderType = Diligent::SHADER_TYPE_COMPUTE;
                    break;
                default:
                    shaderType = Diligent::SHADER_TYPE_UNKNOWN;
            }

            MG_Util::Debug::LogD("Shader %u mapped to Diligent shader type %d", shaderId, shaderType);

            const spvc_reflected_resource* resourceList = nullptr;
            size_t resourceCount = 0;

            spvc_resources_get_resource_list_for_type(
                    resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER,
                    &resourceList, &resourceCount);
            MG_Util::Debug::LogD("Shader %u has %zu uniform buffers", shaderId, resourceCount);

            for (size_t i = 0; i < resourceCount; i++) {
                bool already_added = false;
                for(const auto& existing_var : Variables) {
                    if (strcmp(existing_var.Name, resourceList[i].name) == 0) {
                        already_added = true;
                        MG_Util::Debug::LogD("Uniform buffer %s already added, skipping.", resourceList[i].name);
                        break;
                    }
                }
                if (already_added) continue;

                Diligent::ShaderResourceVariableDesc varDesc;
                varDesc.Name = strdup(resourceList[i].name);
                varDesc.ShaderStages = shaderType;
                varDesc.Type = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;
                MG_Util::Debug::LogD("Uniform buffer: %s, Stage: %u, Type: %u", varDesc.Name, varDesc.ShaderStages, varDesc.Type);
                Variables.push_back(varDesc);
            }

            spvc_resources_get_resource_list_for_type(
                    resources, SPVC_RESOURCE_TYPE_SAMPLED_IMAGE,
                    &resourceList, &resourceCount);
            MG_Util::Debug::LogD("Shader %u has %zu sampled images", shaderId, resourceCount);

            for (size_t i = 0; i < resourceCount; i++) {
                Diligent::ShaderResourceVariableDesc varDesc;
                varDesc.Name = strdup(resourceList[i].name);
                varDesc.ShaderStages = shaderType;
                varDesc.Type = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;
                MG_Util::Debug::LogD("Sampled image: %s", varDesc.Name);
                Variables.push_back(varDesc);
            }

            spvc_resources_get_resource_list_for_type(
                    resources, SPVC_RESOURCE_TYPE_SEPARATE_SAMPLERS,
                    &resourceList, &resourceCount);
            MG_Util::Debug::LogD("Shader %u has %zu separate samplers", shaderId, resourceCount);

            for (size_t i = 0; i < resourceCount; i++) {
                Diligent::ShaderResourceVariableDesc varDesc;
                varDesc.Name = strdup(resourceList[i].name);
                varDesc.ShaderStages = shaderType;
                varDesc.Type = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;
                Variables.push_back(varDesc);
                MG_Util::Debug::LogD("Separate sampler: %s", varDesc.Name);
            }

            spvc_resources_get_resource_list_for_type(
                    resources, SPVC_RESOURCE_TYPE_STORAGE_IMAGE,
                    &resourceList, &resourceCount);
            MG_Util::Debug::LogD("Shader %u has %zu storage images", shaderId, resourceCount);

            for (size_t i = 0; i < resourceCount; i++) {
                Diligent::ShaderResourceVariableDesc varDesc;
                varDesc.Name = strdup(resourceList[i].name);
                varDesc.ShaderStages = shaderType;
                varDesc.Type = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;
                MG_Util::Debug::LogD("Storage image: %s", varDesc.Name);
                Variables.push_back(varDesc);
            }

            spvc_context_destroy(context);
        }

        if (!Variables.empty()) {
            ResourceLayout.Variables = Variables.data();
            ResourceLayout.NumVariables = static_cast<Diligent::Uint32>(Variables.size());
            MG_Util::Debug::LogD("Added %u shader resource variables", ResourceLayout.NumVariables);
        }
        
        ResourceLayout.ImmutableSamplers = nullptr;
        ResourceLayout.NumImmutableSamplers = 0;
        MG_Util::Debug::LogD("Immutable samplers disabled");

        MG_Util::Debug::LogD("Finished configuring resource layout");
    }



    PipelineStateManager g_PSOManager;
}

using namespace Diligent;

namespace MG_EGL::Diligent {
    void CreateWindowDiligentCoreOpenGL(NativeWindowType window) {
        void *handle = dlopen("libGraphicsEngineOpenGL.so", RTLD_LAZY);
        auto Diligent_GetEngineFactoryOpenGL =
                (Diligent_GetEngineFactoryOpenGL_t) dlsym(handle,
                                                          "Diligent_GetEngineFactoryOpenGL");

        auto *pFactoryOpenGL = Diligent_GetEngineFactoryOpenGL();
        ::Diligent::EngineGLCreateInfo EngineCI;
        auto *nativeWindow = static_cast<ANativeWindow *>(window);
        EngineCI.Window.pAWindow = nativeWindow;

        ::Diligent::SwapChainDesc SCDesc;

        MG_Util::Debug::LogD("Creating OpenGL device and swap chain");
        pFactoryOpenGL->CreateDeviceAndSwapChainGL(
                EngineCI, &MG_Diligent::g_pDevice, &MG_Diligent::g_pContext, SCDesc, &MG_Diligent::g_pSwapChain);
        if (MG_Diligent::g_pDevice && MG_Diligent::g_pContext && MG_Diligent::g_pSwapChain) {
            MG_Util::Debug::LogD("OpenGL device created: device=%p, context=%p, swapchain=%p", 
                                 MG_Diligent::g_pDevice, MG_Diligent::g_pContext, MG_Diligent::g_pSwapChain);
        } else {
            MG_Util::Debug::LogE("Failed to create OpenGL device");
        }
    }

    void CreateWindowDiligentCoreVulkan(NativeWindowType window) {
        AndroidNativeWindow nativeWindow{window};
        auto* pFactoryVk = static_cast<IEngineFactoryVk*>(MG_Diligent::g_pFactory);
        MG_Util::Debug::LogD("Creating Vulkan swap chain");

        ::Diligent::SwapChainDesc SCDesc;
        SCDesc.ColorBufferFormat = TEX_FORMAT_RGBA8_UNORM;
        SCDesc.PreTransform = SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180;

        pFactoryVk->CreateSwapChainVk(
                MG_Diligent::g_pDevice, MG_Diligent::g_pContext, SCDesc, nativeWindow, &MG_Diligent::g_pSwapChain);
        if (MG_Diligent::g_pSwapChain) {
            MG_Util::Debug::LogD("Vulkan swap chain created: %p", MG_Diligent::g_pSwapChain);
        } else {
            MG_Util::Debug::LogE("Failed to create Vulkan swap chain");
        }
    }

    void CreateWindowDiligentCore(NativeWindowType window) {
        switch (MG_Diligent::DILIGENT_BACKEND_TYPE) {
            case MG_Constants::Backend::BACKEND_DILIGENT_VULKAN:
                CreateWindowDiligentCoreVulkan(window);
                break;
            case MG_Constants::Backend::BACKEND_DILIGENT_OPENGL:
            default:
                CreateWindowDiligentCoreOpenGL(window);
                break;
        }
    }

    void LoadDiligentCoreVulkan() {
        void *handle = dlopen("libGraphicsEngineVk.so", RTLD_LAZY);
        auto Diligent_GetEngineFactoryVk =
                (Diligent_GetEngineFactoryVk_t) dlsym(handle,
                                                      "Diligent_GetEngineFactoryVk");

        auto *pFactoryVk = Diligent_GetEngineFactoryVk();
        MG_Diligent::g_pFactory = pFactoryVk;
        ::Diligent::EngineVkCreateInfo EngineCI;
        EngineCI.DynamicHeapSize = 512 << 20;
        EngineCI.Features.NativeMultiDraw = DEVICE_FEATURE_STATE_OPTIONAL;

        MG_Util::Debug::LogD("Creating Vulkan device and contexts");
        pFactoryVk->CreateDeviceAndContextsVk(
                EngineCI, &MG_Diligent::g_pDevice, &MG_Diligent::g_pContext);

        if (MG_Diligent::g_pDevice && MG_Diligent::g_pContext) {
            MG_Util::Debug::LogD("Vulkan device created: device=%p, context=%p",
                                 MG_Diligent::g_pDevice, MG_Diligent::g_pContext);
            auto version = MG_Diligent::g_pDevice->GetDeviceInfo().APIVersion;
            std::string apiVersion;
            switch (MG_Diligent::DILIGENT_BACKEND_TYPE) {
                case MG_Constants::Backend::BACKEND_DILIGENT_VULKAN:
                    apiVersion = std::format("Vulkan {}.{}", version.Major, version.Minor);
                    break;
                case MG_Constants::Backend::BACKEND_DILIGENT_METAL:
                    apiVersion = std::format("Metal {}.{}", version.Major, version.Minor);
                    break;
                case MG_Constants::Backend::BACKEND_DILIGENT_OPENGL:
                    apiVersion = std::format("OpenGL {}.{}", version.Major, version.Minor);
                    break;
                default:
                    apiVersion = "<unknown>";
            }
            MG_Util::Debug::LogI("Running on %s, %s", MG_Diligent::g_pDevice->GetAdapterInfo().Description, apiVersion.c_str());
            MG_Util::Debug::LogI("NativeMultiDraw is%s supported.", (MG_Diligent::g_pDevice->GetDeviceInfo().Features.NativeMultiDraw != DEVICE_FEATURE_STATE_DISABLED) ? " " : " not");
        } else {
            MG_Util::Debug::LogE("Failed to create Vulkan device");
        }

    }

    void LoadDiligentCore() {
        switch (MG_Diligent::DILIGENT_BACKEND_TYPE) {
            case MG_Constants::Backend::BACKEND_DILIGENT_VULKAN:
                LoadDiligentCoreVulkan();
                break;
            case MG_Constants::Backend::BACKEND_DILIGENT_OPENGL:
            default:
//                CreateWindowDiligentCoreOpenGL(window);
                break;
        }
    }

    void CreateDefaultRenderPass() {
        ::Diligent::RenderPassDesc RPDesc;
        RPDesc.AttachmentCount = 2;
        ::Diligent::RenderPassAttachmentDesc Attachments[2];
        RPDesc.pAttachments = Attachments;

        Attachments[0].Format = ::Diligent::TEX_FORMAT_RGBA8_UNORM;
        Attachments[0].InitialState = ::Diligent::RESOURCE_STATE_RENDER_TARGET;
        Attachments[0].FinalState = ::Diligent::RESOURCE_STATE_RENDER_TARGET;
        Attachments[0].LoadOp = ::Diligent::ATTACHMENT_LOAD_OP_CLEAR;
        Attachments[0].StoreOp = ::Diligent::ATTACHMENT_STORE_OP_STORE;

        Attachments[1].Format = ::Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
        Attachments[1].InitialState = ::Diligent::RESOURCE_STATE_DEPTH_WRITE;
        Attachments[1].FinalState = ::Diligent::RESOURCE_STATE_DEPTH_WRITE;
        Attachments[1].LoadOp = ::Diligent::ATTACHMENT_LOAD_OP_CLEAR;
        Attachments[1].StoreOp = ::Diligent::ATTACHMENT_STORE_OP_STORE;

        ::Diligent::AttachmentReference RTAttachmentRef{0, ::Diligent::RESOURCE_STATE_RENDER_TARGET};
        ::Diligent::AttachmentReference DSAttachmentRef{1, ::Diligent::RESOURCE_STATE_DEPTH_WRITE};

        ::Diligent::SubpassDesc Subpasses[1];
        Subpasses[0].RenderTargetAttachmentCount = 1;
        Subpasses[0].pRenderTargetAttachments = &RTAttachmentRef;
        Subpasses[0].pDepthStencilAttachment  = &DSAttachmentRef;

        RPDesc.SubpassCount = 1;
        RPDesc.pSubpasses = Subpasses;

//        MG_Util::Debug::LogD("Creating default render pass");
//        MG_Diligent::g_pDevice->CreateRenderPass(RPDesc, &MG_Diligent::g_FramebufferMap[0].pRenderPass);
//        if (MG_Diligent::g_FramebufferMap[0].pRenderPass) {
//            MG_Util::Debug::LogD("Default render pass created: %p", MG_Diligent::g_FramebufferMap[0].pRenderPass);
//        } else {
//            MG_Util::Debug::LogE("Failed to create default render pass");
//        }
    }

    EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, NativeWindowType window,
                                      const EGLint *attrib_list) {
        CreateWindowDiligentCore(window);
        MG_GL::GL::UpdateDefaultFramebuffer();
        return (EGLSurface) 1;
    }

    EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs,
                               EGLint config_size, EGLint *num_config) {
        *num_config = 1;
        return EGL_TRUE;
    }

    EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext shareCtx,
                                const EGLint *attrib_list) {
        return (EGLContext) 1;
    }

    EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor) {
        LoadDiligentCore();
        if (major) *major = 1;
        if (minor) *minor = 5;
        return EGL_TRUE;
    }

    EGLDisplay eglGetDisplay(NativeDisplayType display) {
        return (EGLDisplay) 1;
    }

    EGLint eglGetError() {
        return EGL_SUCCESS;
    }

    EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
        return EGL_TRUE;
    }

    EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx) {
        return EGL_TRUE;
    }

    EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface) {
        return EGL_TRUE;
    }

    EGLBoolean eglTerminate(EGLDisplay dpy) {
        return EGL_TRUE;
    }

    EGLBoolean eglReleaseThread(void) {
        return EGL_TRUE;
    }

    EGLContext eglGetCurrentContext(void) {
        return (EGLContext) 1;
    }

    EGLBoolean
    eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value) {
        if (attribute == EGL_NATIVE_VISUAL_ID)
            *value = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
        return EGL_TRUE;
    }

    EGLBoolean eglBindAPI(EGLenum api) {
        return EGL_TRUE;
    }

    EGLSurface eglGetCurrentSurface(EGLint readdraw) {
        return (EGLSurface) 1;
    }

    EGLBoolean
    eglQuerySurface(EGLDisplay display, EGLSurface surface, EGLint attribute, EGLint *value) {
        return EGL_TRUE;
    }

    EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval) {
        return EGL_TRUE;
    }

    EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface draw) {
        MG_Util::Debug::LogD("Flushing context");
//        if (MG_Diligent::IsInRenderPass) {
//            MG_Util::Debug::LogD("Ending current render pass.");
//            MG_Diligent::g_pContext->EndRenderPass();
//            MG_Diligent::IsInRenderPass = false;
//            MG_Util::Debug::LogD("Current render pass ended.");
//        } else {
//            MG_Util::Debug::LogD("No active render pass to end.");
//        }
        MG_Diligent::g_pContext->Flush();

        MG_Util::Debug::LogD("Presenting swap chain");
        MG_Diligent::g_pSwapChain->Present();
        MG_GL::GL::UpdateDefaultFramebuffer();
//        if (MG_Diligent::IsInRenderPass) {
//            MG_Util::Debug::LogD("Ending current render pass.");
//            MG_Diligent::g_pContext->EndRenderPass();
//            MG_Diligent::IsInRenderPass = false;
//            MG_Util::Debug::LogD("Current render pass ended.");
//        } else {
//            MG_Util::Debug::LogD("No active render pass to end.");
//        }
        MG_Diligent::g_pContext->Flush();
    
        MG_Util::Debug::LogD("Finishing frame");
        MG_Diligent::g_pContext->FinishFrame();

        auto& fbInfo = MG_Diligent::g_FramebufferMap[0];
//        if (/*fbInfo.pRenderPass &&*/ fbInfo.pFramebuffer) {
            MG_Util::Debug::LogD("Setting render targets: %zu color attachments", fbInfo.ColorRTVs.size());
            MG_Diligent::g_pContext->SetRenderTargets(
                    static_cast<Uint32>(fbInfo.ColorRTVs.size()),
                    fbInfo.ColorRTVs.data(),
                    fbInfo.pDepthStencilRTV,
                    ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
            );

            static ::Diligent::OptimizedClearValue DefaultClearValues[2];
            DefaultClearValues[0].Format = fbInfo.ColorRTVs[0]->GetDesc().Format;
            DefaultClearValues[0].Color[0] = 0.0f;
            DefaultClearValues[0].Color[1] = 0.0f;
            DefaultClearValues[0].Color[2] = 0.0f;
            DefaultClearValues[0].Color[3] = 1.0f;
            DefaultClearValues[1].Format = fbInfo.DepthStencilFormat;
            DefaultClearValues[1].DepthStencil.Depth = 1.0f;
            DefaultClearValues[1].DepthStencil.Stencil = 0;

//            MG_Util::Debug::LogD("Beginning render pass: pass=%p, fb=%p", fbInfo.pRenderPass, fbInfo.pFramebuffer);
//            MG_Diligent::g_pContext->BeginRenderPass(
//                    ::Diligent::BeginRenderPassAttribs{ fbInfo.pRenderPass, fbInfo.pFramebuffer,
//                                                        2, DefaultClearValues,
//                                                        ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION}
//                                                        );
//            MG_Diligent::IsInRenderPass = true;
//        }
//        const auto& SCDesc = MG_Diligent::g_pSwapChain->GetDesc();
//        MG_Util::Debug::LogD("Setting viewport: width=%d, height=%d", SCDesc.Width, SCDesc.Height);

//        ::Diligent::Viewport viewport;
//        viewport.TopLeftX = 0.f;
//        viewport.TopLeftY = SCDesc.Height;
//        viewport.Width = SCDesc.Width;
//        viewport.Height = SCDesc.Height;
//        viewport.MinDepth = 0.f;
//        viewport.MaxDepth = 1.f;
        MG_Diligent::g_pContext->SetViewports(1, nullptr, 0, 0);
        return EGL_TRUE;
    }

    EGLSurface
    eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) {
        return (EGLSurface) 1;
    }

}

void *libGLES = nullptr, *libEGL = nullptr;

static const char *LibPathPrefixes[] = {
        "",
        "/opt/vc/lib/",
        "/usr/local/lib/",
        "/usr/lib/",
        nullptr
};

static const char *LibExts[] = {
        "so",
        "so.1",
        "so.2",
        "dylib",
        "dll",
        nullptr
};

static const char *GLES3Libs[] = {
        "libGLESv3_CM",
        "libGLESv3",
        nullptr
};

static const char *EGLLibs[] = {
        "libEGL",
        nullptr
};

void *OpenLib(const char **names, const char *override) {
    void *lib = nullptr;

    char path_name[PATH_MAX + 1];
    int flags = RTLD_LOCAL | RTLD_NOW;
    if (override) {
        if ((lib = dlopen(override, flags))) {
            strncpy(path_name, override, PATH_MAX);
            return lib;
        } else {
            MG_Util::Debug::LogE("LIBGL_GLES override failed: %s\n", dlerror());
        }
    }
    for (int p = 0; LibPathPrefixes[p]; p++) {
        for (int i = 0; names[i]; i++) {
            for (int e = 0; LibExts[e]; e++) {
                snprintf(path_name, PATH_MAX, "%s%s.%s", LibPathPrefixes[p], names[i], LibExts[e]);
                if ((lib = dlopen(path_name, flags))) {
                    return lib;
                }
            }
        }
    }
    return lib;
}

typedef __eglMustCastToProperFunctionPointerType (* eglGetProcAddress_t)(const char *procname);
eglGetProcAddress_t real_eglGetProcAddress = nullptr;

__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char *procname) {
    if (strncmp(procname, "eglGetProcAddress", strlen("eglGetProcAddress")) == 0) {
        return (__eglMustCastToProperFunctionPointerType)(&eglGetProcAddress);
    }

#define MG_EGL_PROC_EXPORT(name) \
    if (strncmp(procname, #name, strlen(#name)) == 0) { \
        return (__eglMustCastToProperFunctionPointerType)(&MG_EGL::Diligent::name); \
    }

    MG_EGL_PROC_EXPORT(eglCreateWindowSurface);
    MG_EGL_PROC_EXPORT(eglChooseConfig);
    MG_EGL_PROC_EXPORT(eglCreateContext);
    MG_EGL_PROC_EXPORT(eglInitialize);
    MG_EGL_PROC_EXPORT(eglGetDisplay);
    MG_EGL_PROC_EXPORT(eglGetError);
    MG_EGL_PROC_EXPORT(eglMakeCurrent);
    MG_EGL_PROC_EXPORT(eglDestroyContext);
    MG_EGL_PROC_EXPORT(eglDestroySurface);
    MG_EGL_PROC_EXPORT(eglTerminate);
    MG_EGL_PROC_EXPORT(eglReleaseThread);
    MG_EGL_PROC_EXPORT(eglGetCurrentContext);
    MG_EGL_PROC_EXPORT(eglGetConfigAttrib);
    MG_EGL_PROC_EXPORT(eglBindAPI);
    MG_EGL_PROC_EXPORT(eglGetCurrentSurface);
    MG_EGL_PROC_EXPORT(eglQuerySurface);
    MG_EGL_PROC_EXPORT(eglSwapInterval);
    MG_EGL_PROC_EXPORT(eglSwapBuffers);
    MG_EGL_PROC_EXPORT(eglCreatePbufferSurface);
#undef MG_EGL_PROC_EXPORT

    /* TODO: Call real eglGetProcAddress() to get the rest (should be real native GLES functions) */
    if (!libEGL) {
        libEGL = OpenLib(EGLLibs, nullptr);
        real_eglGetProcAddress = (eglGetProcAddress_t)dlsym(libEGL, "eglGetProcAddress");
    }
    if (real_eglGetProcAddress)
        return real_eglGetProcAddress(procname);

    return nullptr;
}