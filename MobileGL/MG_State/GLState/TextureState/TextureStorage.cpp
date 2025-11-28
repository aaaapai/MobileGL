#include "TextureStorage.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            template <SizeT TargetCount>
            TextureStorage<TargetCount>::TextureStorage() {
                static_assert(TargetCount > 0, "Mipmap size must be greater than zero");
            }

            template <SizeT TargetCount>
            void TextureStorage<TargetCount>::AllocateLevel(Uint targetIndex, Uint level, MipmapInput input) {
                MOBILEGL_ASSERT(targetIndex < TargetCount, "AllocateLevel: target invalid");

                auto& targetData = m_data[targetIndex];
                targetData.reserve(std::bit_ceil(level + 1));
                targetData.resize(level + 1);
                auto& targetTexelSizes = m_texelSizes[targetIndex];
                targetTexelSizes.reserve(std::bit_ceil(level + 1));
                targetTexelSizes.resize(level + 1);
                targetTexelSizes[level] = input.texelSize;
                auto& dirtyArr = m_isDirty[targetIndex];
                dirtyArr.resize(level + 1, false);

                auto& data = targetData[level];
                data.resize(input.byteSize, 0);
            }

            template <SizeT TargetCount>
            void TextureStorage<TargetCount>::UpdateSubData(Uint targetIndex, Uint level, DataPtr input) {
                MOBILEGL_ASSERT(targetIndex < TargetCount, "UpdateSubData: target invalid");
                auto& targetData = m_data[targetIndex];
                MOBILEGL_ASSERT(level < targetData.size(), "UpdateSubData: level out of range");
                auto& levelData = targetData[level];
                MOBILEGL_ASSERT(levelData.size() < input.size, "UpdateSubData: input data larger than allocated");

                if (input.data && input.size > 0) {
                    const Uint8* src = static_cast<const Uint8*>(input.data);
                    Memcpy(levelData.data(), src, input.size);
                    m_isDirty[targetIndex][level] = true;
                }
            }

            template <SizeT TargetCount>
            void* TextureStorage<TargetCount>::MapData(Uint targetIndex, Uint level) {
                MOBILEGL_ASSERT(targetIndex < TargetCount, "UpdateSubData: target invalid");
                auto& targetData = m_data[targetIndex];
                MOBILEGL_ASSERT(level < targetData.size(), "UpdateSubData: level out of range");
                auto& levelData = targetData[level];
                return levelData.data();
            }

            template <SizeT TargetCount>
            IntVec3 TextureStorage<TargetCount>::GetTexelSize(Uint targetIndex, Uint level) const {
                MOBILEGL_ASSERT(targetIndex < TargetCount, "GetTexelSize: target invalid");

                auto& targetTexelSizes = m_texelSizes[targetIndex];
                if (level >= targetTexelSizes.size()) return {0, 0, 0};
                return targetTexelSizes[level];
            }

            template <SizeT TargetCount>
            SizeT TextureStorage<TargetCount>::GetByteSize(Uint targetIndex, Uint level) const {
                MOBILEGL_ASSERT(targetIndex < TargetCount, "GetByteSize: target invalid");

                auto& data = m_data[targetIndex];
                return data[level].size();
            }

            template <SizeT TargetCount>
            SizeT TextureStorage<TargetCount>::GetLevelCount() const {
                return m_data[0].size();
            }

            template <SizeT TargetCount>
            void TextureStorage<TargetCount>::MarkDirty(Uint targetIndex, Uint level, bool dirty) {
                MOBILEGL_ASSERT(targetIndex < TargetCount, "MarkDirty: target invalid");
                MOBILEGL_ASSERT(level < m_isDirty[targetIndex].size(), "MarkDirty: level out of range");
                m_isDirty[targetIndex][level] = dirty;
            }

            template <SizeT TargetCount>
            bool TextureStorage<TargetCount>::IsDirty(Uint targetIndex, Uint level) const {
                MOBILEGL_ASSERT(targetIndex < TargetCount, "IsDirty: target invalid");
                MOBILEGL_ASSERT(level < m_isDirty[targetIndex].size(), "IsDirty: level out of range");
                return m_isDirty[targetIndex][level];
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
