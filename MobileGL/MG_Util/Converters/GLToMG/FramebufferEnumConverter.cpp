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
    } // namespace MG_Util
} // namespace MobileGL