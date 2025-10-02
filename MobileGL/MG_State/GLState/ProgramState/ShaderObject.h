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

            inline static GLenum ConvertGLShaderTypeByMGLShaderStage(ShaderStage stage) {
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

            inline static ShaderStage ConvertMGLShaderStageByGLShaderType(GLenum type) {
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
                ShaderObject(const ShaderStage stage, const Uint id) : m_stage(stage), m_id(id) {}
                void SetShaderSource(const String& source);
                void SetShaderSource(String&& source);
                void Compile();
                void MarkAsDeleted();

                Uint GetId() const { return m_id; }
                ShaderStage GetShaderStage() const { return m_stage; }
                const String& GetShaderSource() const { return m_source; }
                SharedPtr<glslang::TShader> GetCompiledShader() const { return m_shader; }
                const String& GetInfoLog() const { return m_infoLog; }
                const UnorderedMap<String, Uint>& GetUniformLocations() const { return m_uniforms; }
                Bool GetCompileStatus() const { return m_compileStatus; }
                Bool GetDeleteStatus() const { return m_deleteStatus; }

            private:
                // bool DoReflection();
                const Uint m_id = 0;
                const ShaderStage m_stage;
                String m_source;
                SharedPtr<glslang::TShader> m_shader;
                UnorderedMap<String, Uint> m_uniforms;

                String m_infoLog;
                Bool m_deleteStatus = false;
                Bool m_compileStatus = false;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
