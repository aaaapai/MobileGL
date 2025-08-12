#include "TextureEnumConverter.h"
#include "MG_Util/Types.h"

namespace MobileGL {
    namespace MG_Util {
        String ConvertTextureTargetToString(TextureTarget target) {
            switch (target) {
            case TextureTarget::Texture1D:
                return "Texture1D";
            case TextureTarget::Texture2D:
                return "Texture2D";
            case TextureTarget::Texture3D:
                return "Texture3D";
            case TextureTarget::TextureCubeMap:
                return "TextureCubeMap";
            case TextureTarget::Texture2DArray:
                return "Texture2DArray";
            case TextureTarget::Texture2DMultisample:
                return "Texture2DMultisample";
            case TextureTarget::TextureCubeMapArray:
                return "TextureCubeMapArray";
            default:
                return "Unknown";
            }
        }

        String ConvertTextureInputFormatToString(TextureInputFormat format) {
            switch (format) {
            case TextureInputFormat::Red:
                return "Red";
            case TextureInputFormat::RG:
                return "RG";
            case TextureInputFormat::RGB:
                return "RGB";
            case TextureInputFormat::BGR:
                return "BGR";
            case TextureInputFormat::RGBA:
                return "RGBA";
            case TextureInputFormat::BGRA:
                return "BGRA";
            case TextureInputFormat::RInteger:
                return "RInteger";
            case TextureInputFormat::RGInteger:
                return "RGInteger";
            case TextureInputFormat::RGBInteger:
                return "RGBInteger";
            case TextureInputFormat::BGRInteger:
                return "BGRInteger";
            case TextureInputFormat::RGBAInteger:
                return "RGBAInteger";
            case TextureInputFormat::BGRAInteger:
                return "BGRAInteger";
            case TextureInputFormat::StencilIndex:
                return "StencilIndex";
            case TextureInputFormat::DepthComponent:
                return "DepthComponent";
            case TextureInputFormat::DepthStencil:
                return "DepthStencil";
            default:
                return "Unknown";
            }
        }

        String ConvertTextureSizedInternalFormatToString(TextureSizedInternalFormat format) {
            switch (format) {
            case TextureSizedInternalFormat::R8:
                return "R8";
            case TextureSizedInternalFormat::R8Snorm:
                return "R8Snorm";
            case TextureSizedInternalFormat::R16:
                return "R16";
            case TextureSizedInternalFormat::R16Snorm:
                return "R16Snorm";
            case TextureSizedInternalFormat::RG8:
                return "RG8";
            case TextureSizedInternalFormat::RG8Snorm:
                return "RG8Snorm";
            case TextureSizedInternalFormat::RG16:
                return "RG16";
            case TextureSizedInternalFormat::RG16Snorm:
                return "RG16Snorm";
            case TextureSizedInternalFormat::R3G3B2:
                return "R3G3B2";
            case TextureSizedInternalFormat::RGB4:
                return "RGB4";
            case TextureSizedInternalFormat::RGB5:
                return "RGB5";
            case TextureSizedInternalFormat::RGB8:
                return "RGB8";
            case TextureSizedInternalFormat::RGB8Snorm:
                return "RGB8Snorm";
            case TextureSizedInternalFormat::RGB10:
                return "RGB10";
            case TextureSizedInternalFormat::RGB12:
                return "RGB12";
            case TextureSizedInternalFormat::RGB16Snorm:
                return "RGB16Snorm";
            case TextureSizedInternalFormat::RGBA2:
                return "RGBA2";
            case TextureSizedInternalFormat::RGBA4:
                return "RGBA4";
            case TextureSizedInternalFormat::RGB5A1:
                return "RGB5A1";
            case TextureSizedInternalFormat::RGBA8:
                return "RGBA8";
            case TextureSizedInternalFormat::RGBA8Snorm:
                return "RGBA8Snorm";
            case TextureSizedInternalFormat::RGB10A2:
                return "RGB10A2";
            case TextureSizedInternalFormat::RGB10A2UI:
                return "RGB10A2UI";
            case TextureSizedInternalFormat::RGBA12:
                return "RGBA12";
            case TextureSizedInternalFormat::RGBA16:
                return "RGBA16";
            case TextureSizedInternalFormat::SRGB8:
                return "SRGB8";
            case TextureSizedInternalFormat::SRGB8Alpha8:
                return "SRGB8Alpha8";
            case TextureSizedInternalFormat::R16F:
                return "R16F";
            case TextureSizedInternalFormat::RG16F:
                return "RG16F";
            case TextureSizedInternalFormat::RGB16F:
                return "RGB16F";
            case TextureSizedInternalFormat::RGBA16F:
                return "RGBA16F";
            case TextureSizedInternalFormat::R32F:
                return "R32F";
            case TextureSizedInternalFormat::RG32F:
                return "RG32F";
            case TextureSizedInternalFormat::RGB32F:
                return "RGB32F";
            case TextureSizedInternalFormat::RGBA32F:
                return "RGBA32F";
            case TextureSizedInternalFormat::R11FG11FB10F:
                return "R11FG11FB10F";
            case TextureSizedInternalFormat::RGB9E5:
                return "RGB9E5";
            case TextureSizedInternalFormat::R8I:
                return "R8I";
            case TextureSizedInternalFormat::R8UI:
                return "R8UI";
            case TextureSizedInternalFormat::R16I:
                return "R16I";
            case TextureSizedInternalFormat::R16UI:
                return "R16UI";
            case TextureSizedInternalFormat::R32I:
                return "R32I";
            case TextureSizedInternalFormat::R32UI:
                return "R32UI";
            case TextureSizedInternalFormat::RG8I:
                return "RG8I";
            case TextureSizedInternalFormat::RG8UI:
                return "RG8UI";
            case TextureSizedInternalFormat::RG16I:
                return "RG16I";
            case TextureSizedInternalFormat::RG16UI:
                return "RG16UI";
            case TextureSizedInternalFormat::RG32I:
                return "RG32I";
            case TextureSizedInternalFormat::RG32UI:
                return "RG32UI";
            case TextureSizedInternalFormat::RGB8I:
                return "RGB8I";
            case TextureSizedInternalFormat::RGB8UI:
                return "RGB8UI";
            case TextureSizedInternalFormat::RGB16I:
                return "RGB16I";
            case TextureSizedInternalFormat::RGB16UI:
                return "RGB16UI";
            case TextureSizedInternalFormat::RGB32I:
                return "RGB32I";
            case TextureSizedInternalFormat::RGB32UI:
                return "RGB32UI";
            case TextureSizedInternalFormat::RGBA8I:
                return "RGBA8I";
            case TextureSizedInternalFormat::RGBA8UI:
                return "RGBA8UI";
            case TextureSizedInternalFormat::RGBA16I:
                return "RGBA16I";
            case TextureSizedInternalFormat::RGBA16UI:
                return "RGBA16UI";
            case TextureSizedInternalFormat::RGBA32I:
                return "RGBA32I";
            case TextureSizedInternalFormat::RGBA32UI:
                return "RGBA32UI";
            default:
                return "Unknown";
            }
        }

        String ConvertTexturePixelDataTypeToString(TexturePixelDataType type) {
            switch (type) {
            case TexturePixelDataType::UnsignedByte:
                return "UnsignedByte";
            case TexturePixelDataType::Byte:
                return "Byte";
            case TexturePixelDataType::UnsignedShort:
                return "UnsignedShort";
            case TexturePixelDataType::Short:
                return "Short";
            case TexturePixelDataType::UnsignedInt:
                return "UnsignedInt";
            case TexturePixelDataType::Int:
                return "Int";
            case TexturePixelDataType::Float:
                return "Float";
            case TexturePixelDataType::UnsignedByte332:
                return "UnsignedByte332";
            case TexturePixelDataType::UnsignedByte233Rev:
                return "UnsignedByte233Rev";
            case TexturePixelDataType::UnsignedShort565:
                return "UnsignedShort565";
            case TexturePixelDataType::UnsignedShort565Rev:
                return "UnsignedShort565Rev";
            case TexturePixelDataType::UnsignedShort4444:
                return "UnsignedShort4444";
            case TexturePixelDataType::UnsignedShort4444Rev:
                return "UnsignedShort4444Rev";
            case TexturePixelDataType::UnsignedShort5551:
                return "UnsignedShort5551";
            case TexturePixelDataType::UnsignedShort1555Rev:
                return "UnsignedShort1555Rev";
            default:
                return "Unknown";
            }
        }
    } // namespace MG_Util
} // namespace MobileGL