// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureObjectStubs.h
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include "MG_State/GLState/TextureState/TextureEnum.h"
#include "TextureObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            /* These texture types are not yet implemented:
             * TextureRectangle,
             * Texture2DMultisample,
             * TextureBuffer,
             * Texture1DArray,
             * Texture2DArray,
             * TextureCubeMapArray,
             * Texture2DMultisampleArray
             */
#define STUB_TEXTURE_OBJECT_CLASS_DEFINITION(className, texTarget, uploadTargets)                                      \
    class className : public TextureObjectWithOneMipmap {                                                              \
    public:                                                                                                            \
        explicit className(Uint externalIndex) : TextureObjectWithOneMipmap(texTarget, externalIndex) {}               \
        const Vector<TextureUploadTarget>& GetUploadTargets() const override { return m_uploadTargets; }               \
                                                                                                                       \
    protected:                                                                                                         \
        Uint GetIndexOfTextureUploadTarget(TextureUploadTarget target) const override { return 0; }                    \
        const Vector<TextureUploadTarget> m_uploadTargets = uploadTargets;                                             \
    };

            STUB_TEXTURE_OBJECT_CLASS_DEFINITION(TextureObjectRectangle, TextureTarget::TextureRectangle,
                                                 {TextureUploadTarget::TextureRectangle});

            STUB_TEXTURE_OBJECT_CLASS_DEFINITION(TextureObject2DMultisample, TextureTarget::Texture2DMultisample,
                                                 {TextureUploadTarget::Texture2DMultisample});

            // STUB_TEXTURE_OBJECT_CLASS_DEFINITION(TextureObjectBuffer, TextureTarget::TextureBuffer,
            //                                      {TextureUploadTarget::Unknown});

            STUB_TEXTURE_OBJECT_CLASS_DEFINITION(TextureObject1DArray, TextureTarget::Texture1DArray,
                                                 {TextureUploadTarget::Texture1DArray});

            STUB_TEXTURE_OBJECT_CLASS_DEFINITION(TextureObject2DArray, TextureTarget::Texture2DArray,
                                                 {TextureUploadTarget::Texture2D});

            STUB_TEXTURE_OBJECT_CLASS_DEFINITION(TextureObjectCubeMapArray, TextureTarget::TextureCubeMapArray,
                                                 {TextureUploadTarget::CubeMapArray});

            STUB_TEXTURE_OBJECT_CLASS_DEFINITION(TextureObject2DMultisampleArray,
                                                 TextureTarget::Texture2DMultisampleArray,
                                                 {TextureUploadTarget::Texture2DMultisampleArray});
#undef STUB_TEXTURE_OBJECT_CLASS_DEFINITION

        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL