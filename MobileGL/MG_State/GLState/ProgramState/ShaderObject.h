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
                Compute,
                Unknown = -1
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
                    return GL_FALSE;
                }
            }

            inline static ShaderStage GetMGLShaderStageByGLShaderType(GLenum type) {
                switch (type) {
                    case GL_VERTEX_SHADER:
                        return ShaderStage::Vertex;
                    case GL_TESS_CONTROL_SHADER:
                        return ShaderStage::TessControl;
                    case GL_TESS_EVALUATION_SHADER:
                        return ShaderStage::TessEval;
                    case GL_GEOMETRY_SHADER:
                        return ShaderStage::Geometry;
                    case GL_FRAGMENT_SHADER:
                        return ShaderStage::Fragment;
                    case GL_COMPUTE_SHADER:
                        return ShaderStage::Compute;
                    default:
                        assert(false);
                        return ShaderStage::Unknown;
                }
            }

            class ShaderObject {
            public:
                ShaderObject(const ShaderStage stage) : m_stage(stage) {}
                void SetShaderSource(const std::string& source);
                void Compile();
                void MarkAsDeleted();

                ShaderStage GetShaderStage() const { return m_stage; }
                const std::string& GetShaderSource() const { return m_source; }
                SharedPtr<glslang::TShader> GetCompiledShader() const { return m_shader; }
            private:
                const ShaderStage m_stage;
                std::string m_source;
                SharedPtr<glslang::TShader> m_shader;
                std::string m_infoLog;

                Bool m_deleteStatus = false;
                Bool m_compileStatus = false;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
