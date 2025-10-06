#pragma once
#include "MG_Util/Types.h"
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
                SamplerObject()
                    : m_wrapS(SamplerWrapMode::Repeat), m_wrapT(SamplerWrapMode::Repeat),
                      m_wrapR(SamplerWrapMode::Repeat), m_minFilter(SamplerFilterMode::Linear),
                      m_magFilter(SamplerFilterMode::Linear), m_mipmapMode(SamplerMipmapMode::None), m_minLod(0.0f),
                      m_maxLod(1000.0f), m_lodBias(0.0f), m_compareMode(SamplerCompareMode::None),
                      m_compareFunc(SamplerCompareFunc::Less) {}

                void SetWrapS(SamplerWrapMode mode) { m_wrapS = mode; }
                void SetWrapT(SamplerWrapMode mode) { m_wrapT = mode; }
                void SetWrapR(SamplerWrapMode mode) { m_wrapR = mode; }
                void SetMinFilter(SamplerFilterMode mode) { m_minFilter = mode; }
                void SetMagFilter(SamplerFilterMode mode) { m_magFilter = mode; }
                void SetMipmapMode(SamplerMipmapMode mode) { m_mipmapMode = mode; }
                void SetLodRange(Float minLod, Float maxLod) {
                    if (minLod > maxLod) {
                        THROW_EXCEPTION("minLod cannot be greater than maxLod");
                    }
                    m_minLod = minLod;
                    m_maxLod = maxLod;
                }
                void SetLodBias(Float bias) { m_lodBias = bias; }
                void SetSamplerCompareFunc(SamplerCompareFunc func) { m_compareFunc = func; }
                void SetCompareMode(SamplerCompareMode mode) { m_compareMode = mode; }

                SamplerWrapMode GetWrapS() const { return m_wrapS; }
                SamplerWrapMode GetWrapT() const { return m_wrapT; }
                SamplerWrapMode GetWrapR() const { return m_wrapR; }
                SamplerFilterMode GetMinFilter() const { return m_minFilter; }
                SamplerFilterMode GetMagFilter() const { return m_magFilter; }
                SamplerMipmapMode GetMipmapMode() const { return m_mipmapMode; }
                Float GetMinLod() const { return m_minLod; }
                Float GetMaxLod() const { return m_maxLod; }
                Float GetLodBias() const { return m_lodBias; }
                SamplerCompareMode GetCompareMode() const { return m_compareMode; }
                SamplerCompareFunc GetSamplerCompareFunc() const { return m_compareFunc; }

            private:
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