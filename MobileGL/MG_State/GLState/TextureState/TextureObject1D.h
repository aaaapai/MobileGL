#pragma once
#include "TextureObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class TextureObject1D : public TextureObjectBase {
            public:
                explicit TextureObject1D(Uint externalIndex);

            protected:
                // void SetMipmapImpl(const MipmapLevelInput& level) override;
                Uint GetIndexOfTextureUploadTarget(TextureUploadTarget target) const override;
            };
        }
    }
}
