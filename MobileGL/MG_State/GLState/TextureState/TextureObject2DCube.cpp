#include "TextureObject2DCube.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            TextureObject2DCube::TextureObject2DCube(Uint externalIndex):
                TextureObjectBase(TextureTarget::TextureCubeMap, externalIndex) {

            }

            Uint TextureObject2DCube::GetMipmapLevelCount() const {
                return m_textureStorage.GetLevelCount();
            }

            const IntVec3 TextureObject2DCube::GetMipmapTexelSize(TextureUploadTarget target,
                                                                  Uint mipmapLevel) const {
                return m_textureStorage.GetTexelSize(GetIndexOfTextureUploadTarget(target), mipmapLevel);
            }

            const SizeT TextureObject2DCube::GetMipmapByteSize(TextureUploadTarget target,
                                                               Uint mipmapLevel) const {
                return m_textureStorage.GetByteSize(GetIndexOfTextureUploadTarget(target), mipmapLevel);
            }

            void
            TextureObject2DCube::AllocateStorage(TextureUploadTarget uploadTarget, Uint mipmapLevel,
                                                 MipmapInput input) {
                m_textureStorage.AllocateLevel(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel, input);
            }

            void TextureObject2DCube::UpdateMipmapSubData(TextureUploadTarget uploadTarget,
                                                          Uint mipmapLevel, DataPtr input) {
                m_textureStorage.UpdateSubData(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel, input);
            }

            void *
            TextureObject2DCube::MapMipmapData(TextureUploadTarget uploadTarget, Uint mipmapLevel) {
                return m_textureStorage.MapData(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel);
            }

            void TextureObject2DCube::MarkStorageDirty(TextureUploadTarget uploadTarget,
                                                       Uint mipmapLevel, bool dirty) {
                m_textureStorage.MarkDirty(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel, dirty);
            }

            bool TextureObject2DCube::IsStorageDirty(TextureUploadTarget uploadTarget,
                                                     Uint mipmapLevel) const {
                return m_textureStorage.IsDirty(GetIndexOfTextureUploadTarget(uploadTarget), mipmapLevel);
            }

            Uint
            TextureObject2DCube::GetIndexOfTextureUploadTarget(TextureUploadTarget target) const {
                MOBILEGL_ASSERT(TextureUploadTarget::CubeMapPositiveX <= target && target <= TextureUploadTarget::ProxyCubeMap,
                                "Invalid TextureUploadTarget!");
                return (Uint)target - (Uint)TextureUploadTarget::CubeMapPositiveX;
            }
        }
    }
}
