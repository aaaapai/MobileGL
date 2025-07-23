#pragma once

namespace MobileGL {
	namespace MG_Util {
		String ConvertBufferTargetToString(BufferTarget bufferTarget);
		String ConvertBufferUsageToString(BufferUsage usage);
		String ConvertBufferMappingAccessToString(BufferMappingAccessBit access);
	}
}