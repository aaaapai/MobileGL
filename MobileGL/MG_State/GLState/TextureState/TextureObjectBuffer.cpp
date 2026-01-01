// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureObjectBuffer.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

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