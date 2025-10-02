#include "Validators.h"
#include <MG_State/GLState/Core.h>
#include <MG_State/GLState/ErrorState/Error.h>
#include <MG_Util/Converters/GLToStr/GLEnumConverter.h>
#include <MG_Util/Converters/MGToGL/FramebufferEnumConverter.h>
#include <MG_Util/Converters/MGToStr/FramebufferEnumConverter.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace FramebufferImpl {
        Bool ValidateFramebufferTarget(FramebufferTarget target) {
            if (target == FramebufferTarget::Unknown) {
                using namespace MG_Util;
                String bufferTargetStr = ConvertFramebufferTargetToString(target);
                String glTargetStr = ConvertGLEnumToString(ConvertFramebufferTargetToGLEnum(target));
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>(
                        "MG_Impl/GLImpl/FramebufferImpl", "ValidateFramebufferTarget",
                        std::format("Target {} ({}) is not valid.", bufferTargetStr, glTargetStr)));
                return false;
            }
            return true;
        }

        Bool ValidateFramebufferName(Uint index, Bool allowZero) {
            if (index == 0 && !allowZero) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl/FramebufferImpl", "ValidateFramebufferName",
                                                 "Framebuffer name 0 is not valid in this situation."));
                return false;
            }
            Bool isValid = MG_State::pGLContext->ValidateFramebufferName(index);
            if (isValid) return true;
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeShared<GenericErrorInfo>("MG_Impl/GLImpl/FramebufferImpl", "ValidateFramebufferName",
                                             std::format("Framebuffer name {} is not valid.", index)));
            return false;
        }

        Bool ValidateFramebufferAttachmentType(FramebufferAttachmentType attachment) {
            if (attachment == FramebufferAttachmentType::Unknown) {
                using namespace MG_Util;
                String attachmentStr = ConvertFramebufferAttachmentTypeToString(attachment);
                String glAttachmentStr = ConvertGLEnumToString(ConvertFramebufferAttachmentTypeToGLEnum(attachment));
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>(
                        "MG_Impl/GLImpl/FramebufferImpl", "ValidateFramebufferAttachmentType",
                        std::format("Attachment type {} ({}) is not valid.", attachmentStr, glAttachmentStr)));
                return false;
            }
            return true;
        }
    } // namespace FramebufferImpl
} // namespace MobileGL::MG_Impl::GLImpl