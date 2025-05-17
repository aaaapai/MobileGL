//
// Created by BZLZHH on 2025/5/3.
//

#ifndef MOBILEGL_GLSLTOOL_H
#define MOBILEGL_GLSLTOOL_H

#include "../../Includes.h"

namespace MG_Util::Program {
    inline static TBuiltInResource InitResources()
    {
        TBuiltInResource Resources{};
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

    inline int QueryGLSLVersion(const std::string& code) {
        static std::regex version_pattern(R"(#version\s+(\d{3}))");
        std::smatch match;
        if (std::regex_search(code, match, version_pattern)) {
            return std::stoi(match[1].str());
        }

        return -1;
    }

    inline GLsizei GetMatrixElementCount(GLenum matrixType) {
        switch(matrixType) {
            case GL_FLOAT_MAT2: return 4;
            case GL_FLOAT_MAT3: return 9;
            case GL_FLOAT_MAT4: return 16;
            case GL_FLOAT_MAT2x3: return 6;
            case GL_FLOAT_MAT2x4: return 8;
            case GL_FLOAT_MAT3x2: return 6;
            case GL_FLOAT_MAT3x4: return 12;
            case GL_FLOAT_MAT4x2: return 8;
            case GL_FLOAT_MAT4x3: return 12;
            default: return 0;
        }
    }

    inline std::string GetShaderTypeName(GLenum shaderType) {
        switch(shaderType) {
            case GL_VERTEX_SHADER:          return "Vertex Shader";
            case GL_FRAGMENT_SHADER:        return "Fragment Shader";
            case GL_GEOMETRY_SHADER:        return "Geometry Shader";
            case GL_TESS_CONTROL_SHADER:    return "Tessellation Control Shader";
            case GL_TESS_EVALUATION_SHADER: return "Tessellation Evaluation Shader";
            case GL_COMPUTE_SHADER:         return "Compute Shader";
            default:                        return "Unknown Shader Type (" + std::to_string(shaderType) + ")";
        }
    }
    
    inline EShLanguage GetEShLanguageByShaderType(GLenum shaderType) {
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
    
    std::vector<unsigned> CompileGLSLToSPIRV(GLenum shaderType, const std::string& source, std::string& infoLog);
    std::string CompileGLSLToTShader(GLenum shaderType, const std::string& source, glslang::TShader *&shader);
    std::vector<std::vector<unsigned>> CompileMultipleShadersToSPIRV(const ProgramState& state, ProgramObject& prog, std::string& infoLog);
    void ReflectSPIRVUniforms(const std::vector<std::vector<unsigned>>& allSpirv, ProgramObject& prog, std::string& infoLog);
    std::string CompileSPIRVToGLSL(std::vector<unsigned int> spirv, uint glslVersion, bool isES);
}

#endif //MOBILEGL_GLSLTOOL_H
