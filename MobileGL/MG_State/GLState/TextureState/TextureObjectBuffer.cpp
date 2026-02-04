// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureObjectBuffer.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
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