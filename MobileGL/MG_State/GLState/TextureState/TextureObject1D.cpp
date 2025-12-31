// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureObject1D.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "TextureObject1D.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            TextureObject1D::TextureObject1D(Uint externalIndex)
                : TextureObjectWithOneMipmap(TextureTarget::Texture1D, externalIndex) {}

            Uint TextureObject1D::GetIndexOfTextureUploadTarget(TextureUploadTarget target) const {
                MOBILEGL_ASSERT(target == TextureUploadTarget::Texture1D ||
                                    target == TextureUploadTarget::ProxyTexture1D,
                                "Invalid TextureUploadTarget!");
                return 0;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL