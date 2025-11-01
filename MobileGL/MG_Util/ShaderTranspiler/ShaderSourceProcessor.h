#pragma once
#include <Includes.h>
#include <MG_State/GLState/ProgramState/ShaderObject.h>

namespace MobileGL {
    enum class ShaderProfile {
        Core,
        Compatibility,
        ES,
    };

    namespace MG_Util {
        namespace ShaderTranspiler {
            void PreprocessShaderSource(ShaderStage stage, String& source);
        } // namespace ShaderTranspiler
    } // namespace MG_Util
} // namespace MobileGL