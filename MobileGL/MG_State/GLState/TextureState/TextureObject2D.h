#pragma once
#include "TextureObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class TextureObject2D : public TextureObjectBase {
            public:
                explicit TextureObject2D(Uint externalIndex);

            protected:
                void SetMipmapImpl(const MipmapLevelInput& level) override;
            };
        }
    }
}
