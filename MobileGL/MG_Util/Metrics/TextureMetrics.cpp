#include "TextureMetrics.h"
#include "MG_Util/Math/VectorTypes.h"

namespace MobileGL {
    namespace MG_Util {
        SizeT GetTexturePixelSize(TextureInternalFormat internal) {
            switch (internal) {
            case TextureInternalFormat::R8:
            case TextureInternalFormat::R8Snorm:
            case TextureInternalFormat::R8I:
            case TextureInternalFormat::R8UI:
                return 1;

            case TextureInternalFormat::R16:
            case TextureInternalFormat::R16Snorm:
            case TextureInternalFormat::R16I:
            case TextureInternalFormat::R16UI:
            case TextureInternalFormat::R16F:
            case TextureInternalFormat::RG8:
            case TextureInternalFormat::RG8Snorm:
            case TextureInternalFormat::RG8I:
            case TextureInternalFormat::RG8UI:
                return 2;

            case TextureInternalFormat::R3G3B2:
            case TextureInternalFormat::RGB4:
            case TextureInternalFormat::RGB5:
            case TextureInternalFormat::RGB8:
            case TextureInternalFormat::RGB8Snorm:
            case TextureInternalFormat::SRGB8:
            case TextureInternalFormat::RGB8I:
            case TextureInternalFormat::RGB8UI:
                return 3;

            case TextureInternalFormat::RGBA2:
            case TextureInternalFormat::RGBA4:
            case TextureInternalFormat::RGB5A1:
            case TextureInternalFormat::RGBA8:
            case TextureInternalFormat::RGBA8Snorm:
            case TextureInternalFormat::RGBA8I:
            case TextureInternalFormat::RGBA8UI:
            case TextureInternalFormat::SRGB8Alpha8:
            case TextureInternalFormat::RGB10A2:
            case TextureInternalFormat::RGB10A2UI:
            case TextureInternalFormat::R32F:

            case TextureInternalFormat::RG16:
            case TextureInternalFormat::RG16Snorm:
            case TextureInternalFormat::RG16I:
            case TextureInternalFormat::RG16UI:
            case TextureInternalFormat::RG16F:
                return 4;

            case TextureInternalFormat::RGBA12:
            case TextureInternalFormat::RGBA16:
            case TextureInternalFormat::RGBA16I:
            case TextureInternalFormat::RGBA16UI:
            case TextureInternalFormat::RGBA16F:
            case TextureInternalFormat::RG32F:
            case TextureInternalFormat::RG32I:
            case TextureInternalFormat::RG32UI:
                return 8;

            case TextureInternalFormat::RGB16F:
            case TextureInternalFormat::RGB32F:
            case TextureInternalFormat::RGB16I:
            case TextureInternalFormat::RGB16UI:
            case TextureInternalFormat::RGB32I:
            case TextureInternalFormat::RGB32UI:
                return 12;

            case TextureInternalFormat::RGBA32F:
            case TextureInternalFormat::RGBA32I:
            case TextureInternalFormat::RGBA32UI:
                return 16;

            case TextureInternalFormat::R11FG11FB10F:
            case TextureInternalFormat::RGB9E5:
                return 4;

            default:
                return 0;
            }
        }

        SizeT GetTexturePixelDataTypeSize(TexturePixelDataType type) {
            switch (type) {
            case TexturePixelDataType::UnsignedByte:
            case TexturePixelDataType::Byte:
            case TexturePixelDataType::UnsignedByte332:
            case TexturePixelDataType::UnsignedByte233Rev:
                return 1;
            case TexturePixelDataType::UnsignedShort:
            case TexturePixelDataType::Short:
            case TexturePixelDataType::UnsignedShort565:
            case TexturePixelDataType::UnsignedShort565Rev:
            case TexturePixelDataType::UnsignedShort4444:
            case TexturePixelDataType::UnsignedShort4444Rev:
            case TexturePixelDataType::UnsignedShort5551:
            case TexturePixelDataType::UnsignedShort1555Rev:
                return 2;
            case TexturePixelDataType::UnsignedInt:
            case TexturePixelDataType::Int:
            case TexturePixelDataType::Float:
            case TexturePixelDataType::UnsignedInt8888:
            case TexturePixelDataType::UnsignedInt8888Rev:
            case TexturePixelDataType::UnsignedInt1010102:
            case TexturePixelDataType::UnsignedInt2101010Rev:
            case TexturePixelDataType::UnsignedInt101111Rev:
            case TexturePixelDataType::UnsignedInt5999Rev:
                return 4;
            default:
                return 0;
            }
        }

        SizeT CalculateTextureImageSize(TextureInternalFormat internalFormat, TexturePixelDataType pixelDataType,
                                        IntVec3 size) {
            SizeT pixelSize = GetTexturePixelSize(internalFormat);
            if (pixelSize == 0) return 0;

            SizeT dataTypeSize = GetTexturePixelDataTypeSize(pixelDataType);
            if (dataTypeSize == 0) return 0;

            return pixelSize * dataTypeSize * size.x() * size.y() * size.z();
        }

    } // namespace MG_Util
} // namespace MobileGL
