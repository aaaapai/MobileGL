#include "TextureObject2D.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            TextureObject2D::TextureObject2D(Uint externalIndex)
                : TextureObjectBase(TextureTarget::Texture2D, externalIndex) {}

            Uint TextureObject2D::GetIndexOfTextureUploadTarget(TextureUploadTarget target) const {
                MOBILEGL_ASSERT(target == TextureUploadTarget::Texture1D, "Invalid TextureUploadTarget!");
                return 0;
            }
        }
    }
}