// MobileGL - MobileGL/MG_Impl/GLImpl/VertexArray/Validators.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "Validators.h"
#include <MG_State/GLState/Core.h>
#include <MG_State/GLState/ErrorState/Error.h>
#include <MG_Util/Converters/MGToGL/DataTypeConverter.h>
#include <MG_Util/Converters/MGToStr/DataTypeConverter.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace VertexArrayImpl {

        Bool ValidateVertexArrayName(Uint index) {
            Bool isValid = MG_State::pGLContext->ValidateVertexArrayName(index);
            if (!isValid) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateVertexArrayName",
                                                 std::format("Vertex array name {} is not valid.", index)));
                return false;
            }
            return true;
        }

        Bool ValidateVertexArrayObject(Uint index) {
            if (!MG_State::pGLContext->ValidateVertexArrayObject(index)) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateVertexArrayObject",
                                                 std::format("Vertex array object {} does not exist.", index)));
                return false;
            }
            return true;
        }

        Bool ValidateVertexAttributeIndex(Uint index) {
            if (index >= MG_State::GLState::VertexArrayObject::MAX_VERTEX_ATTRIBS) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>(
                        "MG_Impl/GLImpl", "ValidateVertexAttributeIndex",
                        std::format("Attribute index {} exceeds maximum of {}.", index,
                                    MG_State::GLState::VertexArrayObject::MAX_VERTEX_ATTRIBS - 1)));
                return false;
            }
            return true;
        }

        Bool ValidateVertexAttribPointerParams(Uint index, SizeT size, DataType type, Int stride) {
            if (size < 1 || size > 4) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>(
                        "MG_Impl/GLImpl", "ValidateVertexAttribPointerParams",
                        std::format("Invalid size {} for attribute {}. Must be 1-4.", size, index)));
                return false;
            }

            if (type == DataType::Unknown) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateVertexAttribPointerParams",
                                                 std::format("Invalid type {} for attribute {}.",
                                                             MG_Util::ConvertDataTypeToString(type).c_str(), index)));
                return false;
            }

            if (stride < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>(
                        "MG_Impl/GLImpl", "ValidateVertexAttribPointerParams",
                        std::format("Negative stride {} is not allowed for attribute {}.", stride, index)));
                return false;
            }

            return true;
        }
    } // namespace VertexArrayImpl
} // namespace MobileGL::MG_Impl::GLImpl