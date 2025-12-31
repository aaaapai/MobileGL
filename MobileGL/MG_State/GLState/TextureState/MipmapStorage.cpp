// MobileGL - MobileGL/MG_State/GLState/TextureState/MipmapStorage.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "MipmapStorage.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            SizeT MipmapStorage::GetLevelCount() const {
                return m_data.size();
            }

            void MipmapStorage::AllocateLevel(Uint level, MipmapInput input) {
                m_data.reserve(std::bit_ceil(level + 1));
                m_data.resize(level + 1);
                m_texelSizes.reserve(std::bit_ceil(level + 1));
                m_texelSizes.resize(level + 1);
                m_texelSizes[level] = input.texelSize;
                m_isDirty.resize(level + 1, false);

                auto& data = m_data[level];
                data.resize(input.byteSize, 0);
            }

            void MipmapStorage::UpdateSubData(Uint level, DataPtr input) {
                auto& targetData = m_data;
                MOBILEGL_ASSERT(level < targetData.size(), "UpdateSubData: level out of range");
                auto& levelData = targetData[level];
                MOBILEGL_ASSERT(levelData.size() <= input.size, "UpdateSubData: input data larger than allocated");

                if (input.data && input.size > 0) {
                    const Uint8* src = static_cast<const Uint8*>(input.data);
                    Memcpy(levelData.data(), src, input.size);
                    m_isDirty[level] = true;
                }
            }

            void* MipmapStorage::MapData(Uint level) {
                auto& targetData = m_data;
                MOBILEGL_ASSERT(level < targetData.size(), "UpdateSubData: level out of range");
                auto& levelData = targetData[level];
                return levelData.data();
            }

            IntVec3 MipmapStorage::GetTexelSize(Uint level) const {
                auto& targetTexelSizes = m_texelSizes;
                if (level >= targetTexelSizes.size()) return {0, 0, 0};
                return targetTexelSizes[level];
            }

            SizeT MipmapStorage::GetByteSize(Uint level) const {
                return m_data[level].size();
            }

            void MipmapStorage::MarkDirty(Uint level, bool dirty) {
                MOBILEGL_ASSERT(level < m_isDirty.size(), "MarkDirty: level out of range");
                m_isDirty[level] = dirty;
            }

            bool MipmapStorage::IsDirty(Uint level) const {
                MOBILEGL_ASSERT(level < m_isDirty.size(), "IsDirty: level out of range");
                return m_isDirty[level];
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
