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
            }

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

            Uint TextureObjectWithOneMipmap::GetMipmapLevelCount() const {
                return m_textureStorage.GetLevelCount();
            }

            const IntVec3 TextureObjectWithOneMipmap::GetMipmapTexelSize(TextureUploadTarget target, Uint mipmapLevel) const {
                return m_textureStorage.GetTexelSize(GetIndexOfTextureUploadTarget(target), mipmapLevel);
            }

            const SizeT TextureObjectWithOneMipmap::GetMipmapByteSize(TextureUploadTarget target, Uint mipmapLevel) const {
                return m_textureStorage.GetByteSize(GetIndexOfTextureUploadTarget(target), mipmapLevel);
            }

            void TextureObjectWithOneMipmap::AllocateStorage(TextureUploadTarget uploadTarget, Uint mipmapLevel,
                                                    MipmapInput input) {
                // don't care target, index always 0
                m_textureStorage.AllocateLevel(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel, input);
            }
            void TextureObjectWithOneMipmap::UpdateMipmapSubData(TextureUploadTarget uploadTarget, Uint mipmapLevel,
                                                        DataPtr input) {
                // ditto
                m_textureStorage.UpdateSubData(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel, input);
            }
            void* TextureObjectWithOneMipmap::MapMipmapData(TextureUploadTarget uploadTarget, Uint mipmapLevel) {
                return m_textureStorage.MapData(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel);
            }

            void TextureObjectWithOneMipmap::MarkStorageDirty(TextureUploadTarget uploadTarget, Uint mipmapLevel, bool dirty) {
                // ditto
                m_textureStorage.MarkDirty(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel, dirty);
            }

            bool TextureObjectWithOneMipmap::IsStorageDirty(TextureUploadTarget uploadTarget, Uint mipmapLevel) const {
                return m_textureStorage.IsDirty(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel);
            }

            IntVec3 TextureObjectWithOneMipmap::GetBaseSize() const {
                if (m_textureStorage.GetLevelCount() == 0) {
                    return {0, 0, 0};
                }
                return m_textureStorage.GetTexelSize(0, 0);
            }

            Bool TextureObjectWithOneMipmap::IsComplete() const {
                if (!TextureObjectBase::IsComplete())
                    return false;

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

            // TODO: add other texture types as needed

        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
