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
        SamplerMipmapModeCount,
        Unknown = -1
    };

    enum class SamplerWrapMode {
        ClampToEdge,
        MirroredRepeat,
        Repeat,
        ClampToBorder,
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

            private:
                const Uint m_externalIndex;
                SamplerWrapMode m_wrapS, m_wrapT, m_wrapR;
                SamplerFilterMode m_minFilter, m_magFilter;
                SamplerMipmapMode m_mipmapMode;
                Float m_minLod, m_maxLod;
                Float m_lodBias;
                SamplerCompareMode m_compareMode;
                SamplerCompareFunc m_compareFunc;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
