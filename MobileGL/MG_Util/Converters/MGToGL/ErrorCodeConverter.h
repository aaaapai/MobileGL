#pragma once
#include <Includes.h>
#include <MG_State/GLState/ErrorState/ErrorCode.h>

namespace MobileGL {
    namespace MG_Util {
        GLenum ConvertErrorCodeToGLEnum(ErrorCode code);
    }
} // namespace MobileGL