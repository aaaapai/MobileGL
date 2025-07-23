#pragma once

namespace MobileGL::MG_Impl::GLImpl {
	namespace BufferImpl {
		bool ValidateBufferTarget(BufferTarget target);
		bool ValidateBufferName(Uint index);
		bool ValidateBufferUsage(BufferUsage usage);
		bool ValidateBufferMappingAccess(BufferMappingAccessBit accessBits);
	}
}