#pragma once
#include <Includes.h>
#include <MG_State/GLState/SamplerState/SamplerObject.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace SamplerImpl {
        Bool ValidateSamplerName(GLuint sampler);
        Bool ValidateSamplerObject(GLuint sampler);
        Bool ValidateSamplerParam(GLenum pname, GLenum param);
        Bool ValidateSamplerFloatParam(GLenum pname, GLfloat param);
        Bool ValidateSamplerIntParam(GLenum pname, GLint param);
    } // namespace SamplerImpl
} // namespace MobileGL::MG_Impl::GLImpl
