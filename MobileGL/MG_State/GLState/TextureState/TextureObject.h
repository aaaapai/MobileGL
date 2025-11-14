#pragma once
#include "MG_Util/Types.h"
#include "SamplerObject.h"
#include <Includes.h>
#include <MG_Util/Math/VectorTypes.h>

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

    namespace MG_State {
        namespace GLState {
            struct MipmapLevelBase {
                IntVec3 size = {0, 0, 0};
                Int level = 0;
                Bool compressed = false;
                Int compressedSize = 0;
            };

            struct MipmapLevelInput : MipmapLevelBase {
                DataPtr inputData = {nullptr, 0};

                MipmapLevelInput(IntVec3 size, Int level, Bool compressed, Int compressedSize, DataPtr data)
                    : MipmapLevelBase{size, level, compressed, compressedSize}, inputData(data) {}
            };

            struct MipmapLevelInternal : MipmapLevelBase {
                Data data;
                Bool dirty = true;

                MipmapLevelInternal(const MipmapLevelInput& input) : MipmapLevelBase(input) {
                    data.resize(input.inputData.size);
                    if (input.inputData.data && input.inputData.size > 0) {
                        const Uint8* src = static_cast<const Uint8*>(input.inputData.data);
                        Memcpy(data.data(), src, input.inputData.size);
                    }
                }
            };

            class ITextureObject {
            public:
                using TargetEnum = TextureTarget;
                virtual ~ITextureObject() = default;

                virtual void SetMipmapLevel(const MipmapLevelInput& level) = 0;
                virtual TextureInternalFormat GetFormat() const = 0;
                virtual TextureTarget GetTarget() const = 0;
                virtual IntVec3 GetBaseSize() const = 0;
                virtual SharedPtr<SamplerObject> GetSamplerObject() const = 0;
                virtual const Vector<MipmapLevelInternal>& GetMipmaps() const = 0;
                virtual MipmapLevelInternal& GetMipmap(Int index) = 0;
                virtual void SetInternalFormat(TextureInternalFormat format) = 0;
                virtual Bool IsComplete() const = 0;
                virtual void UnmarkMipmapDirty(Int index) = 0;
                virtual Uint GetExternalIndex() const = 0;
                virtual const FloatVec4& GetBorderColor() const = 0;
                virtual void SetBorderColor(const FloatVec4& color) = 0;
                virtual TextureSwizzleParam GetSwizzleParam(TextureSwizzleParam param) const = 0;
                virtual void SetSwizzleParam(TextureSwizzleParam param, TextureSwizzleParam value) = 0;
                virtual const UintVec2& GetLevelRange() const = 0;
                virtual void SetBaseLevel(Uint baseLevel) = 0;
                virtual void SetMaxLevel(Uint maxLevel) = 0;
            };

            class TextureObjectBase : public ITextureObject {
            public:
                TextureObjectBase(TextureTarget target, Uint externalIndex);
                virtual ~TextureObjectBase() = default;

                void SetMipmapLevel(const MipmapLevelInput& level) override;
                TextureInternalFormat GetFormat() const override;
                TextureTarget GetTarget() const override;
                IntVec3 GetBaseSize() const override;
                SharedPtr<SamplerObject> GetSamplerObject() const override;
                const Vector<MipmapLevelInternal>& GetMipmaps() const override;
                MipmapLevelInternal& GetMipmap(Int index) override;
                void SetInternalFormat(TextureInternalFormat format) override;
                Bool IsComplete() const override;
                void UnmarkMipmapDirty(Int index) override;
                Uint GetExternalIndex() const override;
                const FloatVec4& GetBorderColor() const override;
                void SetBorderColor(const FloatVec4& color) override;
                TextureSwizzleParam GetSwizzleParam(TextureSwizzleParam param) const override;
                void SetSwizzleParam(TextureSwizzleParam param, TextureSwizzleParam value) override;
                const UintVec2& GetLevelRange() const override;
                void SetBaseLevel(Uint baseLevel) override;
                void SetMaxLevel(Uint maxLevel) override;

            protected:
                virtual void SetMipmapImpl(const MipmapLevelInput& level) = 0;

                const Uint m_externalIndex;
                const TextureTarget m_target = TextureTarget::Unknown;
                TextureInternalFormat m_internalFormat = TextureInternalFormat::Unknown;
                Vector<MipmapLevelInternal> m_mipmaps = {};
                SharedPtr<SamplerObject> m_sampler = nullptr;
                FloatVec4 m_borderColor = {0.0f, 0.0f, 0.0f, 0.0f};
                TextureSwizzleParam m_swizzleParams[4] = {TextureSwizzleParam::Red, TextureSwizzleParam::Green,
                                                          TextureSwizzleParam::Blue, TextureSwizzleParam::Alpha};
                UintVec2 m_levelRange = {0, 1000};
            };

            class TextureObject1D : public TextureObjectBase {
            public:
                explicit TextureObject1D(Uint externalIndex);

            protected:
                void SetMipmapImpl(const MipmapLevelInput& level) override;
            };

            class TextureObject2D : public TextureObjectBase {
            public:
                explicit TextureObject2D(Uint externalIndex);

            protected:
                void SetMipmapImpl(const MipmapLevelInput& level) override;
            };

            class TextureObject3D : public TextureObjectBase {
            public:
                explicit TextureObject3D(Uint externalIndex);

            protected:
                void SetMipmapImpl(const MipmapLevelInput& level) override;
            };

            // TODO: add other texture types as needed

        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
