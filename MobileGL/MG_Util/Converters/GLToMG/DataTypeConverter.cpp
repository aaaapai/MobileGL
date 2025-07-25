#include "../../../Includes.h"

namespace MobileGL {
	namespace MG_Util {
		DataType ConvertGLEnumToDataType(GLenum type) {
			switch (type) {
			case GL_BYTE: return DataType::Int8;
			case GL_UNSIGNED_BYTE: return DataType::Uint8;
			case GL_SHORT: return DataType::Int16;
			case GL_UNSIGNED_SHORT: return DataType::Uint16;
			case GL_INT: return DataType::Int32;
			case GL_UNSIGNED_INT: return DataType::Uint32;
			case GL_HALF_FLOAT: return DataType::Float16;
			case GL_FLOAT: return DataType::Float32;
			case GL_DOUBLE: return DataType::Float64;
			default:
				return DataType::Unknown;
			}
		}
	}
}