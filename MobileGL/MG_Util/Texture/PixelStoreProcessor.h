#pragma once
#include <Includes.h>
#include <MG_State/GLState/RenderState/RenderState.h>

namespace MobileGL::MG_Util::PixelStoreProcessor {
    void* ProcessTexturePixelsDataUnpack(const void* inputPixels, const PixelStoreParameters& params, SizeT pixelSize,
                                         IntVec3 dimension, Bool isBitmap, SizeT& outSize);
    void* ProcessTexturePixelsDataPack(const void* inputPixels, const PixelStoreParameters& params, SizeT pixelSize,
                                       IntVec3 dimension, Bool isBitmap, SizeT& outSize);
} // namespace MobileGL::MG_Util::PixelStoreProcessor
