#pragma once
#include <Includes.h>
#include <MG_Util/Math/VectorTypes.h>
#include "TextureEnum.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            struct MipmapInput {
                IntVec3 texelSize = {0, 0, 0};
                SizeT byteSize = 0;
            };

            template <SizeT TargetCount>
            class TextureStorage {
            public:
                TextureStorage();

                void AllocateLevel(Uint targetIndex, Uint level, MipmapInput input);
                void UpdateSubData(Uint targetIndex, Uint level, DataPtr input);
                void* MapData(Uint targetIndex, Uint level);
                IntVec3 GetTexelSize(Uint targetIndex, Uint level) const;
                SizeT GetByteSize(Uint targetIndex, Uint level) const;
                SizeT GetLevelCount() const;
                void MarkDirty(Uint targetIndex, Uint level, bool dirty);
                bool IsDirty(Uint targetIndex, Uint level) const;

            protected:
                Array<Vector<IntVec3>, TargetCount> m_texelSizes;
                Array<Vector<Vector<Uint8>>, TargetCount> m_data;
                Array<Vector<bool>, TargetCount> m_isDirty;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
