#include "PixelStoreProcessor.h"

namespace MobileGL::MG_Util::PixelStoreProcessor {
    static SizeT CalculateStride(Int width, SizeT pixelSize, Int alignment) {
        if (width <= 0 || pixelSize == 0) return 0;

        const SizeT rowBytes = static_cast<SizeT>(width) * pixelSize;
        const SizeT alignedRowBytes = (rowBytes + alignment - 1) & ~static_cast<SizeT>(alignment - 1);
        return alignedRowBytes;
    }

    static void SwapBytes(void* data, SizeT size, SizeT count) {
        if (size <= 1) return;

        Uint8* bytes = static_cast<Uint8*>(data);
        for (SizeT i = 0; i < count; ++i) {
            Uint8* pixel = bytes + i * size;
            std::reverse(pixel, pixel + size);
        }
    }

    static inline Uint8 ReverseByteBits(Uint8 b) {
        Uint8 r = 0;
        for (int i = 0; i < 8; ++i) {
            r <<= 1;
            r |= (b & 1);
            b >>= 1;
        }
        return r;
    }

    static void ProcessLSBFirst(void* data, SizeT width, SizeT height) {
        if (!data || width == 0 || height == 0) return;

        const SizeT rowBytes = (width + 7) / 8; // packed bits per row
        Uint8* bytes = static_cast<Uint8*>(data);

        for (SizeT y = 0; y < height; ++y) {
            for (SizeT i = 0; i < rowBytes; ++i) {
                bytes[i] = ReverseByteBits(bytes[i]);
            }
            bytes += rowBytes;
        }
    }

    void* ProcessTexturePixelsDataUnpack(const void* inputPixels, const PixelStoreParameters& params, SizeT pixelSize,
                                         IntVec3 dimension, Bool isBitmap, SizeT& outSize) {
        if (pixelSize == 0) {
            outSize = 0;
            return nullptr;
        }

        Int width = dimension.x();
        Int height = dimension.y();
        Int depth = dimension.z();

        const Int effectiveWidth = (params.RowLength > 0) ? params.RowLength : width;
        const Int effectiveHeight = (params.ImageHeight > 0) ? params.ImageHeight : height;
        const SizeT inputStride = CalculateStride(effectiveWidth, pixelSize, params.Alignment);
        const SizeT outputStride = static_cast<SizeT>(width) * pixelSize;

        const Int startX = params.SkipPixels;
        const Int startY = params.SkipRows;
        const Int startZ = params.SkipImages;

        const Int copyWidth = width;
        const Int copyHeight = height;
        const Int copyDepth = depth;

        if (copyWidth <= 0 || copyHeight <= 0 || copyDepth <= 0) {
            outSize = 0;
            return nullptr;
        }

        outSize = static_cast<SizeT>(copyWidth) * copyHeight * copyDepth * pixelSize;
        void* outputPixels = malloc(outSize);
        if (!outputPixels) {
            outSize = 0;
            return nullptr;
        }

        const Uint8* src = static_cast<const Uint8*>(inputPixels);
        Uint8* dst = static_cast<Uint8*>(outputPixels);

        src += static_cast<SizeT>(startZ) * static_cast<SizeT>(effectiveHeight) * inputStride;
        src += static_cast<SizeT>(startY) * inputStride;
        src += static_cast<SizeT>(startX) * pixelSize;

        for (Int z = 0; z < copyDepth; ++z) {
            const Uint8* layerSrc = src;
            Uint8* layerDst = dst;

            for (Int y = 0; y < copyHeight; ++y) {
                Memcpy(layerDst, layerSrc, static_cast<SizeT>(copyWidth) * pixelSize);

                if (params.SwapBytes && pixelSize > 1) {
                    SwapBytes(layerDst, pixelSize, static_cast<SizeT>(copyWidth));
                }

                if (params.LSBFirst && isBitmap) {
                    ProcessLSBFirst(layerDst, static_cast<SizeT>(copyWidth), 1);
                }

                layerSrc += inputStride;
                layerDst += static_cast<SizeT>(copyWidth) * pixelSize;
            }

            src += static_cast<SizeT>(effectiveHeight) * inputStride;
            dst += static_cast<SizeT>(copyHeight) * static_cast<SizeT>(copyWidth) * pixelSize;
        }

        return outputPixels;
    }

    void* ProcessTexturePixelsDataPack(const void* inputPixels, const PixelStoreParameters& params, SizeT pixelSize,
                                       IntVec3 dimension, Bool isBitmap, SizeT& outSize) {
        if (pixelSize == 0) {
            outSize = 0;
            return nullptr;
        }

        Int width = dimension.x();
        Int height = dimension.y();
        Int depth = dimension.z();

        const Int outputWidth = (params.RowLength > 0) ? params.RowLength : width;
        const SizeT outputStride = CalculateStride(outputWidth, pixelSize, params.Alignment);
        const SizeT inputStride = static_cast<SizeT>(width) * pixelSize;

        const Int effectiveHeight = (params.ImageHeight > 0) ? params.ImageHeight : height;

        outSize = static_cast<SizeT>(outputStride) * static_cast<SizeT>(effectiveHeight) * static_cast<SizeT>(depth);
        void* outputPixels = malloc(outSize);
        if (!outputPixels) {
            outSize = 0;
            return nullptr;
        }

        Memset(outputPixels, 0, outSize);

        const Uint8* src = static_cast<const Uint8*>(inputPixels);
        Uint8* dst = static_cast<Uint8*>(outputPixels);

        dst += static_cast<SizeT>(params.SkipImages) * static_cast<SizeT>(effectiveHeight) * outputStride;
        dst += static_cast<SizeT>(params.SkipRows) * outputStride;
        dst += static_cast<SizeT>(params.SkipPixels) * pixelSize;

        Vector<Uint8> tempRow(static_cast<SizeT>(width) * pixelSize);

        for (Int z = 0; z < depth; ++z) {
            Uint8* layerDst = dst;
            const Uint8* layerSrc = src;

            for (Int y = 0; y < height; ++y) {
                Memcpy(tempRow.data(), layerSrc, static_cast<SizeT>(width) * pixelSize);

                if (params.SwapBytes && pixelSize > 1) {
                    SwapBytes(tempRow.data(), pixelSize, static_cast<SizeT>(width));
                }

                if (params.LSBFirst && isBitmap) {
                    ProcessLSBFirst(tempRow.data(), static_cast<SizeT>(width), 1);
                }

                Memcpy(layerDst, tempRow.data(), static_cast<SizeT>(width) * pixelSize);

                layerSrc += inputStride;
                layerDst += outputStride;
            }

            src += static_cast<SizeT>(height) * inputStride;
            dst += static_cast<SizeT>(effectiveHeight) * outputStride;
        }

        return outputPixels;
    }
} // namespace MobileGL::MG_Util::PixelStoreProcessor
