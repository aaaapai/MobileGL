#pragma once
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

namespace MobileGL {
    namespace MG_Util {
        String ConvertTextureTargetToString(TextureTarget target);
        String ConvertTextureInputFormatToString(TextureInputFormat format);
        String ConvertTextureSizedInternalFormatToString(TextureSizedInternalFormat internalformat);
        String ConvertTexturePixelDataTypeToString(TexturePixelDataType type);
    } // namespace MG_Util
} // namespace MobileGL