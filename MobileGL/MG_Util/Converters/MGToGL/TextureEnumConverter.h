// MobileGL - MobileGL/MG_Util/Converters/MGToGL/TextureEnumConverter.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

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