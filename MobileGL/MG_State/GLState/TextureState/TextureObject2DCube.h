#pragma once
#include "TextureObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class TextureObject2DCube : public TextureObjectBase {
            public:
                explicit TextureObject2DCube(Uint externalIndex);

                const Vector<TextureUploadTarget>& GetUploadTargets() const override { return m_uploadTargets; }

                Uint GetMipmapLevelCount() const override;
                const IntVec3 GetMipmapTexelSize(TextureUploadTarget target, Uint mipmapLevel) const override;
                const SizeT GetMipmapByteSize(TextureUploadTarget target, Uint mipmapLevel) const override;
                void AllocateStorage(TextureUploadTarget uploadTarget, Uint mipmapLevel, MipmapInput input) override;
                void UpdateMipmapSubData(TextureUploadTarget uploadTarget, Uint mipmapLevel, DataPtr input) override;
                void* MapMipmapData(TextureUploadTarget uploadTarget, Uint mipmapLevel) override;
                void MarkStorageDirty(TextureUploadTarget uploadTarget, Uint mipmapLevel, bool dirty) override;
                bool IsStorageDirty(TextureUploadTarget uploadTarget, Uint mipmapLevel) const override;

                IntVec3 GetBaseSize() const override;
                Bool IsComplete() const override;

            protected:
                Uint GetIndexOfTextureUploadTarget(TextureUploadTarget target) const override;
                TextureStorage<6> m_textureStorage;
                const Vector<TextureUploadTarget> m_uploadTargets{
                    TextureUploadTarget::CubeMapPositiveX, TextureUploadTarget::CubeMapNegativeX,
                    TextureUploadTarget::CubeMapPositiveY, TextureUploadTarget::CubeMapNegativeY,
                    TextureUploadTarget::CubeMapPositiveZ, TextureUploadTarget::CubeMapNegativeZ,
                };
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
