#include "TextureEnumConverter.h"

namespace MobileGL {
    namespace MG_Util {
        TextureTarget ConvertGLEnumToTextureTarget(GLenum target) {
            switch (target) {
            case GL_TEXTURE_1D:
                return TextureTarget::Texture1D;
            case GL_TEXTURE_2D:
                return TextureTarget::Texture2D;
            case GL_TEXTURE_3D:
                return TextureTarget::Texture3D;
            case GL_TEXTURE_CUBE_MAP:
                return TextureTarget::TextureCubeMap;
            case GL_TEXTURE_2D_ARRAY:
                return TextureTarget::Texture2DArray;
            case GL_TEXTURE_2D_MULTISAMPLE:
                return TextureTarget::Texture2DMultisample;
            case GL_TEXTURE_CUBE_MAP_ARRAY:
                return TextureTarget::TextureCubeMapArray;
            case GL_TEXTURE_BUFFER:
                return TextureTarget::TextureBuffer;
            case GL_TEXTURE_1D_ARRAY:
                return TextureTarget::Texture1DArray;
            case GL_TEXTURE_RECTANGLE:
                return TextureTarget::TextureRectangle;
            case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                return TextureTarget::Texture2DMultisampleArray;
            default:
                return TextureTarget::Unknown;
            }
        }

        TextureInputFormat ConvertGLEnumToTextureInputFormat(GLenum format) {
            switch (format) {
            case GL_RED:
                return TextureInputFormat::Red;
            case GL_RG:
                return TextureInputFormat::RG;
            case GL_RGB:
                return TextureInputFormat::RGB;
            case GL_BGR:
                return TextureInputFormat::BGR;
            case GL_RGBA:
                return TextureInputFormat::RGBA;
            case GL_BGRA:
                return TextureInputFormat::BGRA;
            case GL_RED_INTEGER:
                return TextureInputFormat::RInteger;
            case GL_RG_INTEGER:
                return TextureInputFormat::RGInteger;
            case GL_RGB_INTEGER:
                return TextureInputFormat::RGBInteger;
            case GL_BGR_INTEGER:
                return TextureInputFormat::BGRInteger;
            case GL_RGBA_INTEGER:
                return TextureInputFormat::RGBAInteger;
            case GL_BGRA_INTEGER:
                return TextureInputFormat::BGRAInteger;
            case GL_STENCIL_INDEX:
                return TextureInputFormat::StencilIndex;
            case GL_DEPTH_COMPONENT:
                return TextureInputFormat::DepthComponent;
            case GL_DEPTH_STENCIL:
                return TextureInputFormat::DepthStencil;
            default:
                return TextureInputFormat::Unknown;
            }
        }

        TextureSizedInternalFormat ConvertGLEnumToTextureSizedInternalFormat(GLenum internalformat) {
            switch (internalformat) {
            case GL_R8:
                return TextureSizedInternalFormat::R8;
            case GL_R8_SNORM:
                return TextureSizedInternalFormat::R8Snorm;
            case GL_R16:
                return TextureSizedInternalFormat::R16;
            case GL_R16_SNORM:
                return TextureSizedInternalFormat::R16Snorm;
            case GL_RG8:
                return TextureSizedInternalFormat::RG8;
            case GL_RG8_SNORM:
                return TextureSizedInternalFormat::RG8Snorm;
            case GL_RG16:
                return TextureSizedInternalFormat::RG16;
            case GL_RG16_SNORM:
                return TextureSizedInternalFormat::RG16Snorm;
            case GL_R3_G3_B2:
                return TextureSizedInternalFormat::R3G3B2;
            case GL_RGB4:
                return TextureSizedInternalFormat::RGB4;
            case GL_RGB5:
                return TextureSizedInternalFormat::RGB5;
            case GL_RGB8:
                return TextureSizedInternalFormat::RGB8;
            case GL_RGB8_SNORM:
                return TextureSizedInternalFormat::RGB8Snorm;
            case GL_RGB10:
                return TextureSizedInternalFormat::RGB10;
            case GL_RGB12:
                return TextureSizedInternalFormat::RGB12;
            case GL_RGB16_SNORM:
                return TextureSizedInternalFormat::RGB16Snorm;
            case GL_RGBA2:
                return TextureSizedInternalFormat::RGBA2;
            case GL_RGBA4:
                return TextureSizedInternalFormat::RGBA4;
            case GL_RGB5_A1:
                return TextureSizedInternalFormat::RGB5A1;
            case GL_RGBA8:
                return TextureSizedInternalFormat::RGBA8;
            case GL_RGBA8_SNORM:
                return TextureSizedInternalFormat::RGBA8Snorm;
            case GL_RGB10_A2:
                return TextureSizedInternalFormat::RGB10A2;
            case GL_RGB10_A2UI:
                return TextureSizedInternalFormat::RGB10A2UI;
            case GL_RGBA12:
                return TextureSizedInternalFormat::RGBA12;
            case GL_RGBA16:
                return TextureSizedInternalFormat::RGBA16;
            case GL_SRGB8:
                return TextureSizedInternalFormat::SRGB8;
            case GL_SRGB8_ALPHA8:
                return TextureSizedInternalFormat::SRGB8Alpha8;
            case GL_R16F:
                return TextureSizedInternalFormat::R16F;
            case GL_RG16F:
                return TextureSizedInternalFormat::RG16F;
            case GL_RGB16F:
                return TextureSizedInternalFormat::RGB16F;
            case GL_RGBA16F:
                return TextureSizedInternalFormat::RGBA16F;
            case GL_R32F:
                return TextureSizedInternalFormat::R32F;
            case GL_RG32F:
                return TextureSizedInternalFormat::RG32F;
            case GL_RGB32F:
                return TextureSizedInternalFormat::RGB32F;
            case GL_RGBA32F:
                return TextureSizedInternalFormat::RGBA32F;
            case GL_R11F_G11F_B10F:
                return TextureSizedInternalFormat::R11FG11FB10F;
            case GL_RGB9_E5:
                return TextureSizedInternalFormat::RGB9E5;
            case GL_R8I:
                return TextureSizedInternalFormat::R8I;
            case GL_R8UI:
                return TextureSizedInternalFormat::R8UI;
            case GL_R16I:
                return TextureSizedInternalFormat::R16I;
            case GL_R16UI:
                return TextureSizedInternalFormat::R16UI;
            case GL_R32I:
                return TextureSizedInternalFormat::R32I;
            case GL_R32UI:
                return TextureSizedInternalFormat::R32UI;
            case GL_RG8I:
                return TextureSizedInternalFormat::RG8I;
            case GL_RG8UI:
                return TextureSizedInternalFormat::RG8UI;
            case GL_RG16I:
                return TextureSizedInternalFormat::RG16I;
            case GL_RG16UI:
                return TextureSizedInternalFormat::RG16UI;
            case GL_RG32I:
                return TextureSizedInternalFormat::RG32I;
            case GL_RG32UI:
                return TextureSizedInternalFormat::RG32UI;
            case GL_RGB8I:
                return TextureSizedInternalFormat::RGB8I;
            case GL_RGB8UI:
                return TextureSizedInternalFormat::RGB8UI;
            case GL_RGB16I:
                return TextureSizedInternalFormat::RGB16I;
            case GL_RGB16UI:
                return TextureSizedInternalFormat::RGB16UI;
            case GL_RGB32I:
                return TextureSizedInternalFormat::RGB32I;
            case GL_RGB32UI:
                return TextureSizedInternalFormat::RGB32UI;
            case GL_RGBA8I:
                return TextureSizedInternalFormat::RGBA8I;
            case GL_RGBA8UI:
                return TextureSizedInternalFormat::RGBA8UI;
            case GL_RGBA16I:
                return TextureSizedInternalFormat::RGBA16I;
            case GL_RGBA16UI:
                return TextureSizedInternalFormat::RGBA16UI;
            case GL_RGBA32I:
                return TextureSizedInternalFormat::RGBA32I;
            case GL_RGBA32UI:
                return TextureSizedInternalFormat::RGBA32UI;
            default:
                return TextureSizedInternalFormat::Unknown;
            }
        }

        TexturePixelDataType ConvertGLEnumToTexturePixelDataType(GLenum type) {
            switch (type) {
            case GL_UNSIGNED_BYTE:
                return TexturePixelDataType::UnsignedByte;
            case GL_BYTE:
                return TexturePixelDataType::Byte;
            case GL_UNSIGNED_SHORT:
                return TexturePixelDataType::UnsignedShort;
            case GL_SHORT:
                return TexturePixelDataType::Short;
            case GL_UNSIGNED_INT:
                return TexturePixelDataType::UnsignedInt;
            case GL_INT:
                return TexturePixelDataType::Int;
            case GL_FLOAT:
                return TexturePixelDataType::Float;
            case GL_UNSIGNED_BYTE_3_3_2:
                return TexturePixelDataType::UnsignedByte332;
            case GL_UNSIGNED_BYTE_2_3_3_REV:
                return TexturePixelDataType::UnsignedByte233Rev;
            case GL_UNSIGNED_SHORT_5_6_5:
                return TexturePixelDataType::UnsignedShort565;
            case GL_UNSIGNED_SHORT_5_6_5_REV:
                return TexturePixelDataType::UnsignedShort565Rev;
            case GL_UNSIGNED_SHORT_4_4_4_4:
                return TexturePixelDataType::UnsignedShort4444;
            case GL_UNSIGNED_SHORT_4_4_4_4_REV:
                return TexturePixelDataType::UnsignedShort4444Rev;
            case GL_UNSIGNED_SHORT_5_5_5_1:
                return TexturePixelDataType::UnsignedShort5551;
            case GL_UNSIGNED_SHORT_1_5_5_5_REV:
                return TexturePixelDataType::UnsignedShort1555Rev;
            case GL_UNSIGNED_INT_8_8_8_8:
                return TexturePixelDataType::UnsignedInt8888;
            case GL_UNSIGNED_INT_8_8_8_8_REV:
                return TexturePixelDataType::UnsignedInt8888Rev;
            case GL_UNSIGNED_INT_10F_11F_11F_REV:
                return TexturePixelDataType::UnsignedInt1010102;
            case GL_UNSIGNED_INT_2_10_10_10_REV:
                return TexturePixelDataType::UnsignedInt2101010Rev;
            default:
                return TexturePixelDataType::Unknown;
            }
        }

        TextureUploadTarget ConvertGLEnumToTextureUploadTarget(GLenum target) {
            switch (target) {
            case GL_TEXTURE_2D:
                return TextureUploadTarget::Texture2D;
            case GL_PROXY_TEXTURE_2D:
                return TextureUploadTarget::ProxyTexture2D;
            case GL_TEXTURE_1D_ARRAY:
                return TextureUploadTarget::Texture1DArray;
            case GL_PROXY_TEXTURE_1D_ARRAY:
                return TextureUploadTarget::ProxyTexture1DArray;
            case GL_TEXTURE_RECTANGLE:
                return TextureUploadTarget::TextureRectangle;
            case GL_PROXY_TEXTURE_RECTANGLE:
                return TextureUploadTarget::ProxyTextureRectangle;
            case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
                return TextureUploadTarget::CubeMapPositiveX;
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
                return TextureUploadTarget::CubeMapNegativeX;
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
                return TextureUploadTarget::CubeMapPositiveY;
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
                return TextureUploadTarget::CubeMapNegativeY;
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
                return TextureUploadTarget::CubeMapPositiveZ;
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
                return TextureUploadTarget::CubeMapNegativeZ;
            case GL_PROXY_TEXTURE_CUBE_MAP:
                return TextureUploadTarget::ProxyCubeMap;
            default:
                return TextureUploadTarget::Unknown;
            }
        }

    } // namespace MG_Util
} // namespace MobileGL