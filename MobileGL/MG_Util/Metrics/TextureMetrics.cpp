#include "TextureMetrics.h"

namespace MobileGL {
    namespace MG_Util {
        SizeT GetTexturePixelSize(TextureSizedInternalFormat internal) {
            switch (internal) {
            case TextureSizedInternalFormat::R8:
            case TextureSizedInternalFormat::R8Snorm:
            case TextureSizedInternalFormat::R8I:
            case TextureSizedInternalFormat::R8UI:
                return 1;

            case TextureSizedInternalFormat::R16:
            case TextureSizedInternalFormat::R16Snorm:
            case TextureSizedInternalFormat::R16I:
            case TextureSizedInternalFormat::R16UI:
            case TextureSizedInternalFormat::R16F:
            case TextureSizedInternalFormat::RG8:
            case TextureSizedInternalFormat::RG8Snorm:
            case TextureSizedInternalFormat::RG8I:
            case TextureSizedInternalFormat::RG8UI:
                return 2;

            case TextureSizedInternalFormat::R3G3B2:
            case TextureSizedInternalFormat::RGB4:
            case TextureSizedInternalFormat::RGB5:
            case TextureSizedInternalFormat::RGB8:
            case TextureSizedInternalFormat::RGB8Snorm:
            case TextureSizedInternalFormat::SRGB8:
            case TextureSizedInternalFormat::RGB8I:
            case TextureSizedInternalFormat::RGB8UI:
                return 3;

            case TextureSizedInternalFormat::RGBA2:
            case TextureSizedInternalFormat::RGBA4:
            case TextureSizedInternalFormat::RGB5A1:
            case TextureSizedInternalFormat::RGBA8:
            case TextureSizedInternalFormat::RGBA8Snorm:
            case TextureSizedInternalFormat::RGBA8I:
            case TextureSizedInternalFormat::RGBA8UI:
            case TextureSizedInternalFormat::SRGB8Alpha8:
            case TextureSizedInternalFormat::RGB10A2:
            case TextureSizedInternalFormat::RGB10A2UI:
            case TextureSizedInternalFormat::R32F:

            case TextureSizedInternalFormat::RG16:
            case TextureSizedInternalFormat::RG16Snorm:
            case TextureSizedInternalFormat::RG16I:
            case TextureSizedInternalFormat::RG16UI:
            case TextureSizedInternalFormat::RG16F:
                return 4;

            case TextureSizedInternalFormat::RGBA12:
            case TextureSizedInternalFormat::RGBA16:
            case TextureSizedInternalFormat::RGBA16I:
            case TextureSizedInternalFormat::RGBA16UI:
            case TextureSizedInternalFormat::RGBA16F:
            case TextureSizedInternalFormat::RG32F:
            case TextureSizedInternalFormat::RG32I:
            case TextureSizedInternalFormat::RG32UI:
                return 8;

            case TextureSizedInternalFormat::RGB16F:
            case TextureSizedInternalFormat::RGB32F:
            case TextureSizedInternalFormat::RGB16I:
            case TextureSizedInternalFormat::RGB16UI:
            case TextureSizedInternalFormat::RGB32I:
            case TextureSizedInternalFormat::RGB32UI:
                return 12;

            case TextureSizedInternalFormat::RGBA32F:
            case TextureSizedInternalFormat::RGBA32I:
            case TextureSizedInternalFormat::RGBA32UI:
                return 16;

            case TextureSizedInternalFormat::R11FG11FB10F:
            case TextureSizedInternalFormat::RGB9E5:
                return 4;

            default:
                return 0;
            }
        }
    } // namespace MG_Util
} // namespace MobileGL
