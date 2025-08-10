#include "TextureUnit.h"
#include "MG_Util/Types.h"

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
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL