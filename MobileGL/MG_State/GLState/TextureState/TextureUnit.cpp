// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureUnit.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "TextureUnit.h"

namespace MobileGL::MG_State::GLState {
    TextureUnit::TextureUnit() : m_sampler(nullptr) {
        for (int i = 0; i < (int)TextureTarget::TextureTargetCount; ++i) {
            m_slots[i] = BindingSlot<ITextureObject>(static_cast<TextureTarget>(i));
        }
    }

    BindingSlot<ITextureObject>& TextureUnit::GetBindingSlot(TextureTarget target) {
        return m_slots[(int)target];
    }

    Array<BindingSlot<ITextureObject>, (int)TextureTarget::TextureTargetCount>& TextureUnit::GetAllBindingSlots() {
        return m_slots;
    }

    void TextureUnit::SetSamplerObject(const SharedPtr<SamplerObject>& sampler) {
        m_sampler = sampler;
    }

    const SharedPtr<SamplerObject>& TextureUnit::GetSamplerObject() const {
        return m_sampler;
    }
} // namespace MobileGL::MG_State::GLState
