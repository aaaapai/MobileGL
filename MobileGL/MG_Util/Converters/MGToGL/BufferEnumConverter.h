#pragma once

namespace MobileGL {
	namespace MG_Util {
		GLenum ConvertBufferTargetToGLEnum(BufferTarget bufferTarget);
		GLenum ConvertBufferUsageToGLEnum(BufferUsage usage);
		GLbitfield ConvertBufferMappingAccessToGLEnum(BufferMappingAccessBit access);
	}
}