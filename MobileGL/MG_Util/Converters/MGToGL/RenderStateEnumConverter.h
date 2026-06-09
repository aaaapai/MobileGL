// MobileGL - MobileGL/MG_Util/Converters/MGToGL/RenderStateEnumConverter.h
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
        GLenum ConvertBlendFactorToGLEnum(BlendFactor value);
        GLenum ConvertBlendEquationToGLEnum(BlendEquation value);
        GLenum ConvertLogicOperationToGLEnum(LogicOperation value);
        GLenum ConvertDepthTestFuncToGLEnum(DepthTestFunc value);
        GLenum ConvertStencilOperationToGLEnum(StencilOperation value);
        GLenum ConvertPixelStoreParamToGLEnum(PixelStoreParam value);
        GLenum ConvertCullFaceModeToGLEnum(CullFaceMode value);
        GLenum ConvertFrontFaceModeToGLEnum(FrontFaceMode value);
        GLenum ConvertCapabilityInputToGLEnum(CapabilityInput value);
    } // namespace MG_Util
} // namespace MobileGL
