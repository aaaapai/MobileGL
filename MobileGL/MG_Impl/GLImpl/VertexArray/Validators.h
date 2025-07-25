#pragma once

namespace MobileGL::MG_Impl::GLImpl {
	namespace VertexArrayImpl {
        bool ValidateVertexArrayName(Uint index);
        bool ValidateVertexArrayObject(Uint index);
        bool ValidateVertexAttributeIndex(Uint index);
        bool ValidateVertexAttribPointerParams(Uint index, SizeT size, DataType type, Int stride);
	}
}