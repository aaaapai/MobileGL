#include "TextureObjectBuffer.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            TextureObjectBuffer::TextureObjectBuffer(Uint externalIndex)
                : TextureObjectBase(TextureTarget::TextureBuffer, externalIndex) {}

            Uint TextureObjectBuffer::GetIndexOfTextureUploadTarget(TextureUploadTarget target) const {
                MOBILEGL_ASSERT(target == TextureUploadTarget::TextureBuffer, "Invalid TextureUploadTarget!");
                return 0;
            }

            BindingSlot<BufferObject>& TextureObjectBuffer::GetBufferBindingSlot(TextureUploadTarget target) {
                MOBILEGL_ASSERT(target == TextureUploadTarget::TextureBuffer, "Invalid TextureUploadTarget!");
                return m_bufferBindingSlot;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL