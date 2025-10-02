#pragma once
#include <Includes.h>
#include <MG_State/GLState/FramebufferState/FramebufferObject.h>

namespace MobileGL {
    namespace MG_Util {
        FramebufferTarget ConvertGLEnumToFramebufferTarget(GLenum bufferTarget);
        FramebufferAttachmentType ConvertGLEnumToFramebufferAttachmentType(GLenum attachment);
    } // namespace MG_Util
} // namespace MobileGL