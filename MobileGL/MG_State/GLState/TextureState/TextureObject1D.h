#pragma once
#include "TextureObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class TextureObject1D : public TextureObjectWithOneMipmap {
            public:
                explicit TextureObject1D(Uint externalIndex);

            protected:
                Uint GetIndexOfTextureUploadTarget(TextureUploadTarget target) const override;
            };
        }
    }
}
