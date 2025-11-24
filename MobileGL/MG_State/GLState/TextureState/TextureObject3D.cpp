#include "TextureObject3D.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            TextureObject3D::TextureObject3D(Uint externalIndex)
                            : TextureObjectBase(TextureTarget::Texture3D, externalIndex) {}

            // void TextureObject3D::SetMipmapImpl(const MipmapLevelInput& level) {
            //     if (level.size.x() > 0 && level.size.y() > 0 && level.size.z() > 0) {
            //         m_mipmaps.push_back(MipmapLevelInternal(level));
            //     }
            // }
        }
    }
}