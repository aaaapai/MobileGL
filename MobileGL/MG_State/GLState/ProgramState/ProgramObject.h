#pragma once
#include <Includes.h>
#include "ShaderObject.h"

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
                Int GetUniformLocation(const String& name) {
                    const auto it = m_uniforms.find(name);
                    return (it == m_uniforms.end()) ? -1 : it->second;
                }
            private:
                void PreLink();
                void PostLink();

                const Uint m_id = 0;
                Vector<SharedPtr<ShaderObject>> m_shaders;
                // basically this contains SPIR-V in binary format
                Vector<Vector<Uint>> m_programBinary;

                // Uniforms
                UnorderedMap<String, Uint> m_uniforms;
                // 0 or 1 for if the location is explicitly specified at PreLink stage,
                // offsets into global ubo for PostLink
                Vector<Uint> m_uniformOffsets;
                Vector<Uint8> m_uboScratch;

                String m_infoLog;
                Bool m_deleteStatus = false;
                Bool m_linkStatus = true;
                Bool m_validateStatus = true;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
