#include "ProgramObject.h"
#include <MG_Util/ShaderTranspiler/ShaderCompiler.h>

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            void ProgramObject::AttachShader(SharedPtr<ShaderObject> shader) {
                m_shaders.emplace_back(shader);
            }

            void ProgramObject::DetachShader(SharedPtr<ShaderObject> shader) {
                auto count = std::erase_if(
                    m_shaders, [shader](const SharedPtr<ShaderObject>& s) { return s.get() == shader.get(); });

                if (count == 0) {
                    // FIXME: handle error here
                    THROW_EXCEPTION("Program object does not have such shader object attached");
                }
            }

            void ProgramObject::Link() {
                Vector<GLenum> shaderTypes(m_shaders.size());
                Vector<SharedPtr<glslang::TShader>> shaders(m_shaders.size());
                for (SizeT i = 0; i < m_shaders.size(); i++) {
                    shaderTypes[i] = GetGLShaderTypeByMGLShaderStage(m_shaders[i]->m_stage);
                    shaders[i] = m_shaders[i]->m_shader;
                }

                MG_Util::ShaderTranspiler::ProgramAttrib attrib{
                    .shaderTypes = Move(shaderTypes),
                    .shaders = Move(shaders),
                };

                auto result = MG_Util::ShaderTranspiler::ShaderCompiler::LinkProgram(attrib);
                if (result) {
                    m_linkStatus = true;
                    m_programBinary = Move(result.value());
                } else {
                    m_linkStatus = false;
                    m_infoLog = result.error().log;

                    const std::string e = std::format("Shader link failed: \nerrc: {}\nmsg: {}\n", result.error().errc,
                                                      result.error().log);
                    THROW_EXCEPTION(e);
                }
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL