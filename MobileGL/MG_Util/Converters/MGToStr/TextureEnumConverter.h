// MobileGL - MobileGL/MG_Util/Converters/MGToStr/TextureEnumConverter.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

namespace MobileGL {
    namespace MG_Util {
        String ConvertTextureTargetToString(TextureTarget target);
        String ConvertTextureInputFormatToString(TextureInputFormat format);
        String ConvertTextureInternalFormatToString(TextureInternalFormat internalformat);
        String ConvertTexturePixelDataTypeToString(TexturePixelDataType type);
        String ConvertTextureUploadTargetToString(TextureUploadTarget target);
        String ConvertSamplerFilterModeToString(SamplerFilterMode value);
        String ConvertSamplerMipmapModeToString(SamplerMipmapMode value);
        String ConvertSamplerWrapModeToString(SamplerWrapMode value);
        String ConvertSamplerCompareModeToString(SamplerCompareMode value);
        String ConvertSamplerCompareFuncToString(SamplerCompareFunc value);
    } // namespace MG_Util
} // namespace MobileGL