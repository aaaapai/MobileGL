// MobileGL - MobileGL/MG_Util/Converters/SPIRVCrossToGL/SpvcTypeConverter.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_Util/ShaderTranspiler/SpvcSession.h>

namespace MobileGL {
    namespace MG_Util {
        GLenum ConvertSpvcTypeToGLEnum(const ShaderTranspiler::SpvcType& spvcType);
    } // namespace MG_Util
} // namespace MobileGL