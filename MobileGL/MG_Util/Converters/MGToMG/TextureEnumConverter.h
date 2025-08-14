#pragma once
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

namespace MobileGL {
    namespace MG_Util {
        TextureTarget ConvertTextureUploadTargetToTextureTarget(TextureUploadTarget target);
    } // namespace MG_Util
} // namespace MobileGL