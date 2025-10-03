#pragma once
#include <Includes.h>
#include <MG_State/GLState/RenderState/RenderState.h>

namespace MobileGL {
    namespace MG_Util {
        BlendFactor ConvertGLEnumToBlendFactor(GLenum value);
        DepthFunc ConvertGLEnumToDepthFunc(GLenum value);
        PixelStoreParam ConvertGLEnumToPixelStoreParam(GLenum value);
        CullFaceMode ConvertGLEnumToCullFaceMode(GLenum value);
        CapabilityInput ConvertGLEnumToCapabilityInput(GLenum value);
    } // namespace MG_Util
} // namespace MobileGL