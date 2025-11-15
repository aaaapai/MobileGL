#include "SamplerObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {

            SamplerObject::SamplerObject(Uint externalIndex)
                : m_externalIndex(externalIndex), m_wrapS(SamplerWrapMode::Repeat), m_wrapT(SamplerWrapMode::Repeat),
                  m_wrapR(SamplerWrapMode::Repeat), m_minFilter(SamplerFilterMode::Linear),
                  m_magFilter(SamplerFilterMode::Linear), m_mipmapMode(SamplerMipmapMode::None), m_minLod(0.0f),
                  m_maxLod(1000.0f), m_lodBias(0.0f), m_compareMode(SamplerCompareMode::None),
                  m_compareFunc(SamplerCompareFunc::Less) {}

            void SamplerObject::SetWrapS(SamplerWrapMode mode) {
                m_wrapS = mode;
            }

            void SamplerObject::SetWrapT(SamplerWrapMode mode) {
                m_wrapT = mode;
            }

            void SamplerObject::SetWrapR(SamplerWrapMode mode) {
                m_wrapR = mode;
            }

            void SamplerObject::SetMinFilter(SamplerFilterMode mode) {
                m_minFilter = mode;
            }

            void SamplerObject::SetMagFilter(SamplerFilterMode mode) {
                m_magFilter = mode;
            }

            void SamplerObject::SetMipmapMode(SamplerMipmapMode mode) {
                m_mipmapMode = mode;
            }

            void SamplerObject::SetLodRange(Float minLod, Float maxLod) {
                if (minLod > maxLod) {
                    THROW_EXCEPTION("minLod cannot be greater than maxLod");
                }
                m_minLod = minLod;
                m_maxLod = maxLod;
            }

            void SamplerObject::SetLodBias(Float bias) {
                m_lodBias = bias;
            }

            void SamplerObject::SetSamplerCompareFunc(SamplerCompareFunc func) {
                m_compareFunc = func;
            }

            void SamplerObject::SetCompareMode(SamplerCompareMode mode) {
                m_compareMode = mode;
            }

            SamplerWrapMode SamplerObject::GetWrapS() const {
                return m_wrapS;
            }

            SamplerWrapMode SamplerObject::GetWrapT() const {
                return m_wrapT;
            }

            SamplerWrapMode SamplerObject::GetWrapR() const {
                return m_wrapR;
            }

            SamplerFilterMode SamplerObject::GetMinFilter() const {
                return m_minFilter;
            }

            SamplerFilterMode SamplerObject::GetMagFilter() const {
                return m_magFilter;
            }

            SamplerMipmapMode SamplerObject::GetMipmapMode() const {
                return m_mipmapMode;
            }

            Float SamplerObject::GetMinLod() const {
                return m_minLod;
            }

            Float SamplerObject::GetMaxLod() const {
                return m_maxLod;
            }

            Float SamplerObject::GetLodBias() const {
                return m_lodBias;
            }

            SamplerCompareMode SamplerObject::GetCompareMode() const {
                return m_compareMode;
            }

            SamplerCompareFunc SamplerObject::GetSamplerCompareFunc() const {
                return m_compareFunc;
            }

            Uint SamplerObject::GetExternalIndex() const {
                return m_externalIndex;
            }

        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
