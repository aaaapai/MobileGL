// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureObject.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "TextureObject.h"
#include "MG_Util/Types.h"
#include <MG_Util/Metrics/TextureMetrics.h>

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            // TextureObjectBase implementations
            TextureObjectBase::TextureObjectBase(TextureTarget target, Uint externalIndex)
                : m_target(target), m_externalIndex(externalIndex) {
                m_sampler = MakeShared<SamplerObject>(0);
            }

            TextureInternalFormat TextureObjectBase::GetFormat() const {
                return m_internalFormat;
            }

            TextureTarget TextureObjectBase::GetTarget() const {
                return m_target;
            }

            IntVec3 TextureObjectBase::GetBaseSize() const {
                return {0, 0, 0};
            }

            SharedPtr<SamplerObject> TextureObjectBase::GetSamplerObject() const {
                return m_sampler;
            }

            Bool TextureObjectBase::IsComplete() const {
                if (m_internalFormat == TextureInternalFormat::Unknown) {
                    return false;
                }

                return true;
            }

            void TextureObjectBase::SetInternalFormat(TextureInternalFormat format) {
                m_internalFormat = format;
                ++m_textureParamsVersion;
            }

            Uint TextureObjectBase::GetExternalIndex() const {
                return m_externalIndex;
            }

            const FloatVec4& TextureObjectBase::GetBorderColor() const {
                return m_borderColor;
            }

            void TextureObjectBase::SetBorderColor(const FloatVec4& color) {
                m_borderColor = color;
                ++m_textureParamsVersion;
            }

            TextureSwizzleParam TextureObjectBase::GetSwizzleParam(TextureSwizzleParam param) const {
                switch (param) {
                case TextureSwizzleParam::Red:
                    return m_swizzleParams.r();
                case TextureSwizzleParam::Green:
                    return m_swizzleParams.g();
                case TextureSwizzleParam::Blue:
                    return m_swizzleParams.b();
                case TextureSwizzleParam::Alpha:
                    return m_swizzleParams.a();
                default:
                    MOBILEGL_ASSERT(false, "TextureObjectBase::GetSwizzleParam: Invalid TextureSwizzleParam: %d",
                                    static_cast<Int>(param));
                    return TextureSwizzleParam::Red;
                }
            }

            const Vec4<TextureSwizzleParam>& TextureObjectBase::GetAllSwizzleParams() const {
                return m_swizzleParams;
            }

            void TextureObjectBase::SetSwizzleParam(TextureSwizzleParam param, TextureSwizzleParam value) {
                switch (param) {
                case TextureSwizzleParam::Red:
                    m_swizzleParams.r() = value;
                    break;
                case TextureSwizzleParam::Green:
                    m_swizzleParams.g() = value;
                    break;
                case TextureSwizzleParam::Blue:
                    m_swizzleParams.b() = value;
                    break;
                case TextureSwizzleParam::Alpha:
                    m_swizzleParams.a() = value;
                    break;
                default:
                    MOBILEGL_ASSERT(false, "TextureObjectBase::SetSwizzleParam: Invalid TextureSwizzleParam: %d",
                                    static_cast<Int>(param));
                    break;
                }
                ++m_textureParamsVersion;
            }

            void TextureObjectBase::SetSwizzleParamRGBA(const Vec4<TextureSwizzleParam>& values) {
                m_swizzleParams = values;
                ++m_textureParamsVersion;
            }

            const UintVec2& TextureObjectBase::GetLevelRange() const {
                return m_levelRange;
            }

            void TextureObjectBase::SetBaseLevel(Uint baseLevel) {
                m_levelRange.x() = baseLevel;
                ++m_textureParamsVersion;
            }

            void TextureObjectBase::SetMaxLevel(Uint maxLevel) {
                m_levelRange.y() = maxLevel;
                ++m_textureParamsVersion;
            }

            Bool TextureObjectBase::IsDirty() const {
                return m_dirtyBit;
            }

            Bool TextureObjectBase::CheckDirtyBit(TextureDirtyBit bit) const {
                return m_dirtyBit & bit;
            }

            Uint16 TextureObjectBase::GetTextureParamsVersion() const {
                return m_textureParamsVersion;
            }

            Uint TextureObjectWithOneMipmap::GetMipmapLevelCount() const {
                return m_textureStorage.GetLevelCount();
            }

            const IntVec3 TextureObjectWithOneMipmap::GetMipmapTexelSize(TextureUploadTarget target,
                                                                         Uint mipmapLevel) const {
                return m_textureStorage.GetTexelSize(GetIndexOfTextureUploadTarget(target), mipmapLevel);
            }

            const SizeT TextureObjectWithOneMipmap::GetMipmapByteSize(TextureUploadTarget target,
                                                                      Uint mipmapLevel) const {
                return m_textureStorage.GetByteSize(GetIndexOfTextureUploadTarget(target), mipmapLevel);
            }

            void TextureObjectWithOneMipmap::AllocateStorage(TextureUploadTarget uploadTarget, Uint mipmapLevel,
                                                             MipmapInput input) {
                m_textureStorage.AllocateLevel(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel, input);
            }

            void TextureObjectWithOneMipmap::UpdateMipmapSubData(TextureUploadTarget uploadTarget, Uint mipmapLevel,
                                                                 DataPtr input) {
                m_textureStorage.UpdateSubData(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel, input);
            }

            void* TextureObjectWithOneMipmap::MapMipmapData(TextureUploadTarget uploadTarget, Uint mipmapLevel) {
                return m_textureStorage.MapData(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel);
            }

            void TextureObjectWithOneMipmap::MarkStorageDirty(TextureUploadTarget uploadTarget, Uint mipmapLevel,
                                                              Bool dirty) {
                m_textureStorage.MarkDirty(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel, dirty);
                if (dirty) {
                    m_dirtyBit.Set(TextureDirtyBit::StorageDirtyBit);
                }
            }

            Bool TextureObjectWithOneMipmap::IsStorageDirty(TextureUploadTarget uploadTarget, Uint mipmapLevel) const {
                return m_textureStorage.IsDirty(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel);
            }

            IntVec3 TextureObjectWithOneMipmap::GetBaseSize() const {
                if (m_textureStorage.GetLevelCount() == 0) {
                    return {0, 0, 0};
                }
                return m_textureStorage.GetTexelSize(0, 0);
            }

            Bool TextureObjectWithOneMipmap::IsComplete() const {
                if (!TextureObjectBase::IsComplete()) return false;

                SizeT levelCount = m_textureStorage.GetLevelCount();
                if (levelCount == 0) {
                    MGLOG_D("%s: not complete because levelCount == 0", __func__);
                    return false;
                }

                // For some reason mojang decided to have 0x0 in last level mipmap
                // Relaxing checks for that
                Bool hadZero = false;
                for (SizeT i = 0; i < levelCount; ++i) {
                    const auto& levelSize = m_textureStorage.GetTexelSize(0, i);
                    if (levelSize.x() <= 0 || levelSize.y() <= 0 || levelSize.z() <= 0) {
                        hadZero = true;
                    } else {
                        if (hadZero) {
                            // We're checking for "zero - not zero - zero" here
                            // "not zero - zero - zero" should pass this test
                            MGLOG_D("%s: not complete because 0x0 occurred, and is not last level mipmap", __func__);
                            return false;
                        }
                    }
                }

                // TODO: add more completeness checks based on texture type and mipmap levels
                return true;
            }

            void TextureObjectWithOneMipmap::ClearAllStorageDirtyBit() {
                auto levelCount = GetMipmapLevelCount();
                for (Uint i = 0; i < levelCount; ++i) {
                    m_textureStorage.MarkDirty(0, i, false);
                }
                m_dirtyBit.Clear(TextureDirtyBit::StorageDirtyBit);
            }

            // TODO: add other texture types as needed

        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
