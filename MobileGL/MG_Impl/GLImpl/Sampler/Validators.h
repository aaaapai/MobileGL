// MobileGL - MobileGL/MG_Impl/GLImpl/Sampler/Validators.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

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
