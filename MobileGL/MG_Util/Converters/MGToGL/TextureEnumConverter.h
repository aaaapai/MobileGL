#pragma once
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

namespace MobileGL {
    namespace MG_Util {
        GLenum ConvertTextureTargetToGLEnum(TextureTarget target);
        GLenum ConvertTextureInputFormatToGLEnum(TextureInputFormat format);
        GLenum ConvertTextureInternalFormatToGLEnum(TextureInternalFormat internalformat);
        GLenum ConvertTexturePixelDataTypeToGLEnum(TexturePixelDataType type);
        GLenum ConvertTextureUploadTargetToGLEnum(TextureUploadTarget target);
//        GLenum ConvertSamplerFilterModeToGLEnum(SamplerFilterMode value);
//        GLenum ConvertSamplerMipmapModeToGLEnum(SamplerMipmapMode value);
        GLenum ConvertSamplerFilterModeToGLEnum(SamplerFilterMode filter, SamplerMipmapMode mipmap);
        GLenum ConvertSamplerWrapModeToGLEnum(SamplerWrapMode value);
        GLenum ConvertSamplerCompareModeToGLEnum(SamplerCompareMode value);
        GLenum ConvertSamplerCompareFuncToGLEnum(SamplerCompareFunc value);
        GLenum ConvertTextureSwizzleParamToGLEnum(TextureSwizzleParam value);
    } // namespace MG_Util
} // namespace MobileGL