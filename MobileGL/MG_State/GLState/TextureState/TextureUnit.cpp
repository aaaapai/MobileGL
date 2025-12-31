// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureUnit.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "TextureUnit.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            TextureUnit::TextureUnit() : m_sampler(nullptr) {
                for (int i = 0; i < (int)TextureTarget::TextureTargetCount; ++i) {
                    m_slots[i] = BindingSlot<ITextureObject>(static_cast<TextureTarget>(i));
                }
            }

            BindingSlot<ITextureObject>& TextureUnit::GetBindingSlot(TextureTarget target) {
                return m_slots[(int)target];
            }

            Array<BindingSlot<ITextureObject>, (int)TextureTarget::TextureTargetCount>& TextureUnit::
                GetAllBindingSlots() {
                return m_slots;
            }

            void TextureUnit::SetSamplerObject(SharedPtr<SamplerObject> sampler) {
                m_sampler = sampler;
            }

            SharedPtr<SamplerObject> TextureUnit::GetSamplerObject() const {
                return m_sampler;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL