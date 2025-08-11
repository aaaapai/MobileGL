#include "ShaderObject.h"
#include <MG_Util/ShaderTranspiler/ShaderCompiler.h>

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            void ShaderObject::SetShaderSource(const std::string& source) {
                m_source = source;
            }

            void ShaderObject::Compile() {
                using namespace MG_Util::ShaderTranspiler;
                ShaderAttrib attrib{
                    .shaderType = GetGLShaderTypeByMGLShaderStage(m_stage),
                    .sourceStr = m_source,
                    .flags = 0
                };

                auto result = ShaderCompiler::CompileShader(attrib);
                if (result) {
                    m_compileStatus = true;
                    m_shader = result.value();
                } else {
                    m_compileStatus = false;
                    m_infoLog = result.error().log;

                    const std::string e = std::format("Shader compilation failed: \nerrc: {}\nmsg: {}\n",
                                                      result.error().errc, result.error().log);
                }
            }

            void ShaderObject::MarkAsDeleted() {
                m_deleteStatus = true;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL