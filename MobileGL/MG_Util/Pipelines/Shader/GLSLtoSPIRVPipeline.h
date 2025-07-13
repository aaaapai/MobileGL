//
// Created by Swung 0x48 on 2025-07-12.
//

#ifndef MG_GLSLTOSPIRVPIPELINE_H
#define MG_GLSLTOSPIRVPIPELINE_H

#include "../../../Includes.h"

namespace MobileGL {
namespace MG_Util {
namespace Pipeline {
    struct TUniform {
        String name;
        bool isSampler = false;

        // For isSampler == false
        glslang::TStorageQualifier storageQualifier;
        Uint layoutLocation = 0;
        Uint layoutBinding = 0;
        glslang::TLayoutPacking layoutPacking;

        // For isSampler == true
        glslang::TSampler sampler;
    };

    struct GLSLtoSPIRVPayload {
        GLenum shaderType;
        String sourceStr;
        UniquePtr<glslang::TShader> TShader;
        Vector<TUniform> uniforms;
        Int errc = 0;
        String log;
    };

    class UniformTraverser : public glslang::TIntermTraverser {
    public:
        UniformTraverser(Vector<TUniform>& u): uniforms(u) {}

        void visitSymbol(glslang::TIntermSymbol* symbol) override {
            const auto& type = symbol->getType();
            if (symbol->getQualifier().isUniform()) {
                auto name = symbol->getName();
                auto qualifier = symbol->getQualifier();

                auto& uniform = uniforms.emplace_back();
                uniform.isSampler = false;
                uniform.name = name;
                uniform.storageQualifier = qualifier.storage;
                uniform.layoutLocation = qualifier.layoutLocation;
                uniform.layoutBinding = qualifier.layoutBinding;
                uniform.layoutPacking = qualifier.layoutPacking;
            } else if (type.getBasicType() == glslang::EbtSampler) {
                auto& uniform = uniforms.emplace_back();
                uniform.name = symbol->getName();
                uniform.isSampler = true;
                uniform.sampler = type.getSampler();
            }
        }

        Vector<TUniform>& uniforms;
    };

    class GLSLtoSPIRVPipeline {
        explicit GLSLtoSPIRVPipeline(std::function<void(GLSLtoSPIRVPayload &)> callback);
        void Invoke(SharedPtr<GLSLtoSPIRVPayload> payload);

        inline static EShLanguage GetEShLanguageByShaderType(GLenum shaderType) {
            switch (shaderType) {
                case GL_VERTEX_SHADER:
                    return EShLanguage::EShLangVertex;
                case GL_FRAGMENT_SHADER:
                    return EShLanguage::EShLangFragment;
                case GL_COMPUTE_SHADER:
                    return EShLanguage::EShLangCompute;
                case GL_TESS_CONTROL_SHADER:
                    return EShLanguage::EShLangTessControl;
                case GL_TESS_EVALUATION_SHADER:
                    return EShLanguage::EShLangTessEvaluation;
                case GL_GEOMETRY_SHADER:
                    return EShLanguage::EShLangGeometry;
                default:
                    return EShLanguage::EShLangCount;
            }
        }

        static inline TBuiltInResource& GetTBuiltInResourceInstance() {
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
    private:
        Pipeline<GLSLtoSPIRVPayload> pipeline_;
    };
}
}
}


#endif //MG_GLSLTOSPIRVPIPELINE_H
