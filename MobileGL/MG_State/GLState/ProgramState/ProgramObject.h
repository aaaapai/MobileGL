#pragma once
#include <Includes.h>
#include "ShaderObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class ProgramObject {
            public:
                void AttachShader(SharedPtr<ShaderObject> shader);
                void DetachShader(SharedPtr<ShaderObject> shader);
                void Link();

            private:
                Vector<SharedPtr<ShaderObject>> m_shaders;
                // basically this contains SPIR-V in binary format
                Vector<Vector<Uint>> m_programBinary;
                String m_infoLog;
                bool m_deleteStatus = false;
                bool m_linkStatus = true;
                bool m_validateStatus = true;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
