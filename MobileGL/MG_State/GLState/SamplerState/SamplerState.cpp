// MobileGL - MobileGL/MG_State/GLState/SamplerState/SamplerState.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "SamplerState.h"

namespace MobileGL::MG_State::GLState {
    SamplerState::SamplerState() : m_indexGenerator(1024, 1) {}

    void SamplerState::GenerateNames(Uint number, Vector<Uint>& samplers) {
        samplers.resize(number);
        m_indexGenerator.Generate(number, samplers.data());
    }

    const SharedPtr<SamplerObject>& SamplerState::GetSamplerObject(Uint index) {
        auto it = m_samplerObjects.find(index);
        static SharedPtr<SamplerObject> nullSamplerObject = nullptr;
        return it != m_samplerObjects.end() ? it->second : nullSamplerObject;
    }

    const SharedPtr<SamplerObject>& SamplerState::CreateSamplerObject(Uint index) {
        auto& sampler = m_samplerObjects[index];
        sampler = MakeShared<SamplerObject>(index);
        return sampler;
    }

    void SamplerState::MarkSamplerObjectForDeletion(Uint index) {
        if (m_indexGenerator.IsValid(index)) {
            m_samplerObjects.erase(index);
            m_indexGenerator.Delete(index);
        }
    }

    Bool SamplerState::ValidateName(Uint index) const {
        return m_indexGenerator.IsValid(index);
    }

    Bool SamplerState::ValidateSamplerObject(Uint index) const {
        return m_samplerObjects.find(index) != m_samplerObjects.end();
    }
} // namespace MobileGL::MG_State::GLState
