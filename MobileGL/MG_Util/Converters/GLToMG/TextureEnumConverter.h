#pragma once
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

namespace MobileGL {
    namespace MG_Util {
        TextureTarget ConvertGLEnumToTextureTarget(GLenum target);
        TextureInputFormat ConvertGLEnumToTextureInputFormat(GLenum format);
        TextureSizedInternalFormat ConvertGLEnumToTextureSizedInternalFormat(GLenum internalformat);
        TexturePixelDataType ConvertGLEnumToTexturePixelDataType(GLenum type);
        TextureUploadTarget ConvertGLEnumToTextureUploadTarget(GLenum target);
    } // namespace MG_Util
} // namespace MobileGL