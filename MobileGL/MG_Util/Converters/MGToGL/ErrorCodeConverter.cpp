// MobileGL - MobileGL/MG_Util/Converters/MGToGL/ErrorCodeConverter.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "ErrorCodeConverter.h"

namespace MobileGL {
    namespace MG_Util {
        GLenum ConvertErrorCodeToGLEnum(ErrorCode code) {
            switch (code) {
            case ErrorCode::NoError:
                return GL_NO_ERROR;
            case ErrorCode::InvalidEnum:
                return GL_INVALID_ENUM;
            case ErrorCode::InvalidValue:
                return GL_INVALID_VALUE;
            case ErrorCode::InvalidOperation:
                return GL_INVALID_OPERATION;
            case ErrorCode::InvalidFramebufferOperation:
                return GL_INVALID_FRAMEBUFFER_OPERATION;
            case ErrorCode::OutOfMemory:
                return GL_OUT_OF_MEMORY;
            case ErrorCode::StackUnderflow:
                return GL_STACK_UNDERFLOW;
            case ErrorCode::StackOverflow:
                return GL_STACK_OVERFLOW;
            default:
                return GL_NO_ERROR;
            }
        }
    } // namespace MG_Util
} // namespace MobileGL