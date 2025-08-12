#include "TextureEnumConverter.h"

namespace MobileGL {
    namespace MG_Util {
        GLenum ConvertTextureTargetToGLEnum(TextureTarget target) {
            switch (target) {
            case TextureTarget::Texture1D:
                return GL_TEXTURE_1D;
            case TextureTarget::Texture2D:
                return GL_TEXTURE_2D;
            case TextureTarget::Texture3D:
                return GL_TEXTURE_3D;
            case TextureTarget::TextureCubeMap:
                return GL_TEXTURE_CUBE_MAP;
            case TextureTarget::Texture2DArray:
                return GL_TEXTURE_2D_ARRAY;
            case TextureTarget::Texture2DMultisample:
                return GL_TEXTURE_2D_MULTISAMPLE;
            case TextureTarget::TextureCubeMapArray:
                return GL_TEXTURE_CUBE_MAP_ARRAY;
            default:
                return GL_UNKNOWN_MGL;
            }
        }

        GLenum ConvertTextureInputFormatToGLEnum(TextureInputFormat format) {
            switch (format) {
            case TextureInputFormat::Red:
                return GL_RED;
            case TextureInputFormat::RG:
                return GL_RG;
            case TextureInputFormat::RGB:
                return GL_RGB;
            case TextureInputFormat::BGR:
                return GL_BGR;
            case TextureInputFormat::RGBA:
                return GL_RGBA;
            case TextureInputFormat::BGRA:
                return GL_BGRA;
            case TextureInputFormat::RInteger:
                return GL_RED_INTEGER;
            case TextureInputFormat::RGInteger:
                return GL_RG_INTEGER;
            case TextureInputFormat::RGBInteger:
                return GL_RGB_INTEGER;
            case TextureInputFormat::BGRInteger:
                return GL_BGR_INTEGER;
            case TextureInputFormat::RGBAInteger:
                return GL_RGBA_INTEGER;
            case TextureInputFormat::BGRAInteger:
                return GL_BGRA_INTEGER;
            case TextureInputFormat::StencilIndex:
                return GL_STENCIL_INDEX;
            case TextureInputFormat::DepthComponent:
                return GL_DEPTH_COMPONENT;
            case TextureInputFormat::DepthStencil:
                return GL_DEPTH_STENCIL;
            default:
                return GL_UNKNOWN_MGL;
            }
        }

        GLenum ConvertTextureSizedInternalFormatToGLEnum(TextureSizedInternalFormat format) {
            switch (format) {
            case TextureSizedInternalFormat::R8:
                return GL_R8;
            case TextureSizedInternalFormat::R8Snorm:
                return GL_R8_SNORM;
            case TextureSizedInternalFormat::R16:
                return GL_R16;
            case TextureSizedInternalFormat::R16Snorm:
                return GL_R16_SNORM;
            case TextureSizedInternalFormat::RG8:
                return GL_RG8;
            case TextureSizedInternalFormat::RG8Snorm:
                return GL_RG8_SNORM;
            case TextureSizedInternalFormat::RG16:
                return GL_RG16;
            case TextureSizedInternalFormat::RG16Snorm:
                return GL_RG16_SNORM;
            case TextureSizedInternalFormat::R3G3B2:
                return GL_R3_G3_B2;
            case TextureSizedInternalFormat::RGB4:
                return GL_RGB4;
            case TextureSizedInternalFormat::RGB5:
                return GL_RGB5;
            case TextureSizedInternalFormat::RGB8:
                return GL_RGB8;
            case TextureSizedInternalFormat::RGB8Snorm:
                return GL_RGB8_SNORM;
            case TextureSizedInternalFormat::RGB10:
                return GL_RGB10;
            case TextureSizedInternalFormat::RGB12:
                return GL_RGB12;
            case TextureSizedInternalFormat::RGB16Snorm:
                return GL_RGB16_SNORM;
            case TextureSizedInternalFormat::RGBA2:
                return GL_RGBA2;
            case TextureSizedInternalFormat::RGBA4:
                return GL_RGBA4;
            case TextureSizedInternalFormat::RGB5A1:
                return GL_RGB5_A1;
            case TextureSizedInternalFormat::RGBA8:
                return GL_RGBA8;
            case TextureSizedInternalFormat::RGBA8Snorm:
                return GL_RGBA8_SNORM;
            case TextureSizedInternalFormat::RGB10A2:
                return GL_RGB10_A2;
            case TextureSizedInternalFormat::RGB10A2UI:
                return GL_RGB10_A2UI;
            case TextureSizedInternalFormat::RGBA12:
                return GL_RGBA12;
            case TextureSizedInternalFormat::RGBA16:
                return GL_RGBA16;
            case TextureSizedInternalFormat::SRGB8:
                return GL_SRGB8;
            case TextureSizedInternalFormat::SRGB8Alpha8:
                return GL_SRGB8_ALPHA8;
            case TextureSizedInternalFormat::R16F:
                return GL_R16F;
            case TextureSizedInternalFormat::RG16F:
                return GL_RG16F;
            case TextureSizedInternalFormat::RGB16F:
                return GL_RGB16F;
            case TextureSizedInternalFormat::RGBA16F:
                return GL_RGBA16F;
            case TextureSizedInternalFormat::R32F:
                return GL_R32F;
            case TextureSizedInternalFormat::RG32F:
                return GL_RG32F;
            case TextureSizedInternalFormat::RGB32F:
                return GL_RGB32F;
            case TextureSizedInternalFormat::RGBA32F:
                return GL_RGBA32F;
            case TextureSizedInternalFormat::R11FG11FB10F:
                return GL_R11F_G11F_B10F;
            case TextureSizedInternalFormat::RGB9E5:
                return GL_RGB9_E5;
            case TextureSizedInternalFormat::R8I:
                return GL_R8I;
            case TextureSizedInternalFormat::R8UI:
                return GL_R8UI;
            case TextureSizedInternalFormat::R16I:
                return GL_R16I;
            case TextureSizedInternalFormat::R16UI:
                return GL_R16UI;
            case TextureSizedInternalFormat::R32I:
                return GL_R32I;
            case TextureSizedInternalFormat::R32UI:
                return GL_R32UI;
            case TextureSizedInternalFormat::RG8I:
                return GL_RG8I;
            case TextureSizedInternalFormat::RG8UI:
                return GL_RG8UI;
            case TextureSizedInternalFormat::RG16I:
                return GL_RG16I;
            case TextureSizedInternalFormat::RG16UI:
                return GL_RG16UI;
            case TextureSizedInternalFormat::RG32I:
                return GL_RG32I;
            case TextureSizedInternalFormat::RG32UI:
                return GL_RG32UI;
            case TextureSizedInternalFormat::RGB8I:
                return GL_RGB8I;
            case TextureSizedInternalFormat::RGB8UI:
                return GL_RGB8UI;
            case TextureSizedInternalFormat::RGB16I:
                return GL_RGB16I;
            case TextureSizedInternalFormat::RGB16UI:
                return GL_RGB16UI;
            case TextureSizedInternalFormat::RGB32I:
                return GL_RGB32I;
            case TextureSizedInternalFormat::RGB32UI:
                return GL_RGB32UI;
            case TextureSizedInternalFormat::RGBA8I:
                return GL_RGBA8I;
            case TextureSizedInternalFormat::RGBA8UI:
                return GL_RGBA8UI;
            case TextureSizedInternalFormat::RGBA16I:
                return GL_RGBA16I;
            case TextureSizedInternalFormat::RGBA16UI:
                return GL_RGBA16UI;
            case TextureSizedInternalFormat::RGBA32I:
                return GL_RGBA32I;
            case TextureSizedInternalFormat::RGBA32UI:
                return GL_RGBA32UI;
            default:
                return GL_UNKNOWN_MGL;
            }
        }

        GLenum ConvertTexturePixelDataTypeToGLEnum(TexturePixelDataType type) {
            switch (type) {
            case TexturePixelDataType::UnsignedByte:
                return GL_UNSIGNED_BYTE;
            case TexturePixelDataType::Byte:
                return GL_BYTE;
            case TexturePixelDataType::UnsignedShort:
                return GL_UNSIGNED_SHORT;
            case TexturePixelDataType::Short:
                return GL_SHORT;
            case TexturePixelDataType::UnsignedInt:
                return GL_UNSIGNED_INT;
            case TexturePixelDataType::Int:
                return GL_INT;
            case TexturePixelDataType::Float:
                return GL_FLOAT;
            case TexturePixelDataType::UnsignedByte332:
                return GL_UNSIGNED_BYTE_3_3_2;
            case TexturePixelDataType::UnsignedByte233Rev:
                return GL_UNSIGNED_BYTE_2_3_3_REV;
            case TexturePixelDataType::UnsignedShort565:
                return GL_UNSIGNED_SHORT_5_6_5;
            case TexturePixelDataType::UnsignedShort565Rev:
                return GL_UNSIGNED_SHORT_5_6_5_REV;
            case TexturePixelDataType::UnsignedShort4444:
                return GL_UNSIGNED_SHORT_4_4_4_4;
            case TexturePixelDataType::UnsignedShort4444Rev:
                return GL_UNSIGNED_SHORT_4_4_4_4_REV;
            case TexturePixelDataType::UnsignedShort5551:
                return GL_UNSIGNED_SHORT_5_5_5_1;
            case TexturePixelDataType::UnsignedShort1555Rev:
                return GL_UNSIGNED_SHORT_1_5_5_5_REV;
            case TexturePixelDataType::UnsignedInt8888Rev:
                return GL_UNSIGNED_INT_8_8_8_8_REV;
            case TexturePixelDataType::UnsignedInt1010102:
                return GL_UNSIGNED_INT_10_10_10_2;
            default:
                return GL_UNKNOWN_MGL;
            }
        }

    } // namespace MG_Util
} // namespace MobileGL