// MobileGL - MobileGL/MG_Impl/GLImpl/Sampler/Validators.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "Validators.h"
#include <MG_State/GLState/Core.h>
#include <MG_State/GLState/ErrorState/Error.h>
#include <MG_Util/Converters/GLToMG/TextureEnumConverter.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace SamplerImpl {
        Bool ValidateSamplerName(GLuint sampler) {
            if (!MG_State::pGLContext->ValidateSamplerName(sampler)) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateSamplerName",
                                                 std::format("Invalid sampler name {}", sampler)));
                return false;
            }
            return true;
        }

        Bool ValidateSamplerObject(GLuint sampler) {
            if (!MG_State::pGLContext->ValidateSamplerObject(sampler)) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateSamplerObject",
                                                 std::format("Sampler object {} does not exist", sampler)));
                return false;
            }
            return true;
        }

        Bool ValidateSamplerParam(GLenum pname, GLenum param) {
            using namespace MG_Util;
            switch (pname) {
            case GL_TEXTURE_WRAP_S:
            case GL_TEXTURE_WRAP_T:
            case GL_TEXTURE_WRAP_R:
                if (MG_Util::ConvertGLEnumToSamplerWrapMode(param) == SamplerWrapMode::Unknown) {
                    MG_State::pGLContext->RecordError(
                        ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateSamplerParam",
                                                                             "Invalid wrap mode parameter"));
                    return false;
                }
                break;

            case GL_TEXTURE_MIN_FILTER:
                if (MG_Util::ConvertGLEnumToSamplerFilterMode(param) == SamplerFilterMode::Unknown) {
                    MG_State::pGLContext->RecordError(
                        ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateSamplerParam",
                                                                             "Invalid min filter parameter"));
                    return false;
                }
                break;

            case GL_TEXTURE_MAG_FILTER:
                if (param != GL_NEAREST && param != GL_LINEAR) {
                    MG_State::pGLContext->RecordError(
                        ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateSamplerParam",
                                                                             "Invalid mag filter parameter"));
                    return false;
                }
                break;

            case GL_TEXTURE_COMPARE_MODE:
                if (param != GL_NONE && param != GL_COMPARE_REF_TO_TEXTURE) {
                    MG_State::pGLContext->RecordError(
                        ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateSamplerParam",
                                                                             "Invalid compare mode parameter"));
                    return false;
                }
                break;

            case GL_TEXTURE_COMPARE_FUNC:
                if (param < GL_LEQUAL || param > GL_ALWAYS) {
                    MG_State::pGLContext->RecordError(
                        ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateSamplerParam",
                                                                             "Invalid compare function parameter"));
                    return false;
                }
                break;

            default:
                MG_State::pGLContext->RecordError(ErrorCode::InvalidEnum,
                                                  MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateSamplerParam",
                                                                               "Invalid pname for sampler parameter"));
                return false;
            }
            return true;
        }

        Bool ValidateSamplerFloatParam(GLenum pname, GLfloat param) {
            switch (pname) {
            case GL_TEXTURE_MIN_LOD:
            case GL_TEXTURE_MAX_LOD:
            case GL_TEXTURE_LOD_BIAS:
                return true;

            case GL_TEXTURE_BORDER_COLOR:
                if (param < 0.0f || param > 1.0f) {
                    MG_State::pGLContext->RecordError(
                        ErrorCode::InvalidValue,
                        MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateSamplerFloatParam",
                                                     "Border color component out of [0,1] range"));
                    return false;
                }
                return true;

            default:
                return ValidateSamplerParam(pname, static_cast<GLenum>(param));
            }
        }

        Bool ValidateSamplerIntParam(GLenum pname, GLint param) {
            switch (pname) {
            case GL_TEXTURE_BORDER_COLOR:
                if (param < 0 || param > 255) {
                    MG_State::pGLContext->RecordError(
                        ErrorCode::InvalidValue,
                        MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "ValidateSamplerIntParam",
                                                     "Border color component out of [0,255] range"));
                    return false;
                }
                return true;

            default:
                return ValidateSamplerParam(pname, static_cast<GLenum>(param));
            }
        }
    } // namespace SamplerImpl
} // namespace MobileGL::MG_Impl::GLImpl
