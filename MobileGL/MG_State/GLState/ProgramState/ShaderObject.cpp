#include "../../../Includes.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            void ShaderObject::SetShaderSource(const std::string& source) {
                m_source = source;
            }

            void ShaderObject::Compile() {
                using namespace MG_Util::ShaderTranspiler;
                ShaderAttrib attrib {
                    .sourceStr =  m_source,
                    .shaderType = GetGLShaderTypeByMGLShaderStage(m_stage),
                    .flags = 0
                };

                auto result = ShaderCompiler::CompileShader(attrib);
                if (result) {
                    m_compileStatus = true;
                    m_shader = result.value();
                } else {
                    m_compileStatus = false;
                    m_infoLog = result.error().log;

                    const std::string e =
                        std::format("Shader compilation failed: \nerrc: {}\nmsg: {}\n",
                            result.error().errc, result.error().log);
                    THROW_EXCEPTION(e);
                }
            }
        }
    }
}