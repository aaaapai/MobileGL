#pragma once
#include "TextureObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class TextureObject3D : public TextureObjectWithOneMipmap {
            public:
                explicit TextureObject3D(Uint externalIndex);

            protected:
                Uint GetIndexOfTextureUploadTarget(TextureUploadTarget target) const override;
            };
        }
    }
}
