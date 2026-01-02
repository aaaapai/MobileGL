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
            // 处理 GL_COLOR_ATTACHMENTn
            if (attachment >= GL_COLOR_ATTACHMENT0 && attachment <= GL_COLOR_ATTACHMENT31) {
                return static_cast<FramebufferAttachmentType>(
                    static_cast<SizeT>(FramebufferAttachmentType::Color0) +
                    (attachment - GL_COLOR_ATTACHMENT0));
            }
    
            // 处理默认帧缓冲区的枚举值
            switch (attachment) {
                case GL_NONE:
                    return FramebufferAttachmentType::None;
            
                // 传统默认帧缓冲区枚举
                case GL_FRONT_LEFT:
                    return FramebufferAttachmentType::FrontLeft;
                case GL_FRONT_RIGHT:
                    return FramebufferAttachmentType::FrontRight;
                case GL_BACK_LEFT:
                    return FramebufferAttachmentType::BackLeft;
                case GL_BACK_RIGHT:
                    return FramebufferAttachmentType::BackRight;
            
                // 如果 FramebufferAttachmentType 枚举没有以下值，需要添加
                case GL_FRONT:
                    return FramebufferAttachmentType::Front;  // 可能需要添加这个枚举值
                case GL_BACK:
                    return FramebufferAttachmentType::Back;   // 可能需要添加这个枚举值
                case GL_LEFT:
                    return FramebufferAttachmentType::Left;   // 可能需要添加这个枚举值
                case GL_RIGHT:
                    return FramebufferAttachmentType::Right;  // 可能需要添加这个枚举值
                case GL_FRONT_AND_BACK:
                    return FramebufferAttachmentType::FrontAndBack;  // 可能需要添加这个枚举值
            
                case GL_DEPTH_ATTACHMENT:
                    return FramebufferAttachmentType::Depth;
                case GL_STENCIL_ATTACHMENT:
                    return FramebufferAttachmentType::Stencil;
                case GL_DEPTH_STENCIL_ATTACHMENT:
                    return FramebufferAttachmentType::DepthStencil;
            
                // 其他可能的附件
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
