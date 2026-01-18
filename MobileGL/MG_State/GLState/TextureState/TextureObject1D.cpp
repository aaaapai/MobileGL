// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureObject1D.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
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