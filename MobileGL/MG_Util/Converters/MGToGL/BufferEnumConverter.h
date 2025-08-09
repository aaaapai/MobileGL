#pragma once
#include <Includes.h>
#include <MG_State/GLState/BufferState/BufferObject.h>

namespace MobileGL {
    namespace MG_Util {
        GLenum ConvertBufferTargetToGLEnum(BufferTarget bufferTarget);
        GLenum ConvertBufferUsageToGLEnum(BufferUsage usage);
        GLbitfield ConvertBufferMappingAccessToGLEnum(BufferMappingAccessBit access);
    } // namespace MG_Util
} // namespace MobileGL