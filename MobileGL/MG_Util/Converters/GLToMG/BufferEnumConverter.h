#pragma once
#include <Includes.h>
#include <MG_State/GLState/BufferState/BufferObject.h>

namespace MobileGL {
    namespace MG_Util {
        BufferTarget ConvertGLEnumToBufferTarget(GLenum bufferTarget);
        BufferUsage ConvertGLEnumToBufferUsage(GLenum usage);
        Flags<BufferMappingAccessBit> ConvertGLEnumToBufferMappingAccess(GLbitfield access);
    } // namespace MG_Util
} // namespace MobileGL