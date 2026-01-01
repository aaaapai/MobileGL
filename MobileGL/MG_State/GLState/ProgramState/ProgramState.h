// MobileGL - MobileGL/MG_State/GLState/ProgramState/ProgramState.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

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

                SharedPtr<ProgramObject> GetCurrentProgram() const { return m_currentProgram; }

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