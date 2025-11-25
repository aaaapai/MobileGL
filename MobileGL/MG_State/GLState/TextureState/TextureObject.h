#pragma once
#include "TextureEnum.h"
#include "TextureStorage.h"
#include "MG_Util/Types.h"
#include "../SamplerState/SamplerObject.h"
#include <Includes.h>
#include <MG_Util/Math/VectorTypes.h>

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            struct MipmapLevelBase {
                IntVec3 size = {0, 0, 0};
                Int level = 0;
                Bool compressed = false;
                Int compressedSize = 0;
            };

            struct MipmapLevelInput : MipmapLevelBase {
                DataPtr inputData = {nullptr, 0};

                MipmapLevelInput(IntVec3 size, Int level, Bool compressed, Int compressedSize, DataPtr data)
                    : MipmapLevelBase{size, level, compressed, compressedSize}, inputData(data) {}
            };

            struct MipmapLevelInternal : MipmapLevelBase {
                Data data;
                Bool dirty = true;
                Bool hasData = false;

                MipmapLevelInternal(const MipmapLevelInput& input) : MipmapLevelBase(input) {
                    data.resize(input.inputData.size, 0);
                    if (input.inputData.data && input.inputData.size > 0) {
                        const Uint8* src = static_cast<const Uint8*>(input.inputData.data);
                        Memcpy(data.data(), src, input.inputData.size);
                        hasData = true;
                    }
                }
            };

            class ITextureObject {
            public:
                using TargetEnum = TextureTarget;
                virtual ~ITextureObject() = default;

                // virtual void SetMipmapLevel(const MipmapLevelInput& level) = 0;
                // virtual const Vector<MipmapLevelInternal>& GetMipmaps() const = 0;
                // virtual MipmapLevelInternal& GetMipmap(Int index) = 0;
                // virtual void UnmarkMipmapDirty(Int index) = 0;

                virtual Uint GetMipmapLevelCount() const = 0;
                virtual const IntVec3 GetMipmapTexelSize(TextureUploadTarget target, Uint mipmapLevel) const = 0;
                virtual const SizeT GetMipmapByteSize(TextureUploadTarget target, Uint mipmapLevel) const = 0;
                virtual void AllocateStorage(TextureUploadTarget uploadTarget, Uint mipmapLevel, MipmapInput input) = 0;
                virtual void UpdateMipmapSubData(TextureUploadTarget uploadTarget, Uint mipmapLevel, DataPtr input) = 0;
                virtual void* MapMipmapData(TextureUploadTarget uploadTarget, Uint mipmapLevel) = 0;
                virtual void MarkStorageDirty(TextureUploadTarget uploadTarget, Uint mipmapLevel, bool dirty) = 0;
                virtual bool IsStorageDirty(TextureUploadTarget uploadTarget, Uint mipmapLevel) const = 0;

                virtual TextureInternalFormat GetFormat() const = 0;
                virtual TextureTarget GetTarget() const = 0;
                virtual IntVec3 GetBaseSize() const = 0;
                virtual SharedPtr<SamplerObject> GetSamplerObject() const = 0;
                virtual void SetInternalFormat(TextureInternalFormat format) = 0;
                virtual Bool IsComplete() const = 0;
                virtual Uint GetExternalIndex() const = 0;
                virtual const FloatVec4& GetBorderColor() const = 0;
                virtual void SetBorderColor(const FloatVec4& color) = 0;
                virtual TextureSwizzleParam GetSwizzleParam(TextureSwizzleParam param) const = 0;
                virtual void SetSwizzleParam(TextureSwizzleParam param, TextureSwizzleParam value) = 0;
                virtual const UintVec2& GetLevelRange() const = 0;
                virtual void SetBaseLevel(Uint baseLevel) = 0;
                virtual void SetMaxLevel(Uint maxLevel) = 0;
            protected:
                virtual Uint GetIndexOfTextureUploadTarget(TextureUploadTarget target) const = 0;
            };

            class TextureObjectBase : public ITextureObject {
            public:
                TextureObjectBase(TextureTarget target, Uint externalIndex);
                virtual ~TextureObjectBase() = default;

                // void SetMipmapLevel(const MipmapLevelInput& level) override;
                // const Vector<MipmapLevelInternal>& GetMipmaps() const override;
                // MipmapLevelInternal& GetMipmap(TextureUploadTarget target, Int index) override;
                // void UnmarkMipmapDirty(Int index) override;

                Uint GetMipmapLevelCount() const override;
                const IntVec3 GetMipmapTexelSize(TextureUploadTarget target, Uint mipmapLevel) const override;
                const SizeT GetMipmapByteSize(TextureUploadTarget target, Uint mipmapLevel) const override;
                void AllocateStorage(TextureUploadTarget uploadTarget, Uint mipmapLevel, MipmapInput input) override;
                void UpdateMipmapSubData(TextureUploadTarget uploadTarget, Uint mipmapLevel, DataPtr input) override;
                void* MapMipmapData(TextureUploadTarget uploadTarget, Uint mipmapLevel) override;
                void MarkStorageDirty(TextureUploadTarget uploadTarget, Uint mipmapLevel, bool dirty) override;
                bool IsStorageDirty(TextureUploadTarget uploadTarget, Uint mipmapLevel) const override;

                TextureInternalFormat GetFormat() const override;
                TextureTarget GetTarget() const override;
                IntVec3 GetBaseSize() const override;
                SharedPtr<SamplerObject> GetSamplerObject() const override;
                void SetInternalFormat(TextureInternalFormat format) override;
                Bool IsComplete() const override;
                Uint GetExternalIndex() const override;
                const FloatVec4& GetBorderColor() const override;
                void SetBorderColor(const FloatVec4& color) override;
                TextureSwizzleParam GetSwizzleParam(TextureSwizzleParam param) const override;
                void SetSwizzleParam(TextureSwizzleParam param, TextureSwizzleParam value) override;
                const UintVec2& GetLevelRange() const override;
                void SetBaseLevel(Uint baseLevel) override;
                void SetMaxLevel(Uint maxLevel) override;

            protected:
                // virtual void SetMipmapImpl(const MipmapLevelInput& level) = 0;

                const Uint m_externalIndex;
                const TextureTarget m_target = TextureTarget::Unknown;
                TextureInternalFormat m_internalFormat = TextureInternalFormat::Unknown;
                // Vector<MipmapLevelInternal> m_mipmaps = {};
                TextureStorage<1> m_textureStorage;
                SharedPtr<SamplerObject> m_sampler = nullptr;
                FloatVec4 m_borderColor = {0.0f, 0.0f, 0.0f, 0.0f};
                TextureSwizzleParam m_swizzleParams[4] = {TextureSwizzleParam::Red, TextureSwizzleParam::Green,
                                                          TextureSwizzleParam::Blue, TextureSwizzleParam::Alpha};
                UintVec2 m_levelRange = {0, 1000};
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
