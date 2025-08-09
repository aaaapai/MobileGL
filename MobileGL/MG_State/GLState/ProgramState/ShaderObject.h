#pragma once
#include <Includes.h>

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            enum class ShaderStage {
                Vertex,
                TessControl,
                TessEval,
                Geometry,
                Fragment,
                Compute
            };

            inline static GLenum GetGLShaderTypeByMGLShaderStage(ShaderStage stage) {
                switch (stage) {
                case ShaderStage::Vertex:
                    return GL_VERTEX_SHADER;
                case ShaderStage::TessControl:
                    return GL_TESS_CONTROL_SHADER;
                case ShaderStage::TessEval:
                    return GL_TESS_EVALUATION_SHADER;
                case ShaderStage::Geometry:
                    return GL_GEOMETRY_SHADER;
                case ShaderStage::Fragment:
                    return GL_FRAGMENT_SHADER;
                case ShaderStage::Compute:
                    return GL_COMPUTE_SHADER;
                default:
                    assert(false);
                    return 0;
                }
            }

            class ShaderObject {
            public:
                ShaderObject(const ShaderStage stage) : m_stage(stage) {}
                void SetShaderSource(const std::string& source);
                void Compile();

                const ShaderStage m_stage;
                std::string m_source;
                SharedPtr<glslang::TShader> m_shader;
                std::string m_infoLog;

                bool m_deleteStatus = false;
                bool m_compileStatus = false;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
