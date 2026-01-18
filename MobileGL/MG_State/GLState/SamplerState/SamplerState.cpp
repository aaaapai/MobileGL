// MobileGL - MobileGL/MG_State/GLState/SamplerState/SamplerState.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "SamplerState.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            SamplerState::SamplerState() : m_indexGenerator(1024, 1) {}

            Vector<Uint> SamplerState::GenerateNames(Uint number) {
                Vector<Uint> names(number);
                m_indexGenerator.Generate(number, names.data());
                return names;
            }

            SharedPtr<SamplerObject> SamplerState::GetSamplerObject(Uint index) {
                auto it = m_samplerObjects.find(index);
                return it != m_samplerObjects.end() ? it->second : nullptr;
            }

            SharedPtr<SamplerObject> SamplerState::CreateSamplerObject(Uint index) {
                auto sampler = MakeShared<SamplerObject>(index);
                m_samplerObjects[index] = sampler;
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
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
