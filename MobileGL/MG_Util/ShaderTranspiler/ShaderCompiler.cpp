#include "ShaderCompiler.h"
#include <MG_Util/Converters/GLToStr/GLEnumConverter.h>
#include <MG_Util/Converters/GLToGlslang/ProgramEnumConverter.h>

namespace MobileGL {
    namespace MG_Util {
        namespace ShaderTranspiler {
            TBuiltInResource& GetTBuiltInResourceInstance() {
                static TBuiltInResource Resources{};
                Resources.maxLights = 32;
                Resources.maxClipPlanes = 6;
                Resources.maxTextureUnits = 32;
                Resources.maxTextureCoords = 32;
                Resources.maxVertexAttribs = 64;
                Resources.maxVertexUniformComponents = 4096;
                Resources.maxVaryingFloats = 64;
                Resources.maxVertexTextureImageUnits = 32;
                Resources.maxCombinedTextureImageUnits = 80;
                Resources.maxTextureImageUnits = 32;
                Resources.maxFragmentUniformComponents = 4096;
                Resources.maxDrawBuffers = 32;
                Resources.maxVertexUniformVectors = 128;
                Resources.maxVaryingVectors = 8;
                Resources.maxFragmentUniformVectors = 256;
                Resources.maxVertexOutputVectors = 16;
                Resources.maxFragmentInputVectors = 15;
                Resources.minProgramTexelOffset = -8;
                Resources.maxProgramTexelOffset = 7;
                Resources.maxClipDistances = 8;
                Resources.maxComputeWorkGroupCountX = 65535;
                Resources.maxComputeWorkGroupCountY = 65535;
                Resources.maxComputeWorkGroupCountZ = 65535;
                Resources.maxComputeWorkGroupSizeX = 1024;
                Resources.maxComputeWorkGroupSizeY = 1024;
                Resources.maxComputeWorkGroupSizeZ = 64;
                Resources.maxComputeUniformComponents = 1024;
                Resources.maxComputeTextureImageUnits = 16;
                Resources.maxComputeImageUniforms = 8;
                Resources.maxComputeAtomicCounters = 8;
                Resources.maxComputeAtomicCounterBuffers = 1;
                Resources.maxVaryingComponents = 60;
                Resources.maxVertexOutputComponents = 64;
                Resources.maxGeometryInputComponents = 64;
                Resources.maxGeometryOutputComponents = 128;
                Resources.maxFragmentInputComponents = 128;
                Resources.maxImageUnits = 8;
                Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
                Resources.maxCombinedShaderOutputResources = 8;
                Resources.maxImageSamples = 0;
                Resources.maxVertexImageUniforms = 0;
                Resources.maxTessControlImageUniforms = 0;
                Resources.maxTessEvaluationImageUniforms = 0;
                Resources.maxGeometryImageUniforms = 0;
                Resources.maxFragmentImageUniforms = 8;
                Resources.maxCombinedImageUniforms = 8;
                Resources.maxGeometryTextureImageUnits = 16;
                Resources.maxGeometryOutputVertices = 256;
                Resources.maxGeometryTotalOutputComponents = 1024;
                Resources.maxGeometryUniformComponents = 1024;
                Resources.maxGeometryVaryingComponents = 64;
                Resources.maxTessControlInputComponents = 128;
                Resources.maxTessControlOutputComponents = 128;
                Resources.maxTessControlTextureImageUnits = 16;
                Resources.maxTessControlUniformComponents = 1024;
                Resources.maxTessControlTotalOutputComponents = 4096;
                Resources.maxTessEvaluationInputComponents = 128;
                Resources.maxTessEvaluationOutputComponents = 128;
                Resources.maxTessEvaluationTextureImageUnits = 16;
                Resources.maxTessEvaluationUniformComponents = 1024;
                Resources.maxTessPatchComponents = 120;
                Resources.maxPatchVertices = 32;
                Resources.maxTessGenLevel = 64;
                Resources.maxViewports = 16;
                Resources.maxVertexAtomicCounters = 0;
                Resources.maxTessControlAtomicCounters = 0;
                Resources.maxTessEvaluationAtomicCounters = 0;
                Resources.maxGeometryAtomicCounters = 0;
                Resources.maxFragmentAtomicCounters = 8;
                Resources.maxCombinedAtomicCounters = 8;
                Resources.maxAtomicCounterBindings = 1;
                Resources.maxVertexAtomicCounterBuffers = 0;
                Resources.maxTessControlAtomicCounterBuffers = 0;
                Resources.maxTessEvaluationAtomicCounterBuffers = 0;
                Resources.maxGeometryAtomicCounterBuffers = 0;
                Resources.maxFragmentAtomicCounterBuffers = 1;
                Resources.maxCombinedAtomicCounterBuffers = 1;
                Resources.maxAtomicCounterBufferSize = 16384;
                Resources.maxTransformFeedbackBuffers = 4;
                Resources.maxTransformFeedbackInterleavedComponents = 64;
                Resources.maxCullDistances = 8;
                Resources.maxCombinedClipAndCullDistances = 8;
                Resources.maxSamples = 4;
                Resources.maxMeshOutputVerticesNV = 256;
                Resources.maxMeshOutputPrimitivesNV = 512;
                Resources.maxMeshWorkGroupSizeX_NV = 32;
                Resources.maxMeshWorkGroupSizeY_NV = 1;
                Resources.maxMeshWorkGroupSizeZ_NV = 1;
                Resources.maxTaskWorkGroupSizeX_NV = 32;
                Resources.maxTaskWorkGroupSizeY_NV = 1;
                Resources.maxTaskWorkGroupSizeZ_NV = 1;
                Resources.maxMeshViewCountNV = 4;

                Resources.limits.nonInductiveForLoops = true;
                Resources.limits.whileLoops = true;
                Resources.limits.doWhileLoops = true;
                Resources.limits.generalUniformIndexing = true;
                Resources.limits.generalAttributeMatrixVectorIndexing = true;
                Resources.limits.generalVaryingIndexing = true;
                Resources.limits.generalSamplerIndexing = true;
                Resources.limits.generalVariableIndexing = true;
                Resources.limits.generalConstantMatrixVectorIndexing = true;

                return Resources;
            }

            Result<SharedPtr<glslang::TShader>> ShaderCompiler::CompileShader(const ShaderAttrib& attrib) {
                // 输入验证
                if (attrib.sourceStr.empty()) {
                    ResultInfo r;
                    r.log += "Error: [Preprocess] Empty shader source";
                    r.errc = -100;
                    return std::unexpected(r);
                }

                auto shaderType = attrib.shaderType;
                auto& sourceStr = attrib.sourceStr;

                auto lang = MG_Util::ConvertGLEnumToEShLanguage(shaderType);
                if (lang == EShLanguage::EShLangCount) {
                    ResultInfo r;
                    r.log += "Error: [Preprocess] Unsupported shader type: " + ConvertGLEnumToString(shaderType);
                    r.errc = -1;
                    return std::unexpected(r);
                }

                SharedPtr<glslang::TShader> res;
                try {
                    res = MakeShared<glslang::TShader>(lang);
                    const char* src[] = {sourceStr.data()};
                    res->setStrings(src, 1);
                    res->setInvertY(true);
                    
                    if (attrib.flags & ShaderCompileBits::CompileForOpenGL) {
                        res->setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientOpenGL, 450);
                        res->setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
                        res->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);
                    } else {
                        res->setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientVulkan, 450);
                        res->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
                        res->setEnvTarget(glslang::EShTargetSpv,
                                          ((attrib.flags & ShaderCompileBits::EmitDiscardAsDemote)
                                               ? glslang::EShTargetSpv_1_6
                                               : glslang::EShTargetSpv_1_5));
                        res->setEnvInputVulkanRulesRelaxed();
                    }
                    
                    res->setAutoMapLocations(true);
                    res->setAutoMapBindings(true);
                    res->setGlobalUniformBlockName(GLOBAL_UBO_NAME);
                    
                    if (!res->parse(&GetTBuiltInResourceInstance(), 460, ECoreProfile,
                                    /*forceDefaultVersionAndProfile: */ false,
                                    /*forwardCompatible: */ true, EShMsgDefault)) {
                        ResultInfo r;
                        r.log += "Error: [glslang] Cannot compile " + ConvertGLEnumToString(shaderType) + ":\n" +
                                 std::string(res->getInfoLog());
                        r.errc = -2;
                        return std::unexpected(r);
                    }
                } catch (const std::exception& e) {
                    ResultInfo r;
                    r.log += "Error: [glslang] Exception during shader compilation: " + std::string(e.what());
                    r.errc = -3;
                    return std::unexpected(r);
                } catch (...) {
                    ResultInfo r;
                    r.log += "Error: [glslang] Unknown exception during shader compilation";
                    r.errc = -4;
                    return std::unexpected(r);
                }

                // 最终验证
                if (!res) {
                    ResultInfo r;
                    r.log += "Error: [glslang] Shader compilation produced null result";
                    r.errc = -5;
                    return std::unexpected(r);
                }

                return res;
            }

            Result<SharedPtr<glslang::TProgram>> ShaderCompiler::LinkProgram(const ProgramAttrib& attrib) {
                // 输入验证
                if (attrib.shaders.empty()) {
                    ResultInfo r;
                    r.log = "Error: [Link] No shaders provided for linking";
                    r.errc = -10;
                    return std::unexpected(r);
                }

                SharedPtr<glslang::TProgram> program;
                try {
                    program = MakeShared<glslang::TProgram>();
                    
                    // 添加所有着色器
                    for (auto& s : attrib.shaders) {
                        if (!s) {
                            ResultInfo r;
                            r.log = "Error: [Link] Null shader in shader list";
                            r.errc = -11;
                            return std::unexpected(r);
                        }
                        program->addShader(s.get());
                    }

                    // 链接程序
                    if (!program->link(EShMsgDefault)) {
                        ResultInfo r;
                        r.log = "Error: [glslang] Cannot link the program:\n" + std::string(program->getInfoLog());
                        r.errc = -3;
                        return std::unexpected(r);
                    }

                    // 记录显式设置的顶点输入位置
                    for (auto [name, loc]: attrib.explicitVertexInLocations) {
                        MGLOG_D("%s: got explicitly set - layout(location = %d) %s;", __func__, loc, name.c_str());
                    }

                    // 创建IO解析器
                    UniquePtr<TMglGlslIoResolver> resolver;
                    bool foundStage = false;
                    
                    for (unsigned stage = 0; stage < EShLangCount; stage++) {
                        if (program->getIntermediate((EShLanguage)stage) == nullptr)
                            continue;
                        resolver = MakeUnique<TMglGlslIoResolver>(*program, (EShLanguage)stage,
                            attrib.explicitVertexInLocations,
                            attrib.explicitFragmentOutLocations);
                        foundStage = true;
                        break;
                    }

                    if (!foundStage) {
                        ResultInfo r;
                        r.log = "Error: [glslang] No valid shader stages found in program";
                        r.errc = -12;
                        return std::unexpected(r);
                    }

                    auto ioMapper = UniquePtr<glslang::TIoMapper>(glslang::GetGlslIoMapper());

                    if (!program->mapIO(resolver.get(), ioMapper.get())) {
                        ResultInfo r;
                        r.log = "Error: [glslang] Cannot mapIO:\n" + std::string(program->getInfoLog());
                        r.errc = -4;
                        return std::unexpected(r);
                    }
                } catch (const std::exception& e) {
                    ResultInfo r;
                    r.log = "Error: [glslang] Exception during program linking: " + std::string(e.what());
                    r.errc = -13;
                    return std::unexpected(r);
                } catch (...) {
                    ResultInfo r;
                    r.log = "Error: [glslang] Unknown exception during program linking";
                    r.errc = -14;
                    return std::unexpected(r);
                }

                // 最终验证
                if (!program) {
                    ResultInfo r;
                    r.log = "Error: [glslang] Program linking produced null result";
                    r.errc = -15;
                    return std::unexpected(r);
                }

                return program;
            }

            Result<Vector<Vector<unsigned>>> ShaderCompiler::GetSpirvBinaryFromProgram(
                const ProgramBinaryAttrib& attrib) {
                // 输入验证
                if (!attrib.program) {
                    ResultInfo r;
                    r.log = "Error: [SPIRV] Null program provided";
                    r.errc = -20;
                    return std::unexpected(r);
                }

                if (attrib.shaderTypes.empty()) {
                    ResultInfo r;
                    r.log = "Error: [SPIRV] No shader types specified";
                    r.errc = -21;
                    return std::unexpected(r);
                }

                glslang::SpvOptions spvOptions;
                spvOptions.disableOptimizer = false;

                Vector<Vector<unsigned>> allSpirv;
                
                try {
                    for (auto type : attrib.shaderTypes) {
                        auto intermediate = attrib.program.getIntermediate(ConvertGLEnumToEShLanguage(type));
                        if (!intermediate) {
                            ResultInfo r;
                            r.log = "Error: [SPIRV] No intermediate representation for shader type: " + 
                                   ConvertGLEnumToString(type);
                            r.errc = -22;
                            return std::unexpected(r);
                        }

                        Vector<unsigned> spirv;
                        GlslangToSpv(*intermediate, spirv, &spvOptions);
                        
                        if (spirv.empty()) {
                            ResultInfo r;
                            r.log = "Error: [SPIRV] Empty SPIR-V generated for shader type: " + 
                                   ConvertGLEnumToString(type);
                            r.errc = -23;
                            return std::unexpected(r);
                        }
                        
                        allSpirv.push_back(std::move(spirv));
                    }
                } catch (const std::exception& e) {
                    ResultInfo r;
                    r.log = "Error: [SPIRV] Exception during SPIR-V generation: " + std::string(e.what());
                    r.errc = -24;
                    return std::unexpected(r);
                } catch (...) {
                    ResultInfo r;
                    r.log = "Error: [SPIRV] Unknown exception during SPIR-V generation";
                    r.errc = -25;
                    return std::unexpected(r);
                }

                return allSpirv;
            }

            Result<String> ShaderCompiler::DecompileShader(SpvcSession& session) {
                // 输入验证
                if (!session.IsValid()) {
                    ResultInfo r;
                    r.log = "Error: [Decompile] Invalid SPIRV-Cross session";
                    r.errc = -30;
                    return std::unexpected(r);
                }

                spvc_compiler_options options = nullptr;
                if (!session.CreateOptions(&options)) {
                    ResultInfo r;
                    r.log = "Error: [Decompile] Failed to create compiler options";
                    r.errc = -31;
                    return std::unexpected(r);
                }

                try {
                    spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_GLSL_VERSION, 320);
                    spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_GLSL_ES, SPVC_TRUE);
                    //spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_GLSL_VULKAN_SEMANTICS, SPVC_FALSE);

                    if (!session.SetOptions(options)) {
                        ResultInfo r;
                        r.log = "Error: [Decompile] Failed to set compiler options";
                        r.errc = -32;
                        return std::unexpected(r);
                    }

                    const char* result = nullptr;
                    if (!session.Compile(&result)) {
                        ResultInfo r;
                        r.log += "Failed to compile the shader to GLSL: \n";
                        r.log += session.GetLastErrorString();
                        r.errc = -5;
                        return std::unexpected(r);
                    }

                    if (!result) {
                        ResultInfo r;
                        r.log = "Error: [Decompile] Compilation returned null result";
                        r.errc = -33;
                        return std::unexpected(r);
                    }

                    std::string glsl = result;
                    return glsl;
                    
                } catch (const std::exception& e) {
                    ResultInfo r;
                    r.log = "Error: [Decompile] Exception during decompilation: " + std::string(e.what());
                    r.errc = -34;
                    return std::unexpected(r);
                } catch (...) {
                    ResultInfo r;
                    r.log = "Error: [Decompile] Unknown exception during decompilation";
                    r.errc = -35;
                    return std::unexpected(r);
                }
            }
        } // namespace ShaderTranspiler
    } // namespace MG_Util
} // namespace MobileGL
