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
                void DeleteProgram(Uint program);

            private:
                bool CheckIndexAvail(SizeT idx) { return idx < m_programObjects.size(); }

                void EnsureIndexAvail(SizeT idx) {
                    if (CheckIndexAvail(idx)) return;

                    m_programObjects.reserve(std::bit_ceil(idx));
                    m_programObjects.resize(idx + 1);
                }

                IndexGenerator<Uint> m_indexGenerator;
                Vector<SharedPtr<ProgramObject>> m_programObjects;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL