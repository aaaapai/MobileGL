#pragma once
#include <Includes.h>
#include <MG_State/GLState/ProgramState/ShaderObject.h>

namespace MobileGL {
    namespace MG_Util {
        GLenum ConvertShaderStageToGLEnum(ShaderStage stage);
    }
} // namespace MobileGL