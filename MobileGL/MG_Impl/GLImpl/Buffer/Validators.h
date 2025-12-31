// MobileGL - MobileGL/MG_Impl/GLImpl/Buffer/Validators.h
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_State/GLState/BufferState/BufferObject.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace BufferImpl {
        Bool ValidateBufferTarget(BufferTarget target);
        Bool ValidateBufferName(Uint index, Bool allowZero = false);
        Bool ValidateBufferUsage(BufferUsage usage);
        Bool ValidateBufferMappingAccess(Flags<BufferMappingAccessBit> accessBits);
        Bool ValidateBufferBindingPointTarget(BufferTarget target);
    } // namespace BufferImpl
} // namespace MobileGL::MG_Impl::GLImpl