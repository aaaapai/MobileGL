// MobileGL - MobileGL/MG_Util/Converters/MGToVk/TextureEnumConverter.h
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
        VkImageViewType ConvertTextureTargetToVkEnum(TextureTarget target);
        VkFormat ConvertTextureInputFormatToVkEnum(TextureInputFormat format);
        VkFormat ConvertTextureInternalFormatToVkEnum(TextureInternalFormat internalformat);
        VkFormat ConvertTexturePixelDataTypeToVkEnum(TexturePixelDataType type);
        VkImageViewType ConvertTextureUploadTargetToVkEnum(TextureUploadTarget target);
        VkFilter ConvertSamplerFilterModeToVkEnum(SamplerFilterMode filter, SamplerMipmapMode mipmap);
        VkSamplerMipmapMode ConvertSamplerMipmapModeToVkEnum(SamplerMipmapMode value);
        VkSamplerAddressMode ConvertSamplerWrapModeToVkEnum(SamplerWrapMode value);
        VkBool32 ConvertSamplerCompareModeToVkEnum(SamplerCompareMode value);
        VkCompareOp ConvertSamplerCompareFuncToVkEnum(SamplerCompareFunc value);
        VkComponentSwizzle ConvertTextureSwizzleParamToVkEnum(TextureSwizzleParam value);
    } // namespace MG_Util
} // namespace MobileGL
