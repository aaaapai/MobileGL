// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureObject3D.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "TextureObject3D.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            TextureObject3D::TextureObject3D(Uint externalIndex)
                : TextureObjectWithOneMipmap(TextureTarget::Texture3D, externalIndex) {}

            Uint TextureObject3D::GetIndexOfTextureUploadTarget(TextureUploadTarget target) const {
                MOBILEGL_ASSERT(target == TextureUploadTarget::Texture3D ||
                                    target == TextureUploadTarget::ProxyTexture3D,
                                "Invalid TextureUploadTarget!");
                return 0;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL