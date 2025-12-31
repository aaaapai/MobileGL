// MobileGL - MobileGL/MG_Util/Converters/MGToMG/TextureEnumConverter.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "TextureEnumConverter.h"

namespace MobileGL {
    namespace MG_Util {
        TextureTarget ConvertTextureUploadTargetToTextureTarget(TextureUploadTarget target) {
            switch (target) {
            case TextureUploadTarget::Texture2D:
            case TextureUploadTarget::ProxyTexture2D:
                return TextureTarget::Texture2D;
            case TextureUploadTarget::Texture1DArray:
            case TextureUploadTarget::ProxyTexture1DArray:
                return TextureTarget::Texture1DArray;
            case TextureUploadTarget::TextureRectangle:
            case TextureUploadTarget::ProxyTextureRectangle:
                return TextureTarget::TextureRectangle;
            case TextureUploadTarget::CubeMapPositiveX:
            case TextureUploadTarget::CubeMapNegativeX:
            case TextureUploadTarget::CubeMapPositiveY:
            case TextureUploadTarget::CubeMapNegativeY:
            case TextureUploadTarget::CubeMapPositiveZ:
            case TextureUploadTarget::CubeMapNegativeZ:
            case TextureUploadTarget::ProxyCubeMap:
                return TextureTarget::TextureCubeMap;
            case TextureUploadTarget::CubeMapArray:
            case TextureUploadTarget::ProxyCubeMapArray:
                return TextureTarget::TextureCubeMapArray;
            case TextureUploadTarget::Texture2DMultisample:
            case TextureUploadTarget::ProxyTexture2DMultisample:
                return TextureTarget::Texture2DMultisample;
            case TextureUploadTarget::Texture2DMultisampleArray:
            case TextureUploadTarget::ProxyTexture2DMultisampleArray:
                return TextureTarget::Texture2DMultisampleArray;
            case TextureUploadTarget::Texture3D:
            case TextureUploadTarget::ProxyTexture3D:
                return TextureTarget::Texture3D;
            case TextureUploadTarget::Texture1D:
            case TextureUploadTarget::ProxyTexture1D:
                return TextureTarget::Texture1D;
            case TextureUploadTarget::Texture2DArray:
            case TextureUploadTarget::ProxyTexture2DArray:
                return TextureTarget::Texture2DArray;
            default:
                return TextureTarget::Unknown;
            }
        }
    } // namespace MG_Util
} // namespace MobileGL