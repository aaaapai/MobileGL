#pragma once
#include <Includes.h>
#include <MG_State/GLState/VertexArrayState/VertexArrayObject.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace VertexArrayImpl {
        Bool ValidateVertexArrayName(Uint index);
        Bool ValidateVertexArrayObject(Uint index);
        Bool ValidateVertexAttributeIndex(Uint index);
        Bool ValidateVertexAttribPointerParams(Uint index, SizeT size, DataType type, Int stride);
    } // namespace VertexArrayImpl
} // namespace MobileGL::MG_Impl::GLImpl