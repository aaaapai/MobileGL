// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureObject2D.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "TextureObject2D.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            TextureObject2D::TextureObject2D(Uint externalIndex)
                : TextureObjectWithOneMipmap(TextureTarget::Texture2D, externalIndex) {}

            Uint TextureObject2D::GetIndexOfTextureUploadTarget(TextureUploadTarget target) const {
                MOBILEGL_ASSERT(target == TextureUploadTarget::Texture2D ||
                                    target == TextureUploadTarget::ProxyTexture2D,
                                "Invalid TextureUploadTarget!");
                return 0;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL