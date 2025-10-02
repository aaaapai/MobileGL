#pragma once
#include <Includes.h>
#include <MG_State/GLState/FramebufferState/FramebufferObject.h>

namespace MobileGL {
    namespace MG_Util {
        GLenum ConvertFramebufferTargetToGLEnum(FramebufferTarget bufferTarget);
        GLenum ConvertFramebufferAttachmentTypeToGLEnum(FramebufferAttachmentType attachment);
    } // namespace MG_Util
} // namespace MobileGL