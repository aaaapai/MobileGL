#pragma once
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

namespace MobileGL {
    namespace MG_Util {
        SizeT GetSizedInternalFormatSizeInBytes(TextureInternalFormat internal);
        SizeT GetBaseInternalFormatComponentCount(TextureInternalFormat format);
        SizeT GetSizedTexturePixelDataTypeSize(TexturePixelDataType type);
        SizeT GetBaseTexturePixelDataTypeSize(TexturePixelDataType type);
        // This should respect internal format more
        SizeT GetInternalBytesPerPixel(TextureInternalFormat internalformat, TexturePixelDataType type);
        // This should respect type more, representing data passed in
        SizeT GetInputBytesPerPixel(TextureInternalFormat internalformat, TexturePixelDataType type);
        SizeT CalculateInputTextureImageSize(TextureInternalFormat internalFormat, TexturePixelDataType pixelDataType,
                                             IntVec3 size);
        ComponentSizes GetComponentSizesForInternalFormat(TextureInternalFormat internal);
    } // namespace MG_Util
} // namespace MobileGL
