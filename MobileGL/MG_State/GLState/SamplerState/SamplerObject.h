#pragma once
#include <Includes.h>

namespace MobileGL {
    enum class SamplerFilterMode {
        Nearest,
        Linear,
        SamplerFilterCount,
        Unknown = -1
    };

    enum class SamplerMipmapMode {
        None,
        Nearest,
        Linear,
        SamplerMipmapModeCount,
        Unknown = -1
    };

    enum class SamplerWrapMode {
        ClampToEdge,
        MirroredRepeat,
        Repeat,
        ClampToBorder,
        MirrorClampToEdge,
        SamplerWrapModeCount,
        Unknown = -1
    };

    enum class SamplerCompareMode {
        None,
        CompareToTexture,
        SamplerCompareModeCount,
        Unknown = -1
    };

    enum class SamplerCompareFunc {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always,
        SamplerCompareFuncCount,
        Unknown = -1
    };

    struct SamplerParameters {
        SamplerWrapMode wrapS = SamplerWrapMode::Repeat;
        SamplerWrapMode wrapT = SamplerWrapMode::Repeat;
        SamplerWrapMode wrapR = SamplerWrapMode::Repeat;
        SamplerFilterMode minFilter = SamplerFilterMode::Linear;
        SamplerFilterMode magFilter = SamplerFilterMode::Linear;
        SamplerMipmapMode mipmapMode = SamplerMipmapMode::Linear;
        Float minLod = -1000.0f;
        Float maxLod = 1000.0f;
        Float lodBias = 0.0f;
        SamplerCompareFunc compareFunc = SamplerCompareFunc::Always;
        SamplerCompareMode compareMode = SamplerCompareMode::None;
    };

    namespace MG_State {
        namespace GLState {
            class SamplerObject {
            public:
                SamplerObject(Uint externalIndex);

                void SetWrapS(SamplerWrapMode mode);
                void SetWrapT(SamplerWrapMode mode);
                void SetWrapR(SamplerWrapMode mode);
                void SetMinFilter(SamplerFilterMode mode);
                void SetMagFilter(SamplerFilterMode mode);
                void SetMipmapMode(SamplerMipmapMode mode);
                void SetLodRange(Float minLod, Float maxLod);
                void SetLodBias(Float bias);
                void SetSamplerCompareFunc(SamplerCompareFunc func);
                void SetCompareMode(SamplerCompareMode mode);

                SamplerWrapMode GetWrapS() const;
                SamplerWrapMode GetWrapT() const;
                SamplerWrapMode GetWrapR() const;
                SamplerFilterMode GetMinFilter() const;
                SamplerFilterMode GetMagFilter() const;
                SamplerMipmapMode GetMipmapMode() const;
                Float GetMinLod() const;
                Float GetMaxLod() const;
                Float GetLodBias() const;
                SamplerCompareMode GetCompareMode() const;
                SamplerCompareFunc GetSamplerCompareFunc() const;
                Uint GetExternalIndex() const;
                const SamplerParameters& GetAllSamplerParameters() const;

            private:
                const Uint m_externalIndex;
                SamplerParameters m_samplerParameters;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
