// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureObject3D.cpp
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
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