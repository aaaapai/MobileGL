// MobileGL - MobileGL/MG_Impl/GLImpl/Framebuffer/Validators.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_State/GLState/FramebufferState/FramebufferObject.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace FramebufferImpl {
        Bool ValidateFramebufferTarget(FramebufferTarget target);
        Bool ValidateFramebufferName(Uint index, Bool allowZero = true);
        Bool ValidateFramebufferAttachmentType(FramebufferAttachmentType attachment);
        Bool ValidateRenderbufferTarget(RenderbufferTarget target);
        Bool ValidateRenderbufferName(Uint index, Bool allowZero = true);
    } // namespace FramebufferImpl
} // namespace MobileGL::MG_Impl::GLImpl