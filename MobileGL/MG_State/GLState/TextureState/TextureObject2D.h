#pragma once
#include "TextureObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class TextureObject2D : public TextureObjectBase {
            public:
                explicit TextureObject2D(Uint externalIndex);

            protected:
                Uint GetIndexOfTextureUploadTarget(TextureUploadTarget target) const override;
            };
        }
    }
}
