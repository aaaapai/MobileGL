#include "TextureEnumConverter.h"

namespace MobileGL {
    namespace MG_Util {
        TextureTarget ConvertTextureUploadTargetToTextureTarget(TextureUploadTarget target) {
            switch (target) {
            case TextureUploadTarget::Texture2D:
            case TextureUploadTarget::ProxyTexture2D:
                return TextureTarget::Texture2D;
            case TextureUploadTarget::Texture1DArray:
            case TextureUploadTarget::ProxyTexture1DArray:
                return TextureTarget::Texture1DArray;
            case TextureUploadTarget::TextureRectangle:
            case TextureUploadTarget::ProxyTextureRectangle:
                return TextureTarget::TextureRectangle;
            case TextureUploadTarget::CubeMapPositiveX:
            case TextureUploadTarget::CubeMapNegativeX:
            case TextureUploadTarget::CubeMapPositiveY:
            case TextureUploadTarget::CubeMapNegativeY:
            case TextureUploadTarget::CubeMapPositiveZ:
            case TextureUploadTarget::CubeMapNegativeZ:
            case TextureUploadTarget::ProxyCubeMap:
            case TextureUploadTarget::CubeMapArray:
            case TextureUploadTarget::ProxyCubeMapArray:
                return TextureTarget::TextureCubeMap;
            case TextureUploadTarget::Texture2DMultisample:
            case TextureUploadTarget::ProxyTexture2DMultisample:
                return TextureTarget::Texture2DMultisample;
            case TextureUploadTarget::Texture2DMultisampleArray:
            case TextureUploadTarget::ProxyTexture2DMultisampleArray:
                return TextureTarget::Texture2DMultisampleArray;
            case TextureUploadTarget::Texture3D:
            case TextureUploadTarget::ProxyTexture3D:
                return TextureTarget::Texture3D;
            case TextureUploadTarget::Texture1D:
            case TextureUploadTarget::ProxyTexture1D:
                return TextureTarget::Texture1D;
            case TextureUploadTarget::Texture2DArray:
            case TextureUploadTarget::ProxyTexture2DArray:
                return TextureTarget::Texture2DArray;
            default:
                return TextureTarget::Unknown;
            }
        }
    } // namespace MG_Util
} // namespace MobileGL