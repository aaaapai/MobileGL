#pragma once
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

namespace MobileGL {
    namespace MG_Util {
        TextureTarget ConvertGLEnumToTextureTarget(GLenum target);
        TextureInputFormat ConvertGLEnumToTextureInputFormat(GLenum format);
        TextureInternalFormat ConvertGLEnumToTextureInternalFormat(GLenum internalformat);
        TexturePixelDataType ConvertGLEnumToTexturePixelDataType(GLenum type);
        TextureUploadTarget ConvertGLEnumToTextureUploadTarget(GLenum target);
        SamplerFilterMode ConvertGLEnumToSamplerFilterMode(GLenum value);
        SamplerMipmapMode ConvertGLEnumToSamplerMipmapMode(GLenum value);
        SamplerWrapMode ConvertGLEnumToSamplerWrapMode(GLenum value);
        SamplerCompareMode ConvertGLEnumToSamplerCompareMode(GLenum value);
        SamplerCompareFunc ConvertGLEnumToSamplerCompareFunc(GLenum value);
        TextureSwizzleParam ConvertGLEnumToTextureSwizzleParam(GLenum value);
        TextureSwizzleParam ConvertGLEnumPnameToTextureSwizzleParam(GLenum value);
    } // namespace MG_Util
} // namespace MobileGL