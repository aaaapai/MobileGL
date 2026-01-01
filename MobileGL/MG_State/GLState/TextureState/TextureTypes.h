// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureTypes.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

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
