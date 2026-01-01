// MobileGL - MobileGL/MG_Util/Converters/GLToMG/FramebufferEnumConverter.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "FramebufferEnumConverter.h"

namespace MobileGL {
    namespace MG_Util {
        FramebufferTarget ConvertGLEnumToFramebufferTarget(GLenum framebufferTarget) {
            switch (framebufferTarget) {
            case GL_FRAMEBUFFER:
            case GL_DRAW_FRAMEBUFFER:
                return FramebufferTarget::Draw;
            case GL_READ_FRAMEBUFFER:
                return FramebufferTarget::Read;
            case GL_UNKNOWN_MGL:
            default:
                return FramebufferTarget::Unknown;
            }
        }

        FramebufferAttachmentType ConvertGLEnumToFramebufferAttachmentType(GLenum attachment) {
            if (attachment >= GL_COLOR_ATTACHMENT0 && attachment <= GL_COLOR_ATTACHMENT31) {
                return static_cast<FramebufferAttachmentType>(static_cast<SizeT>(FramebufferAttachmentType::Color0) +
                                                              (attachment - GL_COLOR_ATTACHMENT0));
            }

            switch (attachment) {
            case GL_DEPTH_ATTACHMENT:
                return FramebufferAttachmentType::Depth;
            case GL_STENCIL_ATTACHMENT:
                return FramebufferAttachmentType::Stencil;
            case GL_UNKNOWN_MGL:
            default:
                return FramebufferAttachmentType::Unknown;
            }
        }

        RenderbufferTarget ConvertGLEnumToRenderbufferTarget(GLenum renderbufferTarget) {
            switch (renderbufferTarget) {
            case GL_RENDERBUFFER:
                return RenderbufferTarget::Renderbuffer;
            case GL_UNKNOWN_MGL:
            default:
                return RenderbufferTarget::Unknown;
            }
        }
    } // namespace MG_Util
} // namespace MobileGL