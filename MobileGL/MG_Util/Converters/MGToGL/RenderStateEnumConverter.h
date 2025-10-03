#pragma once
#include <Includes.h>
#include <MG_State/GLState/RenderState/RenderState.h>

namespace MobileGL {
    namespace MG_Util {
        GLenum ConvertBlendFactorToGLEnum(BlendFactor value);
        GLenum ConvertDepthFuncToGLEnum(DepthFunc value);
        GLenum ConvertPixelStoreParamToGLEnum(PixelStoreParam value);
        GLenum ConvertCullFaceModeToGLEnum(CullFaceMode value);
        GLenum ConvertCapabilityInputToGLEnum(CapabilityInput value);
    } // namespace MG_Util
} // namespace MobileGL