// MobileGL - MobileGL/MG_Util/Converters/GLToMG/TextureEnumConverter.h
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

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