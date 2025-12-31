// MobileGL - MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.h
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
        void BlendFunc(GLenum sfactor, GLenum dfactor);
        void Viewport(GLint x, GLint y, GLsizei width, GLsizei height);
        void StencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
        void StencilOp(GLenum fail, GLenum zfail, GLenum zpass);
        void StencilMaskSeparate(GLenum face, GLuint mask);
        void StencilMask(GLuint mask);
        void StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
        void StencilFunc(GLenum func, GLint ref, GLuint mask);
        void Scissor(GLint x, GLint y, GLsizei width, GLsizei height);
        void SampleCoverage(GLfloat value, GLboolean invert);
        void PolygonOffset(GLfloat factor, GLfloat units);
        void PolygonMode(GLenum face, GLenum mode);
        void PointSize(GLfloat size);
        void PointParameterf(GLenum pname, GLfloat param);
        void PointParameteri(GLenum pname, GLint param);
        void PixelStorei(GLenum pname, GLint param);
        void LogicOp(GLenum opcode);
        void LineWidth(GLfloat width);
        GLboolean IsEnabledi(GLenum target, GLuint index);
        GLboolean IsEnabled(GLenum cap);
        void Hint(GLenum target, GLenum mode);
        void FrontFace(GLenum mode);
        void Enable(GLenum cap);
        void Disable(GLenum cap);
        void DepthRange(GLclampd near_val, GLclampd far_val);
        void DepthMask(GLboolean flag);
        void DepthFunc(GLenum func);
        void CullFace(GLenum mode);
        void ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
        void ClampColor(GLenum target, GLenum clamp);
        void BlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
        void BlendEquation(GLenum mode);
        void BlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
        void ReadBuffer(GLenum src);
        void ClearStencil(GLint s);
        void ClearDepth(GLclampd depth);
        void ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
