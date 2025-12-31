// MobileGL - MobileGL/MG_Util/Converters/MGToStr/FramebufferEnumConverter.h
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

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