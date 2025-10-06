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