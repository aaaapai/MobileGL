// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureObject2D.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include "TextureObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class TextureObject2D : public TextureObjectWithOneMipmap {
            public:
                explicit TextureObject2D(Uint externalIndex);
                const Vector<TextureUploadTarget>& GetUploadTargets() const override { return m_uploadTargets; }

            protected:
                Uint GetIndexOfTextureUploadTarget(TextureUploadTarget target) const override;
                const Vector<TextureUploadTarget> m_uploadTargets{TextureUploadTarget::Texture2D};
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
