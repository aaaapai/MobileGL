// MobileGL - MobileGL/MG_State/GLState/TextureState/MipmapUploadTargetArray.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include "MipmapStorage.h"
#include "TextureEnum.h"
#include "MG_Util/Types.h"
#include "MG_Util/Math/VectorTypes.h"
#include "TextureTypes.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            template <SizeT TargetCount>
            class MipmapUploadTargetArray {
            public:
                MipmapUploadTargetArray() { static_assert(TargetCount > 0, "Upload target count must be greater than zero"); }

                void AllocateLevel(Uint targetIndex, Uint level, MipmapInput input) {
                    MOBILEGL_ASSERT(targetIndex < TargetCount, "AllocateLevel: target invalid");

                    m_storage[targetIndex].AllocateLevel(level, input);
                }

                void UpdateSubData(Uint targetIndex, Uint level, DataPtr input) {
                    MOBILEGL_ASSERT(targetIndex < TargetCount, "UpdateSubData: target invalid");
                    m_storage[targetIndex].UpdateSubData(level, input);
                }

                void* MapData(Uint targetIndex, Uint level) {
                    MOBILEGL_ASSERT(targetIndex < TargetCount, "UpdateSubData: target invalid");
                    return m_storage[targetIndex].MapData(level);
                }

                IntVec3 GetTexelSize(Uint targetIndex, Uint level) const {
                    MOBILEGL_ASSERT(targetIndex < TargetCount, "GetTexelSize: target invalid");

                    return m_storage[targetIndex].GetTexelSize(level);
                }

                SizeT GetByteSize(Uint targetIndex, Uint level) const {
                    MOBILEGL_ASSERT(targetIndex < TargetCount, "GetByteSize: target invalid");

                    return m_storage[targetIndex].GetByteSize(level);
                }

                SizeT GetLevelCount() const {
                    static_assert(TargetCount > 0, "Upload target count must be greater than zero");
                    return m_storage[0].GetLevelCount();
                }

                void MarkDirty(Uint targetIndex, Uint level, bool dirty) {
                    MOBILEGL_ASSERT(targetIndex < TargetCount, "MarkDirty: target invalid");
                    m_storage[targetIndex].MarkDirty(level, dirty);
                }

                bool IsDirty(Uint targetIndex, Uint level) const {
                    MOBILEGL_ASSERT(targetIndex < TargetCount, "IsDirty: target invalid");
                    return m_storage[targetIndex].IsDirty(level);
                }

            protected:
                Array<MipmapStorage, TargetCount> m_storage;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
