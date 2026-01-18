// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureUnit.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_State/GLState/SamplerState/SamplerObject.h>
#include "TextureObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class TextureUnit {
            public:
                TextureUnit();
                BindingSlot<ITextureObject>& GetBindingSlot(TextureTarget target);
                SharedPtr<SamplerObject> GetSamplerObject() const;
                Array<BindingSlot<ITextureObject>, (int)TextureTarget::TextureTargetCount>& GetAllBindingSlots();
                void SetSamplerObject(SharedPtr<SamplerObject> sampler);

            private:
                Array<BindingSlot<ITextureObject>, (int)TextureTarget::TextureTargetCount> m_slots;
                SharedPtr<SamplerObject> m_sampler;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL