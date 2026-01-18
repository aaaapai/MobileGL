// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureTypes.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
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
