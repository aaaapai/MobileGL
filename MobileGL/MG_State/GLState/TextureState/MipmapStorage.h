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
