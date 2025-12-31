// MobileGL - MobileGL/MG_Impl/GLImpl/Texture/Validators.h
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

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
        Bool ValidateTextureInternalFormatCompatibleWithInput(TextureInputFormat format, TextureInternalFormat internalFormat,
                                                              TexturePixelDataType type);
        Bool ValidateTextureLevelWithUploadTarget(TextureUploadTarget target, Int level);
        Bool ValidateTextureObject(SharedPtr<MG_State::GLState::ITextureObject> textureObject);
        Bool ValidateTextureTargetUniformity(SharedPtr<MG_State::GLState::ITextureObject> textureObject,
                                             TextureTarget target);
        Bool ValidateTextureSubImageOffsets(SharedPtr<MG_State::GLState::ITextureObject> textureObject, Int xoffset,
                                            Int width, Int yoffset = 0, Int height = 0, Int zoffset = 0, Int depth = 0);
    } // namespace TextureImpl
} // namespace MobileGL::MG_Impl::GLImpl