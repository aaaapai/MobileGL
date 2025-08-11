#pragma once
#include <Includes.h>
#include <MG_Util/Miscellany/IndexGenerator.h>
#include "ProgramObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class ProgramState {
            public:
                // This function WILL actually create the program object.
                // To retrieve created program object, use GetProgramObject()
                Uint CreateProgram();
                SharedPtr<ProgramObject> GetProgramObject(Uint id);
                void MarkProgramObjectForDeletion(Uint program);
                Bool ValidateProgramObject(Uint program) const;

                void UseProgram(Uint program);

                Uint CreateShader(ShaderStage stage);
                SharedPtr<ShaderObject> GetShaderObject(Uint shader);
                void MarkShaderObjectForDeletion(Uint shader);
                Bool ValidateShaderObject(Uint shader) const;
            private:
                template <typename T>
                static Bool CheckIndexAvail(const SizeT idx, const Vector<T>& vec) {
                    return idx < vec.size();
                }

                template <typename T>
                static void EnsureIndexAvail(const SizeT idx, Vector<T>& vec) {
                    if (CheckIndexAvail(idx, vec)) return;

                    vec.reserve(std::bit_ceil(idx));
                    vec.resize(idx + 1);
                }

                IndexGenerator<Uint> m_programIndexGenerator;
                Vector<SharedPtr<ProgramObject>> m_programObjects;

                IndexGenerator<Uint> m_shaderIndexGenerator;
                Vector<SharedPtr<ShaderObject>> m_shaderObjects;

                SharedPtr<ProgramObject> m_currentProgram;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL