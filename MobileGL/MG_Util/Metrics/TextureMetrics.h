// MobileGL - MobileGL/MG_Util/Metrics/TextureMetrics.h
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

namespace MobileGL {
    namespace MG_Util {
        SizeT GetSizedInternalFormatSizeInBytes(TextureInternalFormat internal);
        SizeT GetBaseInternalFormatComponentCount(TextureInternalFormat format);
        SizeT GetSizedTexturePixelDataTypeSize(TexturePixelDataType type);
        SizeT GetBaseTexturePixelDataTypeSize(TexturePixelDataType type);
        // This should respect internal format more
        SizeT GetInternalBytesPerPixel(TextureInternalFormat internalformat, TexturePixelDataType type);
        // This should respect type more, representing data passed in
        SizeT GetInputBytesPerPixel(TextureInputFormat inputFormat, TexturePixelDataType type);
        SizeT CalculateInputTextureImageSize(TextureInputFormat inputFormat, TexturePixelDataType pixelDataType,
                                             IntVec3 size);
        ComponentSizes GetComponentSizesForInternalFormat(TextureInternalFormat internal);
    } // namespace MG_Util
} // namespace MobileGL
