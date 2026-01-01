// MobileGL - MobileGL/MG_Util/Converters/MGToGL/DataTypeConverter.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "DataTypeConverter.h"

namespace MobileGL {
    namespace MG_Util {
        GLenum ConvertDataTypeToGLEnum(DataType type) {
            switch (type) {
            case DataType::Int8:
                return GL_BYTE;
            case DataType::Uint8:
                return GL_UNSIGNED_BYTE;
            case DataType::Int16:
                return GL_SHORT;
            case DataType::Uint16:
                return GL_UNSIGNED_SHORT;
            case DataType::Int32:
                return GL_INT;
            case DataType::Uint32:
                return GL_UNSIGNED_INT;
            case DataType::Float16:
                return GL_HALF_FLOAT;
            case DataType::Float32:
                return GL_FLOAT;
            case DataType::Float64:
                return GL_DOUBLE;
            default:
                return GL_UNKNOWN_MGL;
            }
        }
    } // namespace MG_Util
} // namespace MobileGL