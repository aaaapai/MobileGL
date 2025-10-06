#include "TextureObject.h"
#include <MG_Util/Metrics/TextureMetrics.h>

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            // TextureObjectBase implementations
            TextureObjectBase::TextureObjectBase(TextureTarget target) : m_target(target) {
                m_sampler = MakeShared<SamplerObject>();
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
                if (index > m_mipmaps.size()) {
                    // fallback to the last mipmap
                    index = static_cast<Int>(m_mipmaps.size() - 1);
                }
                return m_mipmaps[index];
            }

            void TextureObjectBase::SetInternalFormat(TextureInternalFormat format) {
                m_internalFormat = format;
            }

            // TextureObject1D
            TextureObject1D::TextureObject1D() : TextureObjectBase(TextureTarget::Texture1D) {}

            void TextureObject1D::SetMipmapImpl(const MipmapLevelInput& level) {
                if (level.size.x() > 0) {
                    m_mipmaps.push_back(level);
                }
            }

            // TextureObject2D
            TextureObject2D::TextureObject2D() : TextureObjectBase(TextureTarget::Texture2D) {}

            void TextureObject2D::SetMipmapImpl(const MipmapLevelInput& level) {
                if (level.size.x() > 0 && level.size.y() > 0) {
                    m_mipmaps.push_back(level);
                }
            }

            // TextureObject3D
            TextureObject3D::TextureObject3D() : TextureObjectBase(TextureTarget::Texture3D) {}

            void TextureObject3D::SetMipmapImpl(const MipmapLevelInput& level) {
                if (level.size.x() > 0 && level.size.y() > 0 && level.size.z() > 0) {
                    m_mipmaps.push_back(level);
                }
            }

            // TODO: add other texture types as needed

        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
