#include "FramebufferEnumConverter.h"

namespace MobileGL {
    namespace MG_Util {
        GLenum ConvertFramebufferTargetToGLEnum(FramebufferTarget target) {
            switch (target) {
            case FramebufferTarget::Draw:
                return GL_DRAW_FRAMEBUFFER;
            case FramebufferTarget::Read:
                return GL_READ_FRAMEBUFFER;
            default:
                return GL_DRAW_FRAMEBUFFER;
            }
        }

        GLenum ConvertFramebufferAttachmentTypeToGLEnum(FramebufferAttachmentType type) {
            if (static_cast<Int>(type) >= static_cast<Int>(FramebufferAttachmentType::Color0) &&
                static_cast<Int>(type) <= static_cast<Int>(FramebufferAttachmentType::Color31)) {
                return GL_COLOR_ATTACHMENT0 +
                       (static_cast<GLenum>(type) - static_cast<GLenum>(FramebufferAttachmentType::Color0));
            }

            switch (type) {
            case FramebufferAttachmentType::Depth:
                return GL_DEPTH_ATTACHMENT;
            case FramebufferAttachmentType::Stencil:
                return GL_STENCIL_ATTACHMENT;
            default:
                return GL_COLOR_ATTACHMENT0;
            }
        }

    } // namespace MG_Util
} // namespace MobileGL