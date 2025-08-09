#include "../../../Includes.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            Uint ProgramState::CreateProgram() {
                Uint programId = 0;
                m_indexGenerator.Generate(1, &programId);
                EnsureIndexAvail(programId);
                m_programObjects[programId] = MakeShared<ProgramObject>();
                return programId;
            }

            SharedPtr<ProgramObject> ProgramState::GetProgramObject(Uint id) {
                if (!CheckIndexAvail(id))
                    return nullptr; // FIXME: add error reporting here
                return m_programObjects[id];
            }

            void ProgramState::DeleteProgram(Uint id) {
                if (!CheckIndexAvail(id))
                    return; // FIXME: add error reporting here
                m_programObjects[id].reset();
            }
        }
    }
}