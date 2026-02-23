// MobileGL - MobileGL/MG_Impl/GLImpl/Sampler/Validators.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_State/GLState/SamplerState/SamplerObject.h>

namespace MobileGL::MG_Impl::GLImpl::SamplerImpl {
    Bool ValidateSamplerName(GLuint sampler);
    Bool ValidateSamplerObject(GLuint sampler);
    Bool ValidateSamplerParam(GLenum pname, GLenum param);
    Bool ValidateSamplerFloatParam(GLenum pname, GLfloat param);
    Bool ValidateSamplerIntParam(GLenum pname, GLint param);
} // namespace MobileGL::MG_Impl::GLImpl::SamplerImpl
