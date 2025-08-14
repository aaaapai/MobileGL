#pragma once
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

namespace MobileGL {
    namespace MG_Util {
        SizeT GetTexturePixelSize(TextureInternalFormat internal);
        SizeT GetTexturePixelDataTypeSize(TexturePixelDataType type);
        SizeT CalculateTextureImageSize(TextureInternalFormat internalFormat, TexturePixelDataType pixelDataType,
                                        IntVec3 size);
    } // namespace MG_Util
} // namespace MobileGL
