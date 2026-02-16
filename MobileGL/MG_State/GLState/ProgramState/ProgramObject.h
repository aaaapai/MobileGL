// MobileGL - MobileGL/MG_State/GLState/ProgramState/ProgramObject.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include "ShaderObject.h"
#include "MG_Util/Metrics/BufferMetrics.h"
#include "MG_Util/ShaderTranspiler/SpvcSession.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class ProgramObject {
            public:
                ProgramObject(Uint externalIndex) : m_externalIndex(externalIndex) {}
                bool ShaderIsAttached(SharedPtr<ShaderObject> shader);
                bool AttachShader(SharedPtr<ShaderObject> shader);
                SizeT DetachShader(SharedPtr<ShaderObject> shader);
                SizeT RemoveShader(SharedPtr<ShaderObject> shader);
                void Link(Bool addDefaultFSIfMissingForRenderingPipelineProgram = false);
                void MarkAsDeleted();

                void SetExplicitVertexInLocation(Uint index, const char* name);
                void SetExplicitFragmentOutLocation(Uint index, const char* name);
                Int GetFragmentDataLocation(const char* name);

                Vector<SharedPtr<ShaderObject>>& GetAttachedShaders();
                const Vector<SharedPtr<ShaderObject>>& GetAttachedShaders() const;
                const String& GetInfoLog() const { return m_infoLog; }
                Int GetUniformMaxLength() const { return m_uniformNameMaxLength; }
                Uint GetUniformCount() { return m_activeUniformCount; }
                Uint GetMaxUniformLocation() const { return m_maxUniformLocation; }
                Int GetUniformLocation(const String& name) {
                    const auto it = m_uniformLocations.find(name);
                    if (it == m_uniformLocations.end()) return -1;
                    return (Int)it->second;
                }

                GLenum GetUniformType(Uint location) const {
                    auto& uniform = m_program->getUniform(m_uniformIndexInTProgram[location]);
                    return uniform.glDefineType;
                }

                const glslang::TType* GetUniformTType(Uint location) const {
                    auto& uniform = m_program->getUniform(m_uniformIndexInTProgram[location]);
                    return uniform.getType();
                }

                Bool IsUniformOpaqueAtLocation(Uint location) const { return GetUniformTType(location)->isOpaque(); }

                const String& GetUniformName(Uint location) const {
                    auto& uniform = m_program->getUniform(m_uniformIndexInTProgram[location]);
                    return uniform.name;
                }
                Uint GetUniformOffset(Uint location) const { return m_uniformOffsets[location]; }
                Uint GetUniformSizesInBytes(Uint location) const {
                    return MG_Util::GetGLTypeSize(GetUniformType(location));
                }

                Int GetAttributeLocation(const String& name) {
                    const auto it = std::find(m_attribs.begin(), m_attribs.end(), name);
                    return (it == m_attribs.end()) ? -1 : std::distance(m_attribs.begin(), it);
                }
                GLenum GetAttribType(Uint index) const { return m_attribTypes[index]; }
                const String& GetAttribName(Uint index) const { return m_attribs[index]; }
                void* MapUBO() { return m_uboScratch.data(); }
                Uint GetUBOSize() const { return static_cast<Uint>(m_uboScratch.size()); }

                void SetUniformSamplerOrImageUnitIndex(Uint location, Int unit) {
                    m_uniformSamplerOrImageUnitIndex[location] = unit;
                }

                Int GetUniformSamplerOrImageUnitIndex(Uint location) const {
                    return m_uniformSamplerOrImageUnitIndex[location];
                }

                Bool GetDeleteStatus() const { return m_deleteStatus; }
                Bool GetLinkStatus() const { return m_linkStatus; }
                Bool GetValidateStatus() const { return m_validateStatus; }
                Int GetActiveAtomicCounterCount() const { return m_program->getNumAtomicCounters(); }
                Int GetActiveAttributesCount() const { return m_program->getNumPipeInputs(); }
                Int GetActiveUniformBlocksCount() const { return m_program->getNumUniformBlocks(); }
                Int GetActiveAttributesMaxLength() const { return m_attribInNameMaxLength; }
                Int GetActiveUniformBlocksMaxNameLength() const { return m_uniformBlockNameMaxLength; }
                Uint GetUniformBlockIndex(const char* name) const {
                    auto it = m_uniformBlockIndexByName.find(name);
                    if (it != m_uniformBlockIndexByName.end()) return it->second;
                    return 0xFFFFFFFFu; // GL_INVALID_INDEX
                }
                Bool IsActiveUniformBlock(Uint index) const {
                    if (index >= GetActiveUniformBlocksCount()) return false;
                    return true;
                }
                Uint GetUBOSizeAt(Uint index) const {
                    if (!IsActiveUniformBlock(index)) return 0;
                    return m_program->getUniformBlock(index).size;
                }

                const String& GetUniformBlockName(Uint index) const {
                    auto& ubo = m_program->getUniformBlock(index);
                    return ubo.name;
                }

                // Set by glUniformBlockBinding
                void SetUniformBlockBinding(Uint index, Uint binding) { m_uniformBlockBinding[index] = binding; }

                Uint GetUniformBlockBinding(Uint index) const { return m_uniformBlockBinding[index]; }

                Vector<Vector<unsigned>>& GetGeneratedSpirv() { return m_generatedSpirv; }
                const Vector<Vector<unsigned>>& GetGeneratedSpirv() const { return m_generatedSpirv; }

                Int GetShaderIndexByStage(ShaderStage stage) const {
                    auto it = std::find_if(m_shaders.begin(), m_shaders.end(),
                        [stage](const SharedPtr<ShaderObject>& shader) { return shader->GetShaderStage() == stage; });
                    return it == m_shaders.end() ? -1 : std::distance(m_shaders.begin(), it);
                }

                Uint GetExternalIndex() const { return m_externalIndex; }

                //                const UnorderedMap<String, Uint>& GetAttribLocationMap() const { return
                //                m_attribLocation; }

            private:
                void DoReflection();
                void GenerateBinary();
                void WaitUntilGenerationCompleted();
                void AddDefaultFragmentShaderIfMissing();

                const Uint m_externalIndex = 0;
                Vector<SharedPtr<ShaderObject>> m_shaders;
                Vector<SharedPtr<ShaderObject>> m_detachedShaders; // Store detached shaders and remove on next link

                SharedPtr<glslang::TProgram> m_program;

                Vector<Vector<unsigned>> m_generatedSpirv;

                // Attributes (Vertex in)
                UnorderedMap<String, Uint> m_explicitAttribLocations;
                Vector<String> m_attribs;
                Vector<GLenum> m_attribTypes;
                // For SpvcSession::SetVertexAttribLocation()
                //                UnorderedMap<String, Uint> m_attribLocation;

                // FragData (Frag out)
                UnorderedMap<String, Uint> m_explicitFragDataLocation;

                // Uniforms
                UnorderedMap<String, Uint> m_uniformLocations;
                // Ordered by location,
                // aka. m_uniformIndexInTProgram[loc] == "uniform index of TProgram at location `loc`"
                Vector<Int> m_uniformIndexInTProgram;
                // ditto. Will be set at glUniform1i
                Vector<Int> m_uniformSamplerOrImageUnitIndex;

                // Ordered by uniform block index
                // index is DIFFERENT from binding!!!
                //
                // Let's define UniformBlockIndex == the order at glslang getUniformBlock()
                // aka `i = glGetUniformBlockIndex(prog, "BlockName")` implies:
                // `prog->getUniformBlock(i) == "BlockName"`
                UnorderedMap<String, Uint> m_uniformBlockIndexByName;
                Vector<Int> m_uniformBlockBinding;

                // Need to be reflected after linking of SPIR-V binary
                Vector<Uint> m_uniformOffsets;
                Vector<Uint> m_uniformSizesInBytes;
                Vector<Uint8> m_uboScratch;

                Uint m_activeUniformCount = 0;
                Uint m_maxUniformLocation = 0;
                Int m_uniformNameMaxLength = 0;
                Int m_attribInNameMaxLength = 0;
                Int m_uniformBlockNameMaxLength = 0;

                String m_infoLog;
                Bool m_deleteStatus = false;
                Bool m_linkStatus = false;
                Bool m_validateStatus = true;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
