#pragma once
#include <Includes.h>
#include "MG_State/GLState/TextureState/SamplerObject.h"
#include "MG_Util/Types.h"
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

            private:
                Array<BindingSlot<ITextureObject>, (int)TextureTarget::TextureTargetCount> m_slots;
                SharedPtr<SamplerObject> m_sampler;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL