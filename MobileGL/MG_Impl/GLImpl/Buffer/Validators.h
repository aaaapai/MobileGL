#pragma once
#include <Includes.h>
#include <MG_State/GLState/BufferState/BufferObject.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace BufferImpl {
        bool ValidateBufferTarget(BufferTarget target);
        bool ValidateBufferName(Uint index);
        bool ValidateBufferUsage(BufferUsage usage);
        bool ValidateBufferMappingAccess(Flags<BufferMappingAccessBit> accessBits);
    } // namespace BufferImpl
} // namespace MobileGL::MG_Impl::GLImpl