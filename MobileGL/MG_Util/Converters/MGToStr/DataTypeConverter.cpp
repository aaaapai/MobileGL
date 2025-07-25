#include "../../../Includes.h"

namespace MobileGL {
	namespace MG_Util {
		String ConvertDataTypeToString(DataType type) {
			switch (type) {
				case DataType::Int8: return "Int8";
				case DataType::Uint8: return "Uint8";
				case DataType::Int16: return "Int16";
				case DataType::Uint16: return "Uint16";
				case DataType::Int32: return "Int32";
				case DataType::Uint32: return "Uint32";
				case DataType::Float16: return "Float16";
				case DataType::Float32: return "Float32";
				case DataType::Float64: return "Float64";
				case DataType::Fixed32: return "Fixed32";
				case DataType::Unknown:
				default: return "Unknown";
			}
		}
	}
}