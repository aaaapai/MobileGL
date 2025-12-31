// MobileGL - MobileGL/MG_Util/Converters/GLToMG/FramebufferEnumConverter.h
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
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