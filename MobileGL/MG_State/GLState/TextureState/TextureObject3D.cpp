#include "TextureObject3D.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            TextureObject3D::TextureObject3D(Uint externalIndex)
                            : TextureObjectBase(TextureTarget::Texture3D, externalIndex) {}

            Uint TextureObject3D::GetIndexOfTextureUploadTarget(TextureUploadTarget target) const {
                MOBILEGL_ASSERT(target == TextureUploadTarget::Texture3D, "Invalid TextureUploadTarget!");
                return 0;
            }
        }
    }
}