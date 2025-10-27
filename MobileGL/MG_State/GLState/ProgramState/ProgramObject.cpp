#include "ProgramObject.h"
#include <MG_Util/ShaderTranspiler/ShaderCompiler.h>
#include <MG_Util/Converters/SPIRVCrossToGL/SpvcTypeConverter.h>

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
                // PreLink();

                Vector<GLenum> shaderTypes(m_shaders.size());
                Vector<SharedPtr<glslang::TShader>> shaders(m_shaders.size());
                for (SizeT i = 0; i < m_shaders.size(); i++) {
                    shaderTypes[i] = ConvertGLShaderTypeByMGLShaderStage(m_shaders[i]->GetShaderStage());
                    shaders[i] = m_shaders[i]->GetCompiledShader();
                }

                MG_Util::ShaderTranspiler::ProgramAttrib attrib{
                    // .shaderTypes = Move(shaderTypes),
                    .shaders = Move(shaders),
                };

                auto result = MG_Util::ShaderTranspiler::ShaderCompiler::LinkProgram(attrib);
                if (result) {
                    m_linkStatus = true;
                    m_program = result.value();
                } else {
                    m_linkStatus = false;
                    m_infoLog = result.error().log;
                }

                // PostLink();

                DoReflection();
                GenerateBinary();
            }

            void ProgramObject::MarkAsDeleted() {
                m_deleteStatus = true;
            }

            Vector<SharedPtr<ShaderObject>>& ProgramObject::GetAttachedShaders() {
                return m_shaders;
            }

            void ProgramObject::DoReflection() {
                if (!m_program->buildReflection()) {
                    m_linkStatus = false;
                    m_infoLog = "Build reflection failed.";
                    return;
                }

                // ------------ Uniforms (GL Plain) ----------------
                // Allocate uniform locations
                m_activeUniformCount = m_program->getNumUniformVariables();
                for (int i = 0; i < m_activeUniformCount; i++) {
                    auto& uniform = m_program->getUniform(i);
                    auto location = uniform.layoutLocation();
                    if (location != 4095) {
                        m_maxUniformLocation = std::max(m_maxUniformLocation, location);
                    }
                    m_uniformNameMaxLength = std::max(m_uniformNameMaxLength, (Int)uniform.name.length());
                    m_uniformLocations[uniform.name] = location;
                }

                if (m_maxUniformLocation + 1 < m_activeUniformCount) {
                    // This means we have fewer than enough gaps to fit
                    // unallocated uniforms
                    m_maxUniformLocation = m_activeUniformCount;
                }

                // i-th elements refers to uniform at layout(location = i, ...)
                // Be aware, there could be gaps in between these vectors
                // Locations can be not sequential
                m_uniformNames.resize(m_maxUniformLocation + 1);
                m_uniformTypes.resize(m_maxUniformLocation + 1, GL_ZERO);
                m_uniformIsOpaqueType.resize(m_maxUniformLocation + 1);
                m_uniformOffsets.resize(m_maxUniformLocation + 1);
                m_uniformArraySizes.resize(m_maxUniformLocation + 1);

                Vector<int> unallocatedUniformIndex;

                // Populate vector with already allocated location
                for (int i = 0; i < m_activeUniformCount; i++) {
                    auto& uniform = m_program->getUniform(i);
                    auto location = uniform.layoutLocation();
                    if (location >= m_uniformNames.size()) {
                        unallocatedUniformIndex.emplace_back(i);
                        continue; // will allocate unallocated uniforms later
                    }
                    m_uniformNames[location] = uniform.name;
                    m_uniformTypes[location] = uniform.glDefineType;
                    m_uniformIsOpaqueType[location] = uniform.getType()->isOpaque();
                    m_uniformArraySizes[location] = uniform.size;
                }

                SizeT locNeedle = 0;
                for (auto index : unallocatedUniformIndex) {
                    auto& uniform = m_program->getUniform(index);
                    for (; locNeedle <= m_maxUniformLocation; locNeedle++) {
                        if (m_uniformTypes[locNeedle] != GL_ZERO) continue;
                        // Found a vacant location at locNeedle
                        m_uniformNames[locNeedle] = uniform.name;
                        m_uniformTypes[locNeedle] = uniform.glDefineType;
                        m_uniformIsOpaqueType[locNeedle] = uniform.getType()->isOpaque();
                        m_uniformArraySizes[locNeedle] = uniform.size;
                        m_uniformLocations[uniform.name] = locNeedle;
                        break;
                    }
                }

                int inCount = m_program->getNumPipeInputs();

                int maxLoc = -1;
                for (int i = 0; i < inCount; ++i) {
                    int loc = m_program->getPipeInput(i).layoutLocation();
                    if (loc >= 0) maxLoc = std::max(maxLoc, loc);
                }

                if (maxLoc < 0) {
                    maxLoc = std::max(0, inCount - 1);
                }

                GLint maxAttribs = 16; // TODO: get from backend

                if (maxLoc >= maxAttribs) {
                    MGLOG_W("ProgramObject::DoReflection - required attrib location %d >= GL_MAX_VERTEX_ATTRIBS (%d). "
                            "Clamping.",
                            maxLoc, maxAttribs);
                    maxLoc = maxAttribs - 1;
                }

                m_attribs.resize(maxLoc + 1);
                m_attribTypes.resize(maxLoc + 1);

                for (int i = 0; i < inCount; ++i) {
                    auto& inVar = m_program->getPipeInput(i);
                    int location = inVar.layoutLocation();
                    m_attribInNameMaxLength = std::max(m_attribInNameMaxLength, (Int)inVar.name.length());

                    if (location >= 0 && location < (int)m_attribs.size()) {
                        m_attribs[location] = inVar.name;
                        m_attribTypes[location] = inVar.glDefineType;
                    } else if (location >= (int)m_attribs.size()) {
                        MGLOG_W("ProgramObject::DoReflection - attrib location %d >= attribs.size() (%zu). Ignoring.",
                                location, m_attribs.size());
                        continue;
                    } else {
                        bool placed = false;
                        for (size_t idx = 0; idx < m_attribs.size(); ++idx) {
                            if (m_attribs[idx].empty()) {
                                m_attribs[idx] = inVar.name;
                                m_attribTypes[idx] = inVar.glDefineType;
                                placed = true;
                                break;
                            }
                        }
                        if (!placed && (int)m_attribs.size() < maxAttribs) {
                            m_attribs.push_back(inVar.name);
                            m_attribTypes.push_back(inVar.glDefineType);
                            placed = true;
                        }
                        if (!placed) {
                            MGLOG_W("ProgramObject::DoReflection - cannot place attrib '%s' (no free slot and at max "
                                    "capacity). Ignoring.",
                                    inVar.name.c_str());
                        }
                    }
                }

                // Implement glBindAttribLocation semantics (explicit locations set by user)
                for (auto& [name, location] : m_explicitAttribLocations) {
                    if (location < 0) continue;
                    if (location >= (int)m_attribs.size()) {
                        if (location >= maxAttribs) {
                            MGLOG_W("SetExplicitAttribLocation: requested location %d >= GL_MAX_VERTEX_ATTRIBS (%d). "
                                    "Ignored for attribute '%s'.",
                                    location, maxAttribs, name.c_str());
                            continue;
                        }
                        m_attribs.resize(location + 1);
                        m_attribTypes.resize(location + 1);
                    }

                    if (m_attribs[location] != name) {
                        auto it = std::find(m_attribs.begin(), m_attribs.end(), name);
                        if (it == m_attribs.end()) continue;
                        auto idx = std::distance(m_attribs.begin(), it);
                        std::swap(m_attribs[location], m_attribs[idx]);
                        std::swap(m_attribTypes[location], m_attribTypes[idx]);
                    }
                }

                // ---------- UBO ----------
                int uboCount = m_program->getNumUniformBlocks();
                for (int i = 0; i < uboCount; i++) {
                    auto& ubo = m_program->getUniformBlock(i);
                    m_uniformBlockNameMaxLength = std::max(m_uniformBlockNameMaxLength, (Int)ubo.name.length());
                }
            }

            void ProgramObject::GenerateBinary() {
                /* As we passed first stage compilation/linking,
                 * we'll assume all the operations here should
                 * pass. We may be able to employ some optimizations
                 * here without the burden of error reporting.
                 */
                using namespace MG_Util::ShaderTranspiler;
                Vector<SharedPtr<glslang::TShader>> shaders(m_shaders.size());
                Vector<GLenum> shaderTypes(m_shaders.size());
                for (SizeT i = 0; i < m_shaders.size(); i++) {
                    auto shaderType = ConvertGLShaderTypeByMGLShaderStage(m_shaders[i]->GetShaderStage());
                    shaderTypes[i] = shaderType;
                    ShaderAttrib attrib{
                        .shaderType = shaderType, .sourceStr = m_shaders[i]->GetShaderSource(), .flags = 0};
                    auto res = ShaderCompiler::CompileShader(attrib);
                    assert(res);
                    shaders[i] = res.value();
                }

                ProgramAttrib attrib{
                    .shaders = Move(shaders),
                };
                auto programResult = ShaderCompiler::LinkProgram(attrib);
                assert(programResult);
                auto program = programResult.value();

                ProgramBinaryAttrib binaryAttrib{
                    .shaderTypes = shaderTypes,
                    .program = *program,
                };
                auto binaryResult = ShaderCompiler::GetSpirvBinaryFromProgram(binaryAttrib);
                assert(binaryResult);
                m_generatedSpirv = Move(binaryResult.value());

                SpvcSession session(m_generatedSpirv[0]);
                auto srcResult = ShaderCompiler::DecompileShader(session);
                assert(srcResult);
                // auto src = srcResult.value();
                // printf("decompiled src: \n%s\n", src.c_str());

                auto& meta = session.GetMetadata();
                auto size = meta.uboSize;
                m_uboScratch.resize(size);
                m_uniformOffsets.resize(m_maxUniformLocation + 1);
                for (const auto& [name, offset] : meta.plainUniformOffsetsInUBO) {
                    if (m_uniformLocations.find(name) != m_uniformLocations.end())
                        m_uniformOffsets[m_uniformLocations[name]] = offset;
                }
                m_uniformSizesInBytes.resize(m_maxUniformLocation + 1);
                for (const auto& [name, size] : meta.plainUniformMemberSizesInBytes) {
                    if (m_uniformLocations.find(name) != m_uniformLocations.end())
                        m_uniformSizesInBytes[m_uniformLocations[name]] = size;
                }
                // assert(m_uniformOffsets.size() == GetUniformCount());
                // assert(m_uniformSizesInBytes.size() == GetUniformCount());
            }

            void ProgramObject::WaitUntilGenerationCompleted() {}

            void ProgramObject::SetExplicitAttribLocation(Uint index, const char* name) {
                m_explicitAttribLocations[name] = index;
            }

            // void ProgramObject::PreLink() {
            //     m_uniforms.clear();
            //     m_uniformOffsets.clear();
            //
            //     for (const auto& shader : m_shaders) {
            //         for (const auto& [name, loc] : shader->GetUniformLocations()) {
            //             // collect all the names to map
            //             if (loc != 4095 || m_uniforms.find(name) == m_uniforms.end()) {
            //                 m_uniforms[name] = loc;
            //             }
            //
            //             // set a flag for those who have an explicit location
            //             if (loc != 4095) {
            //                 if (loc >= m_uniformOffsets.size()) {
            //                     m_uniformOffsets.reserve(std::bit_ceil(loc + 1));
            //                     m_uniformOffsets.resize(loc + 1, 0);
            //                 }
            //                 assert(m_uniformOffsets[loc] == 0);
            //                 m_uniformOffsets[loc] = 1;
            //             }
            //         }
            //     }
            //
            //     // Let's find a location for those who doesn't have one yet
            //     Uint nextLocation = 0;
            //
            //     // Find first empty location
            //     for (SizeT i = 0; i < m_uniformOffsets.size(); i++) {
            //         if (m_uniformOffsets[i] == 0) {
            //             nextLocation = i;
            //             break;
            //         }
            //     }
            //
            //     for (auto& [name, loc] : m_uniforms) {
            //         if (loc == 4095) {
            //             // check if we drained all the holes already
            //             if (nextLocation >= m_uniformOffsets.size()) {
            //                 loc = nextLocation;
            //                 m_uniformOffsets.push_back(1);
            //                 nextLocation++;
            //                 continue;
            //             }
            //
            //             // assign an empty location
            //             loc = nextLocation;
            //             m_uniformOffsets[loc] = 1;
            //
            //             // Find next empty location
            //             for (nextLocation++; nextLocation < m_uniformOffsets.size(); nextLocation++) {
            //                 if (m_uniformOffsets[nextLocation] == 0) break;
            //             }
            //         }
            //     }
            //
            //     m_uniformNames.resize(m_uniformOffsets.size());
            //     for (auto& [name, loc] : m_uniforms) {
            //         m_uniformNames[loc] = name;
            //         m_uniformNameMaxLength = std::max(m_uniformNameMaxLength, (Int)name.length());
            //     }
            // }
            //
            // void ProgramObject::PostLink() {
            // if (m_programBinary.empty()) {
            //     assert(false);
            //     return;
            // }
            // MG_Util::ShaderTranspiler::SpvcSession session(m_programBinary[0]);
            // const char* src = nullptr; // we don't care the source atm
            // auto result = session.Compile(&src);
            // if (result != SPVC_SUCCESS) {
            //     assert(false);
            //     return;
            // }
            // m_metadata = session.GetMetadata();
            // auto& uniformOffsets = m_metadata.plainUniformOffsetsInUBO;
            // for (const auto& [name, offset] : uniformOffsets) {
            //     assert(m_uniforms.find(name) != m_uniforms.end());
            //     assert(m_uniforms[name] < m_uniformOffsets.size());
            //     m_uniformOffsets[m_uniforms[name]] = offset;
            // }
            // m_uboScratch.resize(m_metadata.uboSize, 0);
            //
            // auto& types = m_metadata.plainUniformMemberTypes;
            //
            // assert(types.size() == m_uniformOffsets.size());
            // m_uniformTypes.resize(m_uniformOffsets.size());
            // for (const auto& [name, type] : types) {
            //     auto gltype = MG_Util::ConvertSpvcTypeToGLEnum(type);
            //     auto location = m_uniforms[name];
            //     m_uniformTypes[location] = gltype;
            // }
            // }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL