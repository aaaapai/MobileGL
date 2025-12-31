// MobileGL - MobileGL/MG_Util/Converters/MGToStr/BufferEnumConverter.h
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_State/GLState/BufferState/BufferObject.h>

namespace MobileGL {
    namespace MG_Util {
        String ConvertBufferTargetToString(BufferTarget bufferTarget);
        String ConvertBufferUsageToString(BufferUsage usage);
        String ConvertBufferMappingAccessToString(Flags<BufferMappingAccessBit> access);
    } // namespace MG_Util
} // namespace MobileGL