#pragma once
#include <Includes.h>
#include <MG_State/GLState/BufferState/BufferObject.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace BufferImpl {
        Bool ValidateBufferTarget(BufferTarget target);
        Bool ValidateBufferName(Uint index, Bool allowZero = false);
        Bool ValidateBufferUsage(BufferUsage usage);
        Bool ValidateBufferMappingAccess(Flags<BufferMappingAccessBit> accessBits);
        Bool ValidateBufferBindingPointTarget(BufferTarget target);
    } // namespace BufferImpl
} // namespace MobileGL::MG_Impl::GLImpl