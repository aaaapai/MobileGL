// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureObjectBuffer.h
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include "TextureObject.h"
#include "MG_State/GLState/BufferState/BufferObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class TextureObjectBuffer : public TextureObjectBase {
            public:
                TextureStorageType GetStorageType() const override { return TextureStorageType::Buffer; }
                explicit TextureObjectBuffer(Uint externalIndex);
                const Vector<TextureUploadTarget>& GetUploadTargets() const override { return m_uploadTargets; }
                BindingSlot<BufferObject>& GetBufferBindingSlot(TextureUploadTarget target = TextureUploadTarget::TextureBuffer);

            protected:
                Uint GetIndexOfTextureUploadTarget(TextureUploadTarget target) const override;

                BindingSlot<BufferObject> m_bufferBindingSlot = BindingSlot<BufferObject>(BufferTarget::Texture);
                const Vector<TextureUploadTarget> m_uploadTargets{TextureUploadTarget::TextureBuffer};
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
