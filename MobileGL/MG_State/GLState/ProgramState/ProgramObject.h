#pragma once
#include <Includes.h>
#include "ShaderObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class ProgramObject {
            public:
                void AttachShader(SharedPtr<ShaderObject> shader);
                SizeT DetachShader(SharedPtr<ShaderObject> shader);
                void Link();
                void MarkAsDeleted();
            private:
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
