// MobileGL - MobileGL/MG_Util/Converters/MGToStr/RenderStateEnumConverter.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
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
        String ConvertFrontFaceModeToString(FrontFaceMode value);
        String ConvertCapabilityInputToString(CapabilityInput value);
    } // namespace MG_Util
} // namespace MobileGL
