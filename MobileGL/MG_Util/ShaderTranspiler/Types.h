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

            class SpvcSession {
            public:
                SpvcSession() {}

                explicit SpvcSession(Vector<unsigned int> spirv) {
                    const SpvId *p_spirv = spirv.data();
                    size_t word_count = spirv.size();

                    spvc_context_create(&context);
                    spvc_context_parse_spirv(context, p_spirv, word_count, &ir);
                    spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler);
                }

                SpvcSession(SpvcSession&) = delete;

                SpvcSession(SpvcSession&& that) {
                    std::swap(this->context, that.context);
                    std::swap(this->compiler, that.compiler);
                    std::swap(this->ir, that.ir);
                    std::swap(this->compiler_options, that.compiler_options);
                }

                SpvcSession& operator=(SpvcSession& session) = delete;

                SpvcSession& operator=(SpvcSession&& that) {
                    std::swap(this->context, that.context);
                    std::swap(this->compiler, that.compiler);
                    std::swap(this->ir, that.ir);
                    std::swap(this->compiler_options, that.compiler_options);
                    return *this;
                }

                spvc_result CreateOptions(spvc_compiler_options *options) {
                    return spvc_compiler_create_compiler_options(compiler, options);
                }

                spvc_result SetOptions(spvc_compiler_options options) {
                    compiler_options = options;
                    return spvc_compiler_install_compiler_options(compiler, options);
                }

                spvc_result Compile(const char** result) {
                    return spvc_compiler_compile(compiler, result);
                }

                const char* GetLastErrorString() {
                    return spvc_context_get_last_error_string(context);
                }

                ~SpvcSession() { spvc_context_destroy(context); }
            private:
                spvc_context context = nullptr;
                spvc_parsed_ir ir = nullptr;
                spvc_compiler compiler = nullptr;
                spvc_compiler_options compiler_options = nullptr;
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
