// MobileGL - MobileGL/MG_Impl/GLImpl/Sampler/GL_Sampler.h
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
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
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
