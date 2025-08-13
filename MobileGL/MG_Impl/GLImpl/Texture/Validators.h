#pragma once
#include "MG_Util/Types.h"
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace TextureImpl {
        Bool ValidateTextureTarget(TextureTarget target);
        Bool ValidateTextureName(GLuint texture, Bool allowZero = false);
    } // namespace TextureImpl
} // namespace MobileGL::MG_Impl::GLImpl