#pragma once

namespace MobileGL {
    enum class TextureTarget {
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCubeMap,
        TextureRectangle,
        Texture2DMultisample,
        TextureBuffer,
        Texture1DArray,
        Texture2DArray,
        TextureCubeMapArray,
        Texture2DMultisampleArray,
        TextureTargetCount,
        Unknown = -1
    };

    // Don't tinker with order in this enum
    // it is used in TextureStorage
    enum class TextureUploadTarget {
        Texture2D,
        ProxyTexture2D,
        Texture1DArray,
        ProxyTexture1DArray,
        TextureRectangle,
        ProxyTextureRectangle,
        CubeMapPositiveX,
        CubeMapNegativeX,
        CubeMapPositiveY,
        CubeMapNegativeY,
        CubeMapPositiveZ,
        CubeMapNegativeZ,
        ProxyCubeMap,
        Texture2DMultisample,
        ProxyTexture2DMultisample,
        TextureUploadTargetCount,
        Unknown = -1
    };

    enum class TextureInputFormat {
        Red,
        RG,
        RGB,
        BGR,
        RGBA,
        BGRA,
        RInteger,
        RGInteger,
        RGBInteger,
        BGRInteger,
        RGBAInteger,
        BGRAInteger,
        StencilIndex,
        DepthComponent,
        DepthStencil,

        FormatCount,
        Unknown = -1
    };

    enum class TextureInternalFormat {
        R8,
        R8Snorm,
        R16,
        R16Snorm,
        RG8,
        RG8Snorm,
        RG16,
        RG16Snorm,
        R3G3B2,
        RGB4,
        RGB5,
        RGB8,
        RGB8Snorm,
        RGB10,
        RGB12,
        RGB16Snorm,
        RGBA2,
        RGBA4,
        RGB5A1,
        RGBA8,
        RGBA8Snorm,
        RGB10A2,
        RGB10A2UI,
        RGBA12,
        RGBA16,
        SRGB8,
        SRGB8Alpha8,
        R16F,
        RG16F,
        RGB16F,
        RGBA16F,
        R32F,
        RG32F,
        RGB32F,
        RGBA32F,
        R11FG11FB10F,
        RGB9E5,
        R8I,
        R8UI,
        R16I,
        R16UI,
        R32I,
        R32UI,
        RG8I,
        RG8UI,
        RG16I,
        RG16UI,
        RG32I,
        RG32UI,
        RGB8I,
        RGB8UI,
        RGB16I,
        RGB16UI,
        RGB32I,
        RGB32UI,
        RGBA8I,
        RGBA8UI,
        RGBA16I,
        RGBA16UI,
        RGBA32I,
        RGBA32UI,
        DepthComponent16,
        DepthComponent24,
        DepthComponent32, // not a standard format in OpenGL core profile
        DepthComponent32F,
        Depth24Stencil8,
        Depth32FStencil8,

        DepthComponent,
        DepthStencil,
        Red,
        RG,
        RGB,
        RGBA,

        TextureInternalFormatCount,
        Unknown = -1
    };

    enum class TexturePixelDataType {
        UnsignedByte,
        Byte,
        UnsignedShort,
        Short,
        UnsignedInt,
        Int,
        Float,
        UnsignedByte332,
        UnsignedByte233Rev,
        UnsignedShort565,
        UnsignedShort565Rev,
        UnsignedShort4444,
        UnsignedShort4444Rev,
        UnsignedShort5551,
        UnsignedShort1555Rev,
        UnsignedInt8888,
        UnsignedInt8888Rev,
        UnsignedInt1010102,
        UnsignedInt2101010Rev,
        UnsignedInt101111Rev, // not a standard type in OpenGL core profile
        UnsignedInt5999Rev,   // not a standard type in OpenGL core profile

        TypeCount,
        Unknown = -1
    };

    enum class TextureSwizzleParam {
        Red,
        Green,
        Blue,
        Alpha,
        Zero,
        One,

        SwizzleParamCount,
        Unknown = -1
    };
}