//
// Created by Swung 0x48 on 2025-07-12.
//

#include "GLSLtoSPIRVPipeline.h"

MobileGL::MG_Util::Pipeline::GLSLtoSPIRVPipeline::GLSLtoSPIRVPipeline(std::function<void(GLSLtoSPIRVPayload &)> callback):
    pipeline_(Move(callback)) {
    pipeline_
        .Register([this](SharedPtr<GLSLtoSPIRVPayload> payload, auto next) {
            // source -> glslang TShader
            auto& shaderType = payload->shaderType;
            auto& sourceStr = payload->sourceStr;

            glslang::InitializeProcess();
            auto lang = GetEShLanguageByShaderType(shaderType);
            if (lang == EShLanguage::EShLangCount) {
                payload->log += "Error: [Preprocess] Unsupported shader type: " +
                        ConvertGLEnumToString(shaderType);
                payload->errc = -1;
                return;
            }
            
            auto& tshader = payload->TShader;
            tshader = MakeUnique<glslang::TShader>(lang);
            const char* src[] = { sourceStr.c_str() };
            tshader->setStrings(src, 1);
            tshader->setInvertY(true);
            tshader->setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientOpenGL, 150);
            tshader->setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
            tshader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);
            tshader->setAutoMapLocations(true);
            tshader->setAutoMapBindings(true);
            tshader->setEnvInputVulkanRulesRelaxed(); // using EXT_vulkan_glsl_relaxed for gl_VertexID and gl_InstanceID?

            if (!tshader->parse(&GetTBuiltInResourceInstance(), 450, ECoreProfile,
                                 /*forceDefaultVersionAndProfile: */false,
                                 /*forwardCompatible: */true, EShMsgDefault)) {
                payload->log += "Error: [glslang] Cannot compile " + ConvertGLEnumToString(shaderType) + ":\n"
                           + std::to_string(tshader->getInfoLog());
                payload->errc = -2;
                return;
            }

            (*next)();

            glslang::FinalizeProcess();
        })
        .Register([this](auto payload, auto next) {
            UniformTraverser uniformTraverser(payload->uniforms);
            auto root = payload->TShader->getIntermediate()->getTreeRoot();
            root->traverse(&uniformTraverser);
            (*next)();
        })
        ;
}

void MobileGL::MG_Util::Pipeline::GLSLtoSPIRVPipeline::Invoke(SharedPtr<GLSLtoSPIRVPayload> payload) {
    pipeline_.Process(Move(payload));
}
