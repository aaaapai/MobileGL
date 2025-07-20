//
// Created by Swung 0x48 on 2025/7/20.
//

#ifndef MG_UTIL_SHADERTRANSPILER_TYPES_H
#define MG_UTIL_SHADERTRANSPILER_TYPES_H

namespace MobileGL {
    namespace MG_Util {
        namespace ShaderTranspiler {
            enum class TUniformType {
                Uniform,
                Sampler
            };

            template <TUniformType T>
            struct TUniform {
                static_assert(false, "TUniform<T> does not accept this enum");
            };

            template <>
            struct TUniform<TUniformType::Uniform> {
                String name;

                glslang::TStorageQualifier storageQualifier;
                Uint layoutLocation = 0;
                Uint layoutBinding = 0;
                glslang::TLayoutPacking layoutPacking;
            };

            template <>
            struct TUniform<TUniformType::Sampler> {
                String name;

                glslang::TSampler sampler;
            };

            struct EmptyType {};

            struct ShaderAttrib {
                GLenum shaderType;
                String sourceStr;
            };

            struct CompiledTShader {
                UniquePtr<glslang::TShader> TShader;
                Vector<TUniform<TUniformType::Uniform>> uniforms;
                Vector<TUniform<TUniformType::Sampler>> samplers;
            };

            template <bool Succeeded>
            struct ShaderCompileResult:
                public std::conditional_t<Succeeded, CompiledTShader, EmptyType> {
                Int errc = 0;
                String log;
            };

            using CompilerResult = std::expected<ShaderCompileResult<true>, ShaderCompileResult<false>>;

            struct ShaderPayload {
                // In
                GLenum shaderType;
                String sourceStr;

                // Out
                UniquePtr<glslang::TShader> TShader;
                Vector<TUniform<TUniformType::Uniform>> uniforms;
                Vector<TUniform<TUniformType::Sampler>> samplers;
                Int errc = 0;
                String log;
            };

            struct ProgramPayload {
                // In
                Vector<GLenum> shaderTypes;
                Vector<UniquePtr<glslang::TShader>> shadersToLink;

                // Out
                glslang::TProgram linkedProgram;
                Vector<Vector<Uint>> programSpirv;
                Int errc = 0;
                String log;
            };

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
        }
    }
}

#include "ShaderCompiler.h"

#endif //MG_UTIL_SHADERTRANSPILER_TYPES_H
