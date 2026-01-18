// MobileGL - MobileGL/MG_Util/Converters/MGToStr/TextureEnumConverter.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
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