// MobileGL - MobileGL/MG_Impl/GLImpl/Sampler/GL_Sampler.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MobileGL::MG_Impl::GLImpl {
    /* @INSERTION_POINT:FUNCTION_DECLARATION@ */
    void GetSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params);
    void SamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint* param);
    void SamplerParameterIiv(GLuint sampler, GLenum pname, const GLint* param);
    void SamplerParameteriv(GLuint sampler, GLenum pname, const GLint* param);
    void SamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* param);
    void SamplerParameteri(GLuint sampler, GLenum pname, GLint param);
    void SamplerParameterf(GLuint sampler, GLenum pname, GLfloat param);
    GLboolean IsSampler(GLuint sampler);
    void GetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint* params);
    void GetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint* params);
    void GetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params);
    void GenSamplers(GLsizei count, GLuint* samplers);
    void DeleteSamplers(GLsizei count, const GLuint* samplers);
    void CreateSamplers(GLsizei n, GLuint* samplers);
    void BindSamplers(GLuint first, GLsizei count, const GLuint* samplers);
    void BindSampler(GLuint unit, GLuint sampler);
} // namespace MobileGL::MG_Impl::GLImpl
