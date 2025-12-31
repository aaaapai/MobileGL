// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureUnit.h
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
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