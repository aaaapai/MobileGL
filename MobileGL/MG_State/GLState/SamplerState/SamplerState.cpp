// MobileGL - MobileGL/MG_State/GLState/SamplerState/SamplerState.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
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
