#pragma once
#include "MG_Util/Converters/MGToGL/FramebufferEnumConverter.h"
#include <Includes.h>
#include <MG_State/GLState/FramebufferState/FramebufferObject.h>

namespace MobileGL {
    namespace MG_Util {
        String ConvertFramebufferTargetToString(FramebufferTarget bufferTarget);
        String ConvertFramebufferAttachmentTypeToString(FramebufferAttachmentType attachment);
        String ConvertRenderbufferTargetToString(RenderbufferTarget target);
    } // namespace MG_Util
} // namespace MobileGL