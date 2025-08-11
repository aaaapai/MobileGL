#include "ProgramState.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            Uint ProgramState::CreateProgram() {
                Uint programId = 0;
                m_programIndexGenerator.Generate(1, &programId);
                EnsureIndexAvail(programId, m_programObjects);
                auto programObject = MakeShared<ProgramObject>();
                if (programObject == nullptr)
                    return 0;
                m_programObjects[programId] = programObject;
                return programId;
            }

            SharedPtr<ProgramObject> ProgramState::GetProgramObject(const Uint id) {
                if (!CheckIndexAvail(id, m_programObjects)) return nullptr; // FIXME: add error reporting here
                return m_programObjects[id];
            }

            void ProgramState::MarkProgramObjectForDeletion(const Uint program) {
                if (!CheckIndexAvail(program, m_programObjects)) return; // FIXME: add error reporting here
                auto& programObject = m_programObjects[program];
                if (programObject != nullptr) {
                    programObject->MarkAsDeleted();
                    programObject.reset();
                    m_programIndexGenerator.Delete(program);
                }
            }

            Bool ProgramState::ValidateProgramObject(const Uint program) const {
                return CheckIndexAvail(program, m_programObjects) && m_programObjects[program] != nullptr;
            }

            Uint ProgramState::CreateShader(ShaderStage stage) {
                Uint shaderId = 0;
                m_shaderIndexGenerator.Generate(1, &shaderId);
                EnsureIndexAvail(shaderId, m_shaderObjects);
                auto shaderObject = MakeShared<ShaderObject>(stage);
                if (shaderObject == nullptr)
                    return 0;
                m_shaderObjects[shaderId] = shaderObject;
                return shaderId;
            }

            SharedPtr<ShaderObject> ProgramState::GetShaderObject(const Uint shader) {
                if (!CheckIndexAvail(shader, m_shaderObjects)) return nullptr;
                return m_shaderObjects[shader];
            }

            void ProgramState::MarkShaderObjectForDeletion(Uint shader) {
                if (!CheckIndexAvail(shader, m_shaderObjects)) return;
                auto& shaderObject = m_shaderObjects[shader];
                if (shaderObject != nullptr) {
                    m_shaderObjects[shader]->MarkAsDeleted();
                    m_shaderObjects[shader].reset();
                    m_shaderIndexGenerator.Delete(shader);
                }
            }

            Bool ProgramState::ValidateShaderObject(Uint shader) const {
                return CheckIndexAvail(shader, m_shaderObjects) && m_shaderObjects[shader] != nullptr;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL