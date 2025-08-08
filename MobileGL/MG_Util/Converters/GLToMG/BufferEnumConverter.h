#pragma once

namespace MobileGL {
	namespace MG_Util {
		BufferTarget ConvertGLEnumToBufferTarget(GLenum bufferTarget);
		BufferUsage ConvertGLEnumToBufferUsage(GLenum usage);
		Flags<BufferMappingAccessBit> ConvertGLEnumToBufferMappingAccess(GLbitfield access);
	}
}