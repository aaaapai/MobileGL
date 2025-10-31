#pragma once
#include <Includes.h>
#include <MG_State/GLState/ProgramState/ShaderObject.h>

namespace MobileGL {
    namespace MG_Util {
        ShaderStage ConvertGLEnumToShaderStage(GLenum type);
    }
} // namespace MobileGL