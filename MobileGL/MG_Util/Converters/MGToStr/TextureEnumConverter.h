#pragma once
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

namespace MobileGL {
    namespace MG_Util {
        String ConvertTextureTargetToString(TextureTarget target);
        String ConvertTextureInputFormatToString(TextureInputFormat format);
        String ConvertTextureSizedInternalFormatToString(TextureSizedInternalFormat internalformat);
        String ConvertTexturePixelDataTypeToString(TexturePixelDataType type);
        String ConvertTextureUploadTargetToString(TextureUploadTarget target);
    } // namespace MG_Util
} // namespace MobileGL