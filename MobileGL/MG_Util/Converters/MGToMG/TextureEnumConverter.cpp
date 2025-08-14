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
                return TextureTarget::TextureCubeMap;
            case TextureUploadTarget::Texture2DMultisample:
            case TextureUploadTarget::ProxyTexture2DMultisample:
                return TextureTarget::Texture2DMultisample;
            default:
                return TextureTarget::Unknown;
            }
        }
    } // namespace MG_Util
} // namespace MobileGL