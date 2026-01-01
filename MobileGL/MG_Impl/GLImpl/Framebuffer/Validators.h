// MobileGL - MobileGL/MG_Impl/GLImpl/Framebuffer/Validators.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
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