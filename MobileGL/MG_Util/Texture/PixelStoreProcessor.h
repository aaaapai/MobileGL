// MobileGL - MobileGL/MG_Util/Texture/PixelStoreProcessor.h
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_State/GLState/RenderState/RenderState.h>
#include "MG_State/GLState/TextureState/TextureEnum.h"
#include "MG_Util/Metrics/TextureMetrics.h"

namespace MobileGL::MG_Util::PixelStoreProcessor {
    void* ProcessTexturePixelsDataUnpack(const void* inputPixels, const PixelStoreParameters& params,
                                         TextureInternalFormat targetInternalFormat, TextureInputFormat textureInputFormat, TexturePixelDataType inputDataType,
                                         IntVec3 dimension, Bool isBitmap, SizeT& outSize);
    void* ProcessTexturePixelsDataPack(const void* inputPixels, const PixelStoreParameters& params, SizeT pixelSize,
                                       IntVec3 dimension, Bool isBitmap, SizeT& outSize);
} // namespace MobileGL::MG_Util::PixelStoreProcessor
