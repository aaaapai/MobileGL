// MobileGL - MobileGL/MG_Util/Converters/MGToStr/FramebufferEnumConverter.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
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