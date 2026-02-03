// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureObjectBuffer.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
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
                BindingSlot<BufferObject>& GetBufferBindingSlot(
                    TextureUploadTarget target = TextureUploadTarget::TextureBuffer);
                void ClearAllStorageDirtyBit() override;
                void SetStorageDirtyBit();

            protected:
                Uint GetIndexOfTextureUploadTarget(TextureUploadTarget target) const override;

                BindingSlot<BufferObject> m_bufferBindingSlot = BindingSlot<BufferObject>(BufferTarget::Texture);
                const Vector<TextureUploadTarget> m_uploadTargets{TextureUploadTarget::TextureBuffer};
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
