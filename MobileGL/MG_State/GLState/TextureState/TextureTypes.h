#pragma once
#include "MG_Util/Types.h"
#include "MG_Util/Math/VectorTypes.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            struct MipmapInput {
                IntVec3 texelSize = {0, 0, 0};
                SizeT byteSize = 0;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
