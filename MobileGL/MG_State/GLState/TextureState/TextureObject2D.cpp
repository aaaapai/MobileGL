#include "TextureObject2D.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            TextureObject2D::TextureObject2D(Uint externalIndex)
                : TextureObjectBase(TextureTarget::Texture2D, externalIndex) {}

            // void TextureObject2D::SetMipmapImpl(const MipmapLevelInput& level) {
            //     if (level.size.x() > 0 && level.size.y() > 0) {
            //         m_mipmaps.push_back(MipmapLevelInternal(level));
            //     }
            // }
        }
    }
}