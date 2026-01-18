// MobileGL - MobileGL/MG_Util/Metrics/TextureMetrics.h
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
