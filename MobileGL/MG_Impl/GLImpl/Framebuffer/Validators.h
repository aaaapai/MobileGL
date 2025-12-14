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