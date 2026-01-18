// MobileGL - MobileGL/MG_Util/Converters/GLToMG/FramebufferEnumConverter.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_State/GLState/FramebufferState/FramebufferObject.h>

namespace MobileGL {
    namespace MG_Util {
        FramebufferTarget ConvertGLEnumToFramebufferTarget(GLenum bufferTarget);
        FramebufferAttachmentType ConvertGLEnumToFramebufferAttachmentType(GLenum attachment);
        RenderbufferTarget ConvertGLEnumToRenderbufferTarget(GLenum renderbufferTarget);
    } // namespace MG_Util
} // namespace MobileGL