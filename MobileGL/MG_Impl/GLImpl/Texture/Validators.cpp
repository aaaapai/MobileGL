#include "Validators.h"
#include <MG_State/GLState/Core.h>
#include <MG_State/GLState/ErrorState/Error.h>
#include <MG_Util/Converters/GLToStr/GLEnumConverter.h>
#include <MG_Util/Converters/GLToMG/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToGL/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToStr/TextureEnumConverter.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace TextureImpl {
        Bool ValidateTextureTarget(TextureTarget target) {
            if (target == TextureTarget::Unknown) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateTextureTarget", "Invalid texture target"));
                return false;
            }
            return true;
        }

        Bool ValidateTextureName(GLuint texture, Bool allowZero) {
            if (texture == 0) {
                if (allowZero) return true;

                MG_State::pGLContext->RecordError(ErrorCode::InvalidValue,
                                                  MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateTextureName",
                                                                               "Texture name cannot be zero"));
                return false;
            }

            if (!MG_State::pGLContext->ValidateTextureName(texture)) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateTextureName", "Invalid texture name"));
                return false;
            }
            return true;
        }
    } // namespace TextureImpl
} // namespace MobileGL::MG_Impl::GLImpl
