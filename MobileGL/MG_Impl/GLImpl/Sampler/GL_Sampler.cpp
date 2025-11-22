#include "GL_Sampler.h"
#include "Validators.h"
#include <MG_State/GLState/Core.h>
#include <MG_Util/Converters/GLToMG/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToGL/TextureEnumConverter.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        void SetSamplerParam_State(GLuint sampler, GLenum pname, const void* param, bool isFloat, bool isInteger) {
            if (!SamplerImpl::ValidateSamplerName(sampler)) return;

            auto samplerObj = MG_State::pGLContext->GetSamplerObject(sampler);
            if (!samplerObj) {
                samplerObj = MG_State::pGLContext->CreateSamplerObject(sampler); // for compatibility
            }
            if (!SamplerImpl::ValidateSamplerObject(sampler)) return;

            using namespace MG_Util;
            switch (pname) {
            case GL_TEXTURE_WRAP_S:
                samplerObj->SetWrapS(MG_Util::ConvertGLEnumToSamplerWrapMode(*(const GLint*)param));
                break;
            case GL_TEXTURE_WRAP_T:
                samplerObj->SetWrapT(MG_Util::ConvertGLEnumToSamplerWrapMode(*(const GLint*)param));
                break;
            case GL_TEXTURE_WRAP_R:
                samplerObj->SetWrapR(MG_Util::ConvertGLEnumToSamplerWrapMode(*(const GLint*)param));
                break;
            case GL_TEXTURE_MIN_FILTER:
                samplerObj->SetMinFilter(MG_Util::ConvertGLEnumToSamplerFilterMode(*(const GLint*)param));
                samplerObj->SetMipmapMode(MG_Util::ConvertGLEnumToSamplerMipmapMode(*(const GLint*)param));
                break;
            case GL_TEXTURE_MAG_FILTER:
                samplerObj->SetMagFilter(MG_Util::ConvertGLEnumToSamplerFilterMode(*(const GLint*)param));
                break;
            case GL_TEXTURE_MIN_LOD:
                samplerObj->SetLodRange(*(const GLfloat*)param, samplerObj->GetMaxLod());
                break;
            case GL_TEXTURE_MAX_LOD:
                samplerObj->SetLodRange(samplerObj->GetMinLod(), *(const GLfloat*)param);
                break;
            case GL_TEXTURE_LOD_BIAS:
                samplerObj->SetLodBias(*(const GLfloat*)param);
                break;
            case GL_TEXTURE_COMPARE_MODE:
                samplerObj->SetCompareMode(MG_Util::ConvertGLEnumToSamplerCompareMode(*(const GLint*)param));
                break;
            case GL_TEXTURE_COMPARE_FUNC:
                samplerObj->SetSamplerCompareFunc(MG_Util::ConvertGLEnumToSamplerCompareFunc(*(const GLint*)param));
                break;
            default:
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "SetSamplerParam_State",
                                                                         "Invalid pname for sampler parameter"));
            }
        }

        void GetSamplerParam_State(GLuint sampler, GLenum pname, void* params, bool isFloat, bool isInteger) {
            if (!SamplerImpl::ValidateSamplerName(sampler)) return;

            auto samplerObj = MG_State::pGLContext->GetSamplerObject(sampler);
            if (!samplerObj) {
                samplerObj = MG_State::pGLContext->CreateSamplerObject(sampler); // for compatibility
            }
            if (!SamplerImpl::ValidateSamplerObject(sampler)) return;

            using namespace MG_Util;
            switch (pname) {
            case GL_TEXTURE_WRAP_S:
                *(GLint*)params = MG_Util::ConvertSamplerWrapModeToGLEnum(samplerObj->GetWrapS());
                break;
            case GL_TEXTURE_WRAP_T:
                *(GLint*)params = MG_Util::ConvertSamplerWrapModeToGLEnum(samplerObj->GetWrapT());
                break;
            case GL_TEXTURE_WRAP_R:
                *(GLint*)params = MG_Util::ConvertSamplerWrapModeToGLEnum(samplerObj->GetWrapR());
                break;
            case GL_TEXTURE_MIN_FILTER:
                *(GLint*)params = MG_Util::ConvertSamplerFilterModeToGLEnum(samplerObj->GetMinFilter(), samplerObj->GetMipmapMode());
                break;
            case GL_TEXTURE_MAG_FILTER:
                *(GLint*)params = MG_Util::ConvertSamplerFilterModeToGLEnum(samplerObj->GetMagFilter(), SamplerMipmapMode::None);
                break;
            case GL_TEXTURE_MIN_LOD:
                *(GLfloat*)params = samplerObj->GetMinLod();
                break;
            case GL_TEXTURE_MAX_LOD:
                *(GLfloat*)params = samplerObj->GetMaxLod();
                break;
            case GL_TEXTURE_LOD_BIAS:
                *(GLfloat*)params = samplerObj->GetLodBias();
                break;
            case GL_TEXTURE_COMPARE_MODE:
                *(GLint*)params = MG_Util::ConvertSamplerCompareModeToGLEnum(samplerObj->GetCompareMode());
                break;
            case GL_TEXTURE_COMPARE_FUNC:
                *(GLint*)params = MG_Util::ConvertSamplerCompareFuncToGLEnum(samplerObj->GetSamplerCompareFunc());
                break;
            default:
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "GetSamplerParam_State",
                                                                         "Invalid pname for sampler parameter"));
            }
        }

        GLboolean IsSampler_State(GLuint sampler) {
            return MG_State::pGLContext->ValidateSamplerObject(sampler) ? GL_TRUE : GL_FALSE;
        }

        // migrate below functions without "_State" into this section
        void GenSamplers_State(GLsizei count, GLuint* samplers) {
            if (count < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "GenSamplers", "count must be non-negative"));
                return;
            }

            auto names = MG_State::pGLContext->GenSamplerNames(count);
            for (GLsizei i = 0; i < count; ++i) {
                samplers[i] = names[i];
            }
        }

        void DeleteSamplers_State(GLsizei count, const GLuint* samplers) {
            if (count < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "DeleteSamplers", "count must be non-negative"));
                return;
            }

            for (GLsizei i = 0; i < count; ++i) {
                if (samplers[i] != 0) {
                    MG_State::pGLContext->MarkSamplerObjectForDeletion(samplers[i]);
                }
            }
        }

        void CreateSamplers_State(GLsizei n, GLuint* samplers) {
            if (n < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "GenSamplers", "count must be non-negative"));
                return;
            }

            auto names = MG_State::pGLContext->GenSamplerNames(n);
            for (GLsizei i = 0; i < n; ++i) {
                samplers[i] = names[i];
                MG_State::pGLContext->CreateSamplerObject(names[i]);
            }
        }

        void BindSampler_State(GLuint unit, GLuint sampler) {
            if (unit >= MG_State::GLState::TextureState::MAX_TEXTURE_IMAGE_UNITS) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "BindSampler", "texture unit out of range"));
                return;
            }

            auto& textureUnit = MG_State::pGLContext->GetTextureUnitObject(unit);
            if (sampler == 0) {
                textureUnit.SetSamplerObject(nullptr);
            } else {
                if (!SamplerImpl::ValidateSamplerName(sampler)) return;
                auto samplerObject = MG_State::pGLContext->GetSamplerObject(sampler);
                if (!samplerObject) {
                    samplerObject = MG_State::pGLContext->CreateSamplerObject(sampler);
                }

                textureUnit.SetSamplerObject(MG_State::pGLContext->GetSamplerObject(sampler));
            }
        }

        void BindSamplers_State(GLuint first, GLsizei count, const GLuint* samplers) {
            if (count < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "BindSamplers", "count must be non-negative"));
                return;
            }

            for (GLsizei i = 0; i < count; ++i) {
                BindSampler_State(first + i, samplers ? samplers[i] : 0);
            }
        }

        /* @INSERTION_POINT:FUNCTION_IMPLEMENTATION@ */
        void GetSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params) {
            GetSamplerParam_State(sampler, pname, params, false, false);
        }

        void SamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint* param) {
            SetSamplerParam_State(sampler, pname, param, false, true);
        }

        void SamplerParameterIiv(GLuint sampler, GLenum pname, const GLint* param) {
            SetSamplerParam_State(sampler, pname, param, false, true);
        }

        void SamplerParameteriv(GLuint sampler, GLenum pname, const GLint* param) {
            SetSamplerParam_State(sampler, pname, param, false, false);
        }

        void SamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* param) {
            SetSamplerParam_State(sampler, pname, param, true, false);
        }

        void SamplerParameteri(GLuint sampler, GLenum pname, GLint param) {
            SamplerParameteriv(sampler, pname, &param);
        }

        void SamplerParameterf(GLuint sampler, GLenum pname, GLfloat param) {
            SamplerParameterfv(sampler, pname, &param);
        }

        GLboolean IsSampler(GLuint sampler) {
            return IsSampler_State(sampler);
        }

        void GetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint* params) {
            GetSamplerParam_State(sampler, pname, params, false, true);
        }

        void GetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint* params) {
            GetSamplerParam_State(sampler, pname, params, false, true);
        }

        void GetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params) {
            GetSamplerParam_State(sampler, pname, params, true, false);
        }

        void GenSamplers(GLsizei count, GLuint* samplers) {
            GenSamplers_State(count, samplers);
        }

        void DeleteSamplers(GLsizei count, const GLuint* samplers) {
            DeleteSamplers_State(count, samplers);
        }

        void CreateSamplers(GLsizei n, GLuint* samplers) {
            CreateSamplers_State(n, samplers);
        }

        void BindSamplers(GLuint first, GLsizei count, const GLuint* samplers) {
            BindSamplers_State(first, count, samplers);
        }

        void BindSampler(GLuint unit, GLuint sampler) {
            BindSampler_State(unit, sampler);
        }
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
