#pragma once
#include <Includes.h>
#include <MG_State/GLState/RenderState/RenderState.h>

namespace MobileGL {
    namespace MG_Util {
        String ConvertBlendFactorToString(BlendFactor value);
        String ConvertDepthFuncToString(DepthFunc value);
        String ConvertPixelStoreParamToString(PixelStoreParam value);
        String ConvertCullFaceModeToString(CullFaceMode value);
        String ConvertCapabilityInputToString(CapabilityInput value);
    } // namespace MG_Util
} // namespace MobileGL