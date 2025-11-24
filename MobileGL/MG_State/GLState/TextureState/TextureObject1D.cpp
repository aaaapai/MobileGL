#include "TextureObject1D.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            TextureObject1D::TextureObject1D(Uint externalIndex)
                            : TextureObjectBase(TextureTarget::Texture1D, externalIndex) {}

            void TextureObject1D::SetMipmapImpl(const MipmapLevelInput& level) {
                if (level.size.x() > 0) {
                    m_mipmaps.push_back(MipmapLevelInternal(level));
                }
            }
        }
    }
}