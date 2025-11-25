#include "TextureObject.h"
#include <MG_Util/Metrics/TextureMetrics.h>

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            // TextureObjectBase implementations
            TextureObjectBase::TextureObjectBase(TextureTarget target, Uint externalIndex)
                : m_target(target), m_externalIndex(externalIndex) {
                m_sampler = MakeShared<SamplerObject>(0);
            }

            Uint TextureObjectBase::GetMipmapLevelCount() const {
                return m_textureStorage.GetLevelCount();
            }

            const IntVec3 TextureObjectBase::GetMipmapTexelSize(TextureUploadTarget target, Uint mipmapLevel) const {
                return m_textureStorage.GetTexelSize(GetIndexOfTextureUploadTarget(target), mipmapLevel);
            }

            const SizeT TextureObjectBase::GetMipmapByteSize(TextureUploadTarget target, Uint mipmapLevel) const {
                return m_textureStorage.GetByteSize(GetIndexOfTextureUploadTarget(target), mipmapLevel);
            }

            void TextureObjectBase::AllocateStorage(TextureUploadTarget uploadTarget, Uint mipmapLevel,
                                                    MipmapInput input) {
                // don't care target, index always 0
                m_textureStorage.AllocateLevel(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel, input);
            }
            void TextureObjectBase::UpdateMipmapSubData(TextureUploadTarget uploadTarget, Uint mipmapLevel,
                                                        DataPtr input) {
                // ditto
                m_textureStorage.UpdateSubData(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel, input);
            }
            void* TextureObjectBase::MapMipmapData(TextureUploadTarget uploadTarget, Uint mipmapLevel) {
                return m_textureStorage.MapData(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel);
            }

            void TextureObjectBase::MarkStorageDirty(TextureUploadTarget uploadTarget, Uint mipmapLevel, bool dirty) {
                // ditto
                m_textureStorage.MarkDirty(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel, dirty);
            }

            bool TextureObjectBase::IsStorageDirty(TextureUploadTarget uploadTarget, Uint mipmapLevel) const {
                return m_textureStorage.IsDirty(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel);
            }

            // void TextureObjectBase::SetMipmapLevel(const MipmapLevelInput& level) {
            //     SetMipmapImpl(level);
            // }

            TextureInternalFormat TextureObjectBase::GetFormat() const {
                return m_internalFormat;
            }

            TextureTarget TextureObjectBase::GetTarget() const {
                return m_target;
            }

            IntVec3 TextureObjectBase::GetBaseSize() const {
                if (m_textureStorage.GetLevelCount() == 0) {
                    return {0, 0, 0};
                }
                return m_textureStorage.GetTexelSize(0, 0);
            }

            SharedPtr<SamplerObject> TextureObjectBase::GetSamplerObject() const {
                return m_sampler;
            }

            // const Vector<MipmapLevelInternal>& TextureObjectBase::GetMipmaps() const {
            //     return m_mipmaps;
            // }

            Bool TextureObjectBase::IsComplete() const {
                if (m_internalFormat == TextureInternalFormat::Unknown) {
                    return false;
                }

                SizeT levelCount = m_textureStorage.GetLevelCount();
                if (levelCount == 0) {
                    return false;
                }

                for (size_t i = 0; i < levelCount; ++i) {
                    const auto& levelSize = m_textureStorage.GetTexelSize(0, i);
                    if (levelSize.x() <= 0 || levelSize.y() <= 0 || levelSize.z() <= 0) {
                        return false;
                    }
                }

                // TODO: add more completeness checks based on texture type and mipmap levels
                return true;
            }

            // MipmapLevelInternal& TextureObjectBase::GetMipmap(TextureUploadTarget target, Int index) {
            //     if (index >= m_mipmaps.size()) {
            //         MOBILEGL_ASSERT(false, "GetMipmap: index %d out of bounds, returning last mipmap level", index);
            //         index = static_cast<Int>(m_mipmaps.size() - 1);
            //     }
            //     return m_mipmaps[index];
            // }

            void TextureObjectBase::SetInternalFormat(TextureInternalFormat format) {
                m_internalFormat = format;
            }

            // void TextureObjectBase::UnmarkMipmapDirty(Int index) {
            //     if (index >= 0 && index < static_cast<Int>(m_mipmaps.size())) {
            //         m_mipmaps[index].dirty = false;
            //     }
            // }

            Uint TextureObjectBase::GetExternalIndex() const {
                return m_externalIndex;
            }

            const FloatVec4& TextureObjectBase::GetBorderColor() const {
                return m_borderColor;
            }

            void TextureObjectBase::SetBorderColor(const FloatVec4& color) {
                m_borderColor = color;
            }

            TextureSwizzleParam TextureObjectBase::GetSwizzleParam(TextureSwizzleParam param) const {
                switch (param) {
                case TextureSwizzleParam::Red:
                    return m_swizzleParams[0];
                case TextureSwizzleParam::Green:
                    return m_swizzleParams[1];
                case TextureSwizzleParam::Blue:
                    return m_swizzleParams[2];
                case TextureSwizzleParam::Alpha:
                    return m_swizzleParams[3];
                default:
                    MOBILEGL_ASSERT(false, "TextureObjectBase::GetSwizzleParam: Invalid TextureSwizzleParam: %d",
                                    static_cast<Int>(param));
                    return TextureSwizzleParam::Red;
                }
            }

            void TextureObjectBase::SetSwizzleParam(TextureSwizzleParam param, TextureSwizzleParam value) {
                switch (param) {
                case TextureSwizzleParam::Red:
                    m_swizzleParams[0] = value;
                    break;
                case TextureSwizzleParam::Green:
                    m_swizzleParams[1] = value;
                    break;
                case TextureSwizzleParam::Blue:
                    m_swizzleParams[2] = value;
                    break;
                case TextureSwizzleParam::Alpha:
                    m_swizzleParams[3] = value;
                    break;
                default:
                    MOBILEGL_ASSERT(false, "TextureObjectBase::SetSwizzleParam: Invalid TextureSwizzleParam: %d",
                                    static_cast<Int>(param));
                    break;
                }
            }

            const UintVec2& TextureObjectBase::GetLevelRange() const {
                return m_levelRange;
            }

            void TextureObjectBase::SetBaseLevel(Uint baseLevel) {
                m_levelRange.x() = baseLevel;
            }

            void TextureObjectBase::SetMaxLevel(Uint maxLevel) {
                m_levelRange.y() = maxLevel;
            }

            // TODO: add other texture types as needed

        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
