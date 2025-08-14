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
    } // namespace MG_Util
} // namespace MobileGL