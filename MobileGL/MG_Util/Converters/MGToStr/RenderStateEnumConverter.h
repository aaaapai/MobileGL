// MobileGL - MobileGL/MG_Util/Converters/MGToStr/RenderStateEnumConverter.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_State/GLState/RenderState/RenderState.h>

namespace MobileGL {
    namespace MG_Util {
        String ConvertBlendFactorToString(BlendFactor value);
        String ConvertDepthTestFuncToString(DepthTestFunc value);
        String ConvertPixelStoreParamToString(PixelStoreParam value);
        String ConvertCullFaceModeToString(CullFaceMode value);
        String ConvertCapabilityInputToString(CapabilityInput value);
    } // namespace MG_Util
} // namespace MobileGL