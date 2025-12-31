// MobileGL - MobileGL/MG_State/GLState/SamplerState/SamplerObject.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "SamplerObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            SamplerObject::SamplerObject(Uint externalIndex) : m_externalIndex(externalIndex) {}

            void SamplerObject::SetWrapS(SamplerWrapMode mode) {
                m_samplerParameters.wrapS = mode;
            }

            void SamplerObject::SetWrapT(SamplerWrapMode mode) {
                m_samplerParameters.wrapT = mode;
            }

            void SamplerObject::SetWrapR(SamplerWrapMode mode) {
                m_samplerParameters.wrapR = mode;
            }

            void SamplerObject::SetMinFilter(SamplerFilterMode mode) {
                m_samplerParameters.minFilter = mode;
            }

            void SamplerObject::SetMagFilter(SamplerFilterMode mode) {
                m_samplerParameters.magFilter = mode;
            }

            void SamplerObject::SetMipmapMode(SamplerMipmapMode mode) {
                m_samplerParameters.mipmapMode = mode;
            }

            void SamplerObject::SetLodRange(Float minLod, Float maxLod) {
                if (minLod > maxLod) {
                    THROW_EXCEPTION("minLod cannot be greater than maxLod");
                }
                m_samplerParameters.minLod = minLod;
                m_samplerParameters.maxLod = maxLod;
            }

            void SamplerObject::SetLodBias(Float bias) {
                m_samplerParameters.lodBias = bias;
            }

            void SamplerObject::SetSamplerCompareFunc(SamplerCompareFunc func) {
                m_samplerParameters.compareFunc = func;
            }

            void SamplerObject::SetCompareMode(SamplerCompareMode mode) {
                m_samplerParameters.compareMode = mode;
            }

            SamplerWrapMode SamplerObject::GetWrapS() const {
                return m_samplerParameters.wrapS;
            }

            SamplerWrapMode SamplerObject::GetWrapT() const {
                return m_samplerParameters.wrapT;
            }

            SamplerWrapMode SamplerObject::GetWrapR() const {
                return m_samplerParameters.wrapR;
            }

            SamplerFilterMode SamplerObject::GetMinFilter() const {
                return m_samplerParameters.minFilter;
            }

            SamplerFilterMode SamplerObject::GetMagFilter() const {
                return m_samplerParameters.magFilter;
            }

            SamplerMipmapMode SamplerObject::GetMipmapMode() const {
                return m_samplerParameters.mipmapMode;
            }

            Float SamplerObject::GetMinLod() const {
                return m_samplerParameters.minLod;
            }

            Float SamplerObject::GetMaxLod() const {
                return m_samplerParameters.maxLod;
            }

            Float SamplerObject::GetLodBias() const {
                return m_samplerParameters.lodBias;
            }

            SamplerCompareMode SamplerObject::GetCompareMode() const {
                return m_samplerParameters.compareMode;
            }

            SamplerCompareFunc SamplerObject::GetSamplerCompareFunc() const {
                return m_samplerParameters.compareFunc;
            }

            Uint SamplerObject::GetExternalIndex() const {
                return m_externalIndex;
            }

            const SamplerParameters& SamplerObject::GetAllSamplerParameters() const {
                return m_samplerParameters;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
