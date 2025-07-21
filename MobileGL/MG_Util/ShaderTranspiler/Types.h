//
// Created by Swung 0x48 on 2025/7/20.
//

#ifndef MG_UTIL_SHADERTRANSPILER_TYPES_H
#define MG_UTIL_SHADERTRANSPILER_TYPES_H

namespace MobileGL {
    namespace MG_Util {
        namespace ShaderTranspiler {
            struct EmptyType {};

            struct ShaderAttrib {
                GLenum shaderType;
                String sourceStr;
            };

            struct ProgramAttrib {
                Vector<GLenum> shaderTypes;
                Vector<SharedPtr<glslang::TShader>> shaders;
            };

            struct ResultInfo {
                Int errc = 0;
                String log;
            };

            template <typename T>
            using Result = std::expected<T, ResultInfo>;

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
