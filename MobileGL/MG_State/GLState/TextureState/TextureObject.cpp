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

            void TextureObjectBase::SetMipmapLevel(const MipmapLevelInput& level) {
                SetMipmapImpl(level);
            }

            TextureInternalFormat TextureObjectBase::GetFormat() const {
                return m_internalFormat;
            }

            TextureTarget TextureObjectBase::GetTarget() const {
                return m_target;
            }

            IntVec3 TextureObjectBase::GetBaseSize() const {
                if (m_mipmaps.empty()) {
                    return {0, 0, 0};
                }
                return m_mipmaps[0].size;
            }

            SharedPtr<SamplerObject> TextureObjectBase::GetSamplerObject() const {
                return m_sampler;
            }

            const Vector<MipmapLevelInternal>& TextureObjectBase::GetMipmaps() const {
                return m_mipmaps;
            }

            Bool TextureObjectBase::IsComplete() const {
                if (m_internalFormat == TextureInternalFormat::Unknown) {
                    return false;
                }

                if (m_mipmaps.empty()) {
                    return false;
                }

                for (size_t i = 0; i < m_mipmaps.size(); ++i) {
                    const auto& level = m_mipmaps[i];
                    if (level.size.x() <= 0 || level.size.y() <= 0 || level.size.z() <= 0) {
                        return false;
                    }
                }

                // TODO: add more completeness checks based on texture type and mipmap levels
                return true;
            }

            MipmapLevelInternal& TextureObjectBase::GetMipmap(Int index) {
                if (index >= m_mipmaps.size()) {
                    MOBILEGL_ASSERT(false, "GetMipmap: index %d out of bounds, returning last mipmap level", index);
                    index = static_cast<Int>(m_mipmaps.size() - 1);
                }
                return m_mipmaps[index];
            }

            void TextureObjectBase::SetInternalFormat(TextureInternalFormat format) {
                m_internalFormat = format;
            }

            void TextureObjectBase::UnmarkMipmapDirty(Int index) {
                if (index >= 0 && index < static_cast<Int>(m_mipmaps.size())) {
                    m_mipmaps[index].dirty = false;
                }
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

            // TextureObject1D
            TextureObject1D::TextureObject1D(Uint externalIndex)
                : TextureObjectBase(TextureTarget::Texture1D, externalIndex) {}

            void TextureObject1D::SetMipmapImpl(const MipmapLevelInput& level) {
                if (level.size.x() > 0) {
                    m_mipmaps.push_back(MipmapLevelInternal(level));
                }
            }

            // TextureObject2D
            TextureObject2D::TextureObject2D(Uint externalIndex)
                : TextureObjectBase(TextureTarget::Texture2D, externalIndex) {}

            void TextureObject2D::SetMipmapImpl(const MipmapLevelInput& level) {
                if (level.size.x() > 0 && level.size.y() > 0) {
                    m_mipmaps.push_back(MipmapLevelInternal(level));
                }
            }

            // TextureObject3D
            TextureObject3D::TextureObject3D(Uint externalIndex)
                : TextureObjectBase(TextureTarget::Texture3D, externalIndex) {}

            void TextureObject3D::SetMipmapImpl(const MipmapLevelInput& level) {
                if (level.size.x() > 0 && level.size.y() > 0 && level.size.z() > 0) {
                    m_mipmaps.push_back(MipmapLevelInternal(level));
                }
            }

            // TODO: add other texture types as needed

        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
