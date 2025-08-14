#include "TextureObject.h"
#include <MG_Util/Metrics/TextureMetrics.h>

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            // TextureObjectBase implementations
            TextureObjectBase::TextureObjectBase(TextureTarget target) : m_target(target), m_sampler(nullptr) {}

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
