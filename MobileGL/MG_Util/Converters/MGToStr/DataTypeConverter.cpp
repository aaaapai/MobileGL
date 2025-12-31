// MobileGL - MobileGL/MG_Util/Converters/MGToStr/DataTypeConverter.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "DataTypeConverter.h"

namespace MobileGL {
    namespace MG_Util {
        String ConvertDataTypeToString(DataType type) {
            switch (type) {
            case DataType::Int8:
                return "Int8";
            case DataType::Uint8:
                return "Uint8";
            case DataType::Int16:
                return "Int16";
            case DataType::Uint16:
                return "Uint16";
            case DataType::Int32:
                return "Int32";
            case DataType::Uint32:
                return "Uint32";
            case DataType::Float16:
                return "Float16";
            case DataType::Float32:
                return "Float32";
            case DataType::Float64:
                return "Float64";
            case DataType::Fixed32:
                return "Fixed32";
            case DataType::Unknown:
            default:
                return "Unknown";
            }
        }
    } // namespace MG_Util
} // namespace MobileGL