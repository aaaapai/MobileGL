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
            private:
                const Uint m_id = 0;
                Vector<SharedPtr<ShaderObject>> m_shaders;
                // basically this contains SPIR-V in binary format
                Vector<Vector<Uint>> m_programBinary;
                String m_infoLog;
                Bool m_deleteStatus = false;
                Bool m_linkStatus = true;
                Bool m_validateStatus = true;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
