//
// Created by Swung 0x48 on 2025/7/20.
//

#include "Includes.h"

namespace MobileGL {
    namespace MG_Util {
        namespace ShaderTranspiler {
            TBuiltInResource& GetTBuiltInResourceInstance()
            {
                static TBuiltInResource Resources{};
                Resources.maxLights                                 = 32;
                Resources.maxClipPlanes                             = 6;
                Resources.maxTextureUnits                           = 32;
                Resources.maxTextureCoords                          = 32;
                Resources.maxVertexAttribs                          = 64;
                Resources.maxVertexUniformComponents                = 4096;
                Resources.maxVaryingFloats                          = 64;
                Resources.maxVertexTextureImageUnits                = 32;
                Resources.maxCombinedTextureImageUnits              = 80;
                Resources.maxTextureImageUnits                      = 32;
                Resources.maxFragmentUniformComponents              = 4096;
                Resources.maxDrawBuffers                            = 32;
                Resources.maxVertexUniformVectors                   = 128;
                Resources.maxVaryingVectors                         = 8;
                Resources.maxFragmentUniformVectors                 = 16;
                Resources.maxVertexOutputVectors                    = 16;
                Resources.maxFragmentInputVectors                   = 15;
                Resources.minProgramTexelOffset                     = -8;
                Resources.maxProgramTexelOffset                     = 7;
                Resources.maxClipDistances                          = 8;
                Resources.maxComputeWorkGroupCountX                 = 65535;
                Resources.maxComputeWorkGroupCountY                 = 65535;
                Resources.maxComputeWorkGroupCountZ                 = 65535;
                Resources.maxComputeWorkGroupSizeX                  = 1024;
                Resources.maxComputeWorkGroupSizeY                  = 1024;
                Resources.maxComputeWorkGroupSizeZ                  = 64;
                Resources.maxComputeUniformComponents               = 1024;
                Resources.maxComputeTextureImageUnits               = 16;
                Resources.maxComputeImageUniforms                   = 8;
                Resources.maxComputeAtomicCounters                  = 8;
                Resources.maxComputeAtomicCounterBuffers            = 1;
                Resources.maxVaryingComponents                      = 60;
                Resources.maxVertexOutputComponents                 = 64;
                Resources.maxGeometryInputComponents                = 64;
                Resources.maxGeometryOutputComponents               = 128;
                Resources.maxFragmentInputComponents                = 128;
                Resources.maxImageUnits                             = 8;
                Resources.maxCombinedImageUnitsAndFragmentOutputs   = 8;
                Resources.maxCombinedShaderOutputResources          = 8;
                Resources.maxImageSamples                           = 0;
                Resources.maxVertexImageUniforms                    = 0;
                Resources.maxTessControlImageUniforms               = 0;
                Resources.maxTessEvaluationImageUniforms            = 0;
                Resources.maxGeometryImageUniforms                  = 0;
                Resources.maxFragmentImageUniforms                  = 8;
                Resources.maxCombinedImageUniforms                  = 8;
                Resources.maxGeometryTextureImageUnits              = 16;
                Resources.maxGeometryOutputVertices                 = 256;
                Resources.maxGeometryTotalOutputComponents          = 1024;
                Resources.maxGeometryUniformComponents              = 1024;
                Resources.maxGeometryVaryingComponents              = 64;
                Resources.maxTessControlInputComponents             = 128;
                Resources.maxTessControlOutputComponents            = 128;
                Resources.maxTessControlTextureImageUnits           = 16;
                Resources.maxTessControlUniformComponents           = 1024;
                Resources.maxTessControlTotalOutputComponents       = 4096;
                Resources.maxTessEvaluationInputComponents          = 128;
                Resources.maxTessEvaluationOutputComponents         = 128;
                Resources.maxTessEvaluationTextureImageUnits        = 16;
                Resources.maxTessEvaluationUniformComponents        = 1024;
                Resources.maxTessPatchComponents                    = 120;
                Resources.maxPatchVertices                          = 32;
                Resources.maxTessGenLevel                           = 64;
                Resources.maxViewports                              = 16;
                Resources.maxVertexAtomicCounters                   = 0;
                Resources.maxTessControlAtomicCounters              = 0;
                Resources.maxTessEvaluationAtomicCounters           = 0;
                Resources.maxGeometryAtomicCounters                 = 0;
                Resources.maxFragmentAtomicCounters                 = 8;
                Resources.maxCombinedAtomicCounters                 = 8;
                Resources.maxAtomicCounterBindings                  = 1;
                Resources.maxVertexAtomicCounterBuffers             = 0;
                Resources.maxTessControlAtomicCounterBuffers        = 0;
                Resources.maxTessEvaluationAtomicCounterBuffers     = 0;
                Resources.maxGeometryAtomicCounterBuffers           = 0;
                Resources.maxFragmentAtomicCounterBuffers           = 1;
                Resources.maxCombinedAtomicCounterBuffers           = 1;
                Resources.maxAtomicCounterBufferSize                = 16384;
                Resources.maxTransformFeedbackBuffers               = 4;
                Resources.maxTransformFeedbackInterleavedComponents = 64;
                Resources.maxCullDistances                          = 8;
                Resources.maxCombinedClipAndCullDistances           = 8;
                Resources.maxSamples                                = 4;
                Resources.maxMeshOutputVerticesNV                   = 256;
                Resources.maxMeshOutputPrimitivesNV                 = 512;
                Resources.maxMeshWorkGroupSizeX_NV                  = 32;
                Resources.maxMeshWorkGroupSizeY_NV                  = 1;
                Resources.maxMeshWorkGroupSizeZ_NV                  = 1;
                Resources.maxTaskWorkGroupSizeX_NV                  = 32;
                Resources.maxTaskWorkGroupSizeY_NV                  = 1;
                Resources.maxTaskWorkGroupSizeZ_NV                  = 1;
                Resources.maxMeshViewCountNV                        = 4;

                Resources.limits.nonInductiveForLoops                 = true;
                Resources.limits.whileLoops                           = true;
                Resources.limits.doWhileLoops                         = true;
                Resources.limits.generalUniformIndexing               = true;
                Resources.limits.generalAttributeMatrixVectorIndexing = true;
                Resources.limits.generalVaryingIndexing               = true;
                Resources.limits.generalSamplerIndexing               = true;
                Resources.limits.generalVariableIndexing              = true;
                Resources.limits.generalConstantMatrixVectorIndexing  = true;

                return Resources;
            }

            Result<SharedPtr<glslang::TShader>> ShaderCompiler::CompileShader(const ShaderAttrib& attrib) {
                auto shaderType = attrib.shaderType;
                auto sourceStr = attrib.sourceStr;

                auto lang = GetEShLanguageByShaderType(shaderType);
                if (lang == EShLanguage::EShLangCount) {
                    ResultInfo r;
                    r.log += "Error: [Preprocess] Unsupported shader type: " +
                                    ConvertGLEnumToString(shaderType);
                    r.errc = -1;
                    return std::unexpected(r);
                }

                SharedPtr<glslang::TShader> res;
                auto& tshader = res;
                tshader = MakeShared<glslang::TShader>(lang);
                const char* src[] = { sourceStr.c_str() };
                tshader->setStrings(src, 1);
                tshader->setInvertY(true);
                tshader->setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientVulkan, 450);
                tshader->setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
                tshader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);
                tshader->setAutoMapLocations(true);
                tshader->setAutoMapBindings(true);
                tshader->setEnvInputVulkanRulesRelaxed(); // using EXT_vulkan_glsl_relaxed for gl_VertexID and gl_InstanceID?
                if (!tshader->parse(&GetTBuiltInResourceInstance(), 150, ECompatibilityProfile,
                        /*forceDefaultVersionAndProfile: */false,
                        /*forwardCompatible: */true, EShMsgDefault)) {
                    ResultInfo r;
                    r.log += "Error: [glslang] Cannot compile " + ConvertGLEnumToString(shaderType) + ":\n"
                                    + std::string(tshader->getInfoLog());
                    r.errc = -2;
                    return std::unexpected(r);
                }

                return res;
            }

            Result<Vector<Vector<unsigned>>> ShaderCompiler::LinkProgram(const ProgramAttrib& attrib) {
                glslang::TProgram program;
                for (auto& s : attrib.shaders) {
                    program.addShader(s.get());
                }

                if (!program.link(EShMsgDefault)) {
                    ResultInfo r;
                    r.log = "Error: [glslang] Cannot link the program:\n" + std::string(program.getInfoLog());
                    r.errc = -3;
                    return std::unexpected(r);
                }

                UniquePtr<glslang::TIoMapResolver> resolver;
                for (unsigned stage = 0; stage < EShLangCount; stage++) {
                    auto* pResolver = program.getGlslIoResolver((EShLanguage)stage);
                    if (pResolver) {
                        resolver = UniquePtr<glslang::TIoMapResolver>(pResolver);
                        break;
                    }
                }
                auto ioMapper = UniquePtr<glslang::TIoMapper>(glslang::GetGlslIoMapper());

                if (!program.mapIO(resolver.get(), ioMapper.get())) {
                    ResultInfo r;
                    r.log = "Error: [glslang] Cannot mapIO:\n" + std::string(program.getInfoLog());
                    r.errc = -4;
                    return std::unexpected(r);
                }

                glslang::SpvOptions spvOptions;
                spvOptions.disableOptimizer = false;

                Vector<Vector<unsigned>> allSpirv;
                for (auto type : attrib.shaderTypes) {
                    Vector<unsigned> spirv;
                    GlslangToSpv(*program.getIntermediate(ConvertGLEnumToEShLanguage(type)), spirv, &spvOptions);
                    allSpirv.push_back(spirv);
                }

                return allSpirv;
            }

            Result<String> ShaderCompiler::DecompileShader(Vector<unsigned int> spirv) {
                spvc_context context = nullptr;
                spvc_parsed_ir ir = nullptr;
                spvc_compiler compiler_glsl = nullptr;
                spvc_compiler_options options = nullptr;
                spvc_resources resources = nullptr;
                const spvc_reflected_resource *list = nullptr;
                const char *result = nullptr;
                size_t count;

                const SpvId *p_spirv = spirv.data();
                size_t word_count = spirv.size();

                spvc_context_create(&context);
                spvc_context_parse_spirv(context, p_spirv, word_count, &ir);
                spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler_glsl);
                spvc_compiler_create_shader_resources(compiler_glsl, &resources);
                spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, &list, &count);
                spvc_compiler_create_compiler_options(compiler_glsl, &options);

                spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_GLSL_VERSION, 450);
                spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_GLSL_ES, SPVC_FALSE);
                spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_GLSL_VULKAN_SEMANTICS, SPVC_TRUE);
                spvc_compiler_install_compiler_options(compiler_glsl, options);
                spvc_compiler_compile(compiler_glsl, &result);

                if (!result) {
                    ResultInfo r;
                    r.log += "Failed to compile the shader to GLSL: \n";
                    r.log += spvc_context_get_last_error_string(context);
                    r.errc = -5;
                    return std::unexpected(r);
                }

                std::string glsl = result;

                spvc_context_destroy(context);

                return glsl;
            }
        }
    }
}
