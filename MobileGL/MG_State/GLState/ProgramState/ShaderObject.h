#pragma once
#include <Includes.h>

namespace MobileGL {
    enum class ShaderStage {
        Vertex,
        TessControl,
        TessEval,
        Geometry,
        Fragment,
        Compute,
        Unknown = -1
    };

    namespace MG_State {
        namespace GLState {
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
