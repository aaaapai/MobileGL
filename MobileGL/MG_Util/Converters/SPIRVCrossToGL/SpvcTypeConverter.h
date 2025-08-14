#pragma once
#include <Includes.h>

#include "MG_Util/ShaderTranspiler/SpvcSession.h"

namespace MobileGL {
    namespace MG_Util {
        GLenum ConvertSpvcTypeToGLEnum(const ShaderTranspiler::SpvcType& spvcType);
    } // namespace MG_Util
} // namespace MobileGL