#pragma once
#include "MG_Util/Types.h"
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace TextureImpl {
        Bool ValidateTextureTarget(TextureTarget target);
        Bool ValidateTextureUploadTarget(TextureUploadTarget textureUploadTarget);
        Bool ValidateTextureName(Uint texture, Bool allowZero = false);
        Bool ValidateTextureInputFormat(TextureInputFormat format);
        Bool ValidateTexturePixelDataType(TexturePixelDataType texturePixelDataType);
        Bool ValidateTextureLevelNumber(Int level);
        Bool ValidateTextureSizeWithTextureUploadTarget(TextureUploadTarget target, GLsizei width, GLsizei height);
        Bool ValidateTextureSizeRange(SizeT width, SizeT height);
        Bool ValidateTextureInternalFormat(TextureInternalFormat format);
        Bool ValidateTextureBorderNumber(Int border);
        Bool ValidateTextureFormatWithType(TextureInputFormat format, TextureInternalFormat internalFormat,
                                           TexturePixelDataType type);
        Bool ValidateTextureLevelWithUploadTarget(TextureUploadTarget target, Int level);
        Bool ValidateTextureObject(SharedPtr<MG_State::GLState::ITextureObject> textureObject);
        Bool ValidateTextureTargetUniformity(SharedPtr<MG_State::GLState::ITextureObject> textureObject,
                                             TextureTarget target);
    } // namespace TextureImpl
} // namespace MobileGL::MG_Impl::GLImpl