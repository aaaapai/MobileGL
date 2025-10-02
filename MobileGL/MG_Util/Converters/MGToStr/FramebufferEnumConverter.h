#pragma once
#include <Includes.h>
#include <MG_State/GLState/FramebufferState/FramebufferObject.h>

namespace MobileGL {
    namespace MG_Util {
        String ConvertFramebufferTargetToString(FramebufferTarget bufferTarget);
        String ConvertFramebufferAttachmentTypeToString(FramebufferAttachmentType attachment);
    } // namespace MG_Util
} // namespace MobileGL