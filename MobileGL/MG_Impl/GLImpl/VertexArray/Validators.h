// MobileGL - MobileGL/MG_Impl/GLImpl/VertexArray/Validators.h
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_State/GLState/VertexArrayState/VertexArrayObject.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace VertexArrayImpl {
        Bool ValidateVertexArrayName(Uint index);
        Bool ValidateVertexArrayObject(Uint index);
        Bool ValidateVertexAttributeIndex(Uint index);
        Bool ValidateVertexAttribPointerParams(Uint index, SizeT size, DataType type, Int stride);
    } // namespace VertexArrayImpl
} // namespace MobileGL::MG_Impl::GLImpl