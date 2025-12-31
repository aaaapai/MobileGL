// MobileGL - MobileGL/MG_State/GLState/TextureState/MipmapStorage.h
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include "TextureEnum.h"
#include "MG_Util/Types.h"
#include "MG_Util/Math/VectorTypes.h"
#include "TextureTypes.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class MipmapStorage {
            public:
                SizeT GetLevelCount() const;
                void AllocateLevel(Uint level, MipmapInput input);
                void UpdateSubData(Uint level, DataPtr input);
                void* MapData(Uint level);
                IntVec3 GetTexelSize(Uint level) const;
                SizeT GetByteSize(Uint level) const;
                void MarkDirty(Uint level, bool dirty);
                bool IsDirty(Uint level) const;
            protected:
                Vector<IntVec3> m_texelSizes;
                Vector<Vector<Uint8>> m_data;
                Vector<bool> m_isDirty;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
