#include "ProgramObject.h"
#include <MG_Util/ShaderTranspiler/ShaderCompiler.h>

#include "MG_Util/Converters/SPIRVCrossToGL/SpvcTypeConverter.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            bool ProgramObject::ShaderIsAttached(SharedPtr<ShaderObject> shader) {
                auto it = std::find_if(m_shaders.begin(), m_shaders.end(),
                                       [shader](const SharedPtr<ShaderObject>& s) { return s.get() == shader.get(); });
                return it != m_shaders.end();
            }

            bool ProgramObject::AttachShader(SharedPtr<ShaderObject> shader) {
                if (ShaderIsAttached(shader)) return false;
                m_shaders.emplace_back(shader);
                return true;
            }

            SizeT ProgramObject::DetachShader(SharedPtr<ShaderObject> shader) {
                auto count = std::erase_if(
                    m_shaders, [shader](const SharedPtr<ShaderObject>& s) { return s.get() == shader.get(); });

                return count;
            }

            void ProgramObject::Link() {
                PreLink();

                Vector<GLenum> shaderTypes(m_shaders.size());
                Vector<SharedPtr<glslang::TShader>> shaders(m_shaders.size());
                for (SizeT i = 0; i < m_shaders.size(); i++) {
                    shaderTypes[i] = GetGLShaderTypeByMGLShaderStage(m_shaders[i]->GetShaderStage());
                    shaders[i] = m_shaders[i]->GetCompiledShader();
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

                PostLink();
            }

            void ProgramObject::MarkAsDeleted() {
                m_deleteStatus = true;
            }

            Vector<SharedPtr<ShaderObject>>& ProgramObject::GetAttachedShaders() {
                return m_shaders;
            }

            void ProgramObject::PreLink() {
                m_uniforms.clear();
                m_uniformOffsets.clear();

                for (const auto& shader : m_shaders) {
                    for (const auto& [name, loc] : shader->GetUniformLocations()) {
                        // collect all the names to map
                        if (loc != 4095 || m_uniforms.find(name) == m_uniforms.end()) {
                            m_uniforms[name] = loc;
                        }

                        // set a flag for those who have an explicit location
                        if (loc != 4095) {
                            if (loc >= m_uniformOffsets.size()) {
                                m_uniformOffsets.reserve(std::bit_ceil(loc + 1));
                                m_uniformOffsets.resize(loc + 1, 0);
                            }
                            assert(m_uniformOffsets[loc] == 0);
                            m_uniformOffsets[loc] = 1;
                        }
                    }
                }

                // Let's find a location for those who doesn't have one yet
                Uint nextLocation = 0;

                // Find first empty location
                for (SizeT i = 0; i < m_uniformOffsets.size(); i++) {
                    if (m_uniformOffsets[i] == 0) {
                        nextLocation = i;
                        break;
                    }
                }

                for (auto& [name, loc] : m_uniforms) {
                    if (loc == 4095) {
                        // check if we drained all the holes already
                        if (nextLocation >= m_uniformOffsets.size()) {
                            loc = nextLocation;
                            m_uniformOffsets.push_back(1);
                            nextLocation++;
                            continue;
                        }

                        // assign an empty location
                        loc = nextLocation;
                        m_uniformOffsets[loc] = 1;

                        // Find next empty location
                        for (nextLocation++; nextLocation < m_uniformOffsets.size(); nextLocation++) {
                            if (m_uniformOffsets[nextLocation] == 0) break;
                        }
                    }
                }

                m_uniformNames.resize(m_uniformOffsets.size());
                for (auto& [name, loc] : m_uniforms) {
                    m_uniformNames[loc] = name;
                    m_uniformNameMaxLength = std::max(m_uniformNameMaxLength, (Int)name.length());
                }
            }

            void ProgramObject::PostLink() {
                if (m_programBinary.empty()) {
                    assert(false);
                    return;
                }
                MG_Util::ShaderTranspiler::SpvcSession session(m_programBinary[0]);
                const char* src = nullptr; // we don't care the source atm
                auto result = session.Compile(&src);
                if (result != SPVC_SUCCESS) {
                    assert(false);
                    return;
                }
                m_metadata = session.GetMetadata();
                auto& uniformOffsets = m_metadata.plainUniformOffsetsInUBO;
                for (const auto& [name, offset] : uniformOffsets) {
                    assert(m_uniforms.find(name) != m_uniforms.end());
                    assert(m_uniforms[name] < m_uniformOffsets.size());
                    m_uniformOffsets[m_uniforms[name]] = offset;
                }
                m_uboScratch.resize(m_metadata.uboSize, 0);

                auto& types = m_metadata.plainUniformMemberTypes;

                assert(types.size() == m_uniformOffsets.size());
                m_uniformTypes.resize(m_uniformOffsets.size());
                for (const auto& [name, type] : types) {
                    auto gltype = MG_Util::ConvertSpvcTypeToGLEnum(type);
                    auto location = m_uniforms[name];
                    m_uniformTypes[location] = gltype;
                }
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL