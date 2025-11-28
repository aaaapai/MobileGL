#include "TextureEnumClassifier.h"

namespace MobileGL {
    namespace MG_Util {
        bool IsDepthFormatInternalFormat(TextureInternalFormat internalformat) {
            switch (internalformat) {
            case TextureInternalFormat::DepthComponent16:
            case TextureInternalFormat::DepthComponent24:
            case TextureInternalFormat::DepthComponent32:
            case TextureInternalFormat::DepthComponent32F:
            case TextureInternalFormat::Depth24Stencil8:
            case TextureInternalFormat::Depth32FStencil8:
            case TextureInternalFormat::DepthComponent:
            case TextureInternalFormat::DepthStencil:
                return true;
            default:
                return false;
            }
        }

        bool IsStencilFormatInternalFormat(TextureInternalFormat internalformat) {
            switch (internalformat) {
            case TextureInternalFormat::Depth24Stencil8:
            case TextureInternalFormat::Depth32FStencil8:
            case TextureInternalFormat::DepthStencil:
                return true;
            default:
                return false;
            }
        }
    } // namespace MG_Util
} // namespace MobileGL