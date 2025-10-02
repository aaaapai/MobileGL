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
                ProgramObject(const Uint id) : m_id(id) {}
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
                    return (it == m_uniformLocations.end()) ? -1 : (Int)it->second;
                }
                GLenum GetUniformType(Uint index) const { return m_uniformTypes[index]; }

                Bool IsUniformOpaqueAtLocation(Uint location) const { return m_uniformIsOpaqueType[location]; }

                const String& GetUniformName(Uint index) const { return m_uniformNames[index]; }
                Uint GetUniformOffset(Uint location) const { return m_uniformOffsets[location]; }
                Uint GetUniformSizesInBytes(Uint location) const {
                    return MG_Util::GetGLTypeSize(m_uniformTypes[location]);
                }

                Int GetAttributeLocation(const String& name) {
                    const auto it = std::find(m_attribs.begin(), m_attribs.end(), name);
                    return (it == m_attribs.end()) ? -1 : std::distance(m_attribs.begin(), it);
                }
                GLenum GetAttribType(Uint index) const { return m_attribTypes[index]; }
                const String& GetAttribName(Uint index) const { return m_attribs[index]; }
                void* MapUBO() { return m_uboScratch.data(); }

                Bool GetDeleteStatus() const { return m_deleteStatus; }
                Bool GetLinkStatus() const { return m_linkStatus; }
                Bool GetValidateStatus() const { return m_validateStatus; }
                Int GetActiveAtomicCounterCount() const { return m_program->getNumAtomicCounters(); }
                Int GetActiveAttributesCount() const { return m_program->getNumPipeInputs(); }
                Int GetActiveUniformBlocksCount() const { return m_program->getNumUniformBlocks(); }
                Int GetActiveAttributesMaxLength() const { return m_attribInNameMaxLength; }
                Int GetActiveUniformBlocksMaxNameLength() const { return m_uniformBlockNameMaxLength; }

            private:
                void DoReflection();
                void GenerateBinary();
                void WaitUntilGenerationCompleted();
                // void PreLink();
                // void PostLink();

                const Uint m_id = 0;
                Vector<SharedPtr<ShaderObject>> m_shaders;

                SharedPtr<glslang::TProgram> m_program;

                Vector<Vector<unsigned>> m_generatedSpirv;

                // Attributes
                UnorderedMap<String, Uint> m_explicitAttribLocations;
                Vector<String> m_attribs;
                Vector<GLenum> m_attribTypes;

                // Uniforms
                // MG_Util::ShaderTranspiler::SpvcMetadata m_metadata;

                UnorderedMap<String, Uint> m_uniformLocations;
                // Ordered by location,
                // aka. m_uniformNames[loc] == "name at location `loc`"
                Vector<String> m_uniformNames;
                // ditto.
                Vector<GLenum> m_uniformTypes;
                Vector<Bool> m_uniformIsOpaqueType;
                Vector<Int> m_uniformArraySizes;

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
