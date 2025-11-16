#pragma once
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

namespace MobileGL {
    namespace MG_Util {
        bool IsDepthFormatInternalFormat(TextureInternalFormat internalformat);
        bool IsStencilFormatInternalFormat(TextureInternalFormat internalformat);
    } // namespace MG_Util
} // namespace MobileGL