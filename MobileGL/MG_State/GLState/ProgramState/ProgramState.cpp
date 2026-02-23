// MobileGL - MobileGL/MG_State/GLState/ProgramState/ProgramState.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "ProgramState.h"

namespace MobileGL::MG_State::GLState {
    Uint ProgramState::CreateProgram() {
        Uint programId = 0;
        m_programIndexGenerator.Generate(1, &programId);
        EnsureIndexAvail(programId, m_programObjects);
        auto programObject = MakeShared<ProgramObject>(programId);
        if (programObject == nullptr) return 0;
        m_programObjects[programId] = programObject;
        return programId;
    }

    const SharedPtr<ProgramObject>& ProgramState::GetProgramObject(const Uint id) {
        static SharedPtr<ProgramObject> nullProgramObject = nullptr;
        if (!CheckIndexAvail(id, m_programObjects)) return nullProgramObject; // FIXME: add error reporting here
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

    void ProgramState::UseProgram(Uint program) {
        if (program == 0) m_currentProgram.reset();

        if (!CheckIndexAvail(program, m_programObjects)) return;
        m_currentProgram = m_programObjects[program];
    }

    Uint ProgramState::CreateShader(ShaderStage stage) {
        Uint shaderId = 0;
        m_shaderIndexGenerator.Generate(1, &shaderId);
        EnsureIndexAvail(shaderId, m_shaderObjects);
        auto shaderObject = MakeShared<ShaderObject>(stage, shaderId);
        if (shaderObject == nullptr) return 0;
        m_shaderObjects[shaderId] = shaderObject;
        return shaderId;
    }

    const SharedPtr<ShaderObject>& ProgramState::GetShaderObject(const Uint shader) {
        static SharedPtr<ShaderObject> nullShaderObject = nullptr;
        if (!CheckIndexAvail(shader, m_shaderObjects)) return nullShaderObject;
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
} // namespace MobileGL::MG_State::GLState
