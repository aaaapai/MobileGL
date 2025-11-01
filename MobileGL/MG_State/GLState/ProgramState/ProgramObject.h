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
                void Link();
                void MarkAsDeleted();

                void SetExplicitAttribLocation(Uint index, const char* name);

                Vector<SharedPtr<ShaderObject>>& GetAttachedShaders();
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

                Vector<Vector<unsigned>>& GetGeneratedSpirv() { return m_generatedSpirv; }

                Uint GetExternalIndex() const { return m_externalIndex; }

            private:
                void DoReflection();
                void GenerateBinary();
                void WaitUntilGenerationCompleted();

                const Uint m_externalIndex = 0;
                Vector<SharedPtr<ShaderObject>> m_shaders;

                SharedPtr<glslang::TProgram> m_program;

                Vector<Vector<unsigned>> m_generatedSpirv;

                // Attributes
                UnorderedMap<String, Uint> m_explicitAttribLocations;
                Vector<String> m_attribs;
                Vector<GLenum> m_attribTypes;

                // Uniforms
                UnorderedMap<String, Uint> m_uniformLocations;
                // Ordered by location,
                // aka. m_uniformIndexInTProgram[loc] == "uniform index of TProgram at location `loc`"
                Vector<Int> m_uniformIndexInTProgram;
                // ditto. Will be set at glUniform1i
                Vector<Int> m_uniformSamplerOrImageUnitIndex;

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
