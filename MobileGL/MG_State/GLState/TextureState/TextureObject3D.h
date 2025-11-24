#pragma once
#include "TextureObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class TextureObject3D : public TextureObjectBase {
            public:
                explicit TextureObject3D(Uint externalIndex);

            protected:
                void SetMipmapImpl(const MipmapLevelInput& level) override;
            };
        }
    }
}
