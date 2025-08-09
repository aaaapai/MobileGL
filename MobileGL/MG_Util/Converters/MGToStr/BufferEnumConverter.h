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