#include "TextureObject1D.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            TextureObject1D::TextureObject1D(Uint externalIndex)
                            : TextureObjectWithOneMipmap(TextureTarget::Texture1D, externalIndex) {}

            Uint TextureObject1D::GetIndexOfTextureUploadTarget(TextureUploadTarget target) const {
                MOBILEGL_ASSERT(target == TextureUploadTarget::Texture1D || target == TextureUploadTarget::ProxyTexture1D,
                                "Invalid TextureUploadTarget!");
                return 0;
            }
        }
    }
}