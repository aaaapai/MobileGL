#pragma once
#include <Includes.h>
#include "ShaderObject.h"
#include "MG_Util/ShaderTranspiler/SpvcSession.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class ProgramObject {
            public:
                ProgramObject(const Uint id): m_id(id) {}
                bool ShaderIsAttached(SharedPtr<ShaderObject> shader);
                bool AttachShader(SharedPtr<ShaderObject> shader);
                SizeT DetachShader(SharedPtr<ShaderObject> shader);
                void Link();
                void MarkAsDeleted();
                Vector<SharedPtr<ShaderObject>>& GetAttachedShaders();
                const String& GetInfoLog() const { return m_infoLog; }
                Int GetUniformMaxLength() const { return m_uniformNameMaxLength; }
                Uint GetUniformCount() { return m_uniformOffsets.size(); }
                Int GetUniformLocation(const String& name) {
                    const auto it = m_uniforms.find(name);
                    return (it == m_uniforms.end()) ? -1 : it->second;
                }
                GLenum GetUniformType(Uint index) const {
                    return m_uniformTypes[index];
                }

                const String& GetUniformName(Uint index) const {
                    return m_uniformNames[index];
                }
            private:
                void PreLink();
                void PostLink();

                const Uint m_id = 0;
                Vector<SharedPtr<ShaderObject>> m_shaders;

                SharedPtr<glslang::TProgram> m_program;

                // Uniforms
                MG_Util::ShaderTranspiler::SpvcMetadata m_metadata;

                UnorderedMap<String, Uint> m_uniforms;
                // 0 or 1 for if the location is explicitly specified at PreLink stage,
                // offsets into global ubo for PostLink
                Vector<Uint> m_uniformOffsets;
                Vector<String> m_uniformNames;
                Vector<GLenum> m_uniformTypes;

                Vector<Uint8> m_uboScratch;

                Int m_uniformNameMaxLength = 0;

                String m_infoLog;
                Bool m_deleteStatus = false;
                Bool m_linkStatus = true;
                Bool m_validateStatus = true;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
