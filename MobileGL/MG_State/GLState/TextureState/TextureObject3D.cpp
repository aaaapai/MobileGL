#include "TextureObject3D.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            TextureObject3D::TextureObject3D(Uint externalIndex)
                            : TextureObjectWithOneMipmap(TextureTarget::Texture3D, externalIndex) {}

            Uint TextureObject3D::GetIndexOfTextureUploadTarget(TextureUploadTarget target) const {
                MOBILEGL_ASSERT(target == TextureUploadTarget::Texture3D ||
                                    target == TextureUploadTarget::ProxyTexture3D,
                                "Invalid TextureUploadTarget!");
                return 0;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL