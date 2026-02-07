// MobileGL - MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "GL_RenderState.h"
#include "MG_State/GLState/RenderState/RenderState.h"
#include "MG_Util/Converters/GLToStr/GLEnumConverter.h"
#include <MG_State/GLState/Core.h>
#include <MG_Util/Converters/GLToMG/RenderStateEnumConverter.h>
#include <MG_Util/Converters/MGToGL/RenderStateEnumConverter.h>
#include <MG_Util/Converters/MGToStr/RenderStateEnumConverter.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        void Viewport_State(GLint x, GLint y, GLsizei width, GLsizei height) {
            if (width < 0 || height < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "Viewport_State",
                                                                          "Width abd height must be non-negative."));
                return;
            }

            MG_State::pGLContext->SetViewport(IntVec4(x, y, width, height));
        }

        void StencilOpSeparate_State(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
            // TODO: implement
        }

        void StencilOp_State(GLenum fail, GLenum zfail, GLenum zpass) {
            // TODO: implement
        }

        void StencilMaskSeparate_State(GLenum face, GLuint mask) {
            // TODO: implement
        }

        void StencilMask_State(GLuint mask) {
            // TODO: implement
        }

        void StencilFuncSeparate_State(GLenum face, GLenum func, GLint ref, GLuint mask) {
            // TODO: implement
        }

        void StencilFunc_State(GLenum func, GLint ref, GLuint mask) {
            // TODO: implement
        }

        void Scissor_State(GLint x, GLint y, GLsizei width, GLsizei height) {
            if (width < 0 || height < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "Scissor_State",
                                                                          "Width abd height must be non-negative."));
                return;
            }

            MG_State::pGLContext->SetScissorBox(IntVec4(x, y, width, height));
        }

        void SampleCoverage_State(GLfloat value, GLboolean invert) {
            // TODO: implement
        }

        void PolygonOffset_State(GLfloat factor, GLfloat units) {
            // TODO: implement
        }

        void PolygonMode_State(GLenum face, GLenum mode) {
            // TODO: implement
        }

        void PointSize_State(GLfloat size) {
            // TODO: implement
        }

        void PointParameterf_State(GLenum pname, GLfloat param) {
            // TODO: implement
        }

        void PointParameteri_State(GLenum pname, GLint param) {
            // TODO: implement
        }

        void PixelStorei_State(GLenum pname, GLint param) {
            MGLOG_D("%s: %s = %d", __func__, MG_Util::ConvertGLEnumToString(pname).c_str(), param);
            PixelStoreParam pixelStoreParam = MG_Util::ConvertGLEnumToPixelStoreParam(pname);
            if (pixelStoreParam == PixelStoreParam::Unknown) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "PixelStorei_State",
                                                 "Pixel store param enum " +
                                                     MG_Util::ConvertPixelStoreParamToString(pixelStoreParam) + "(" +
                                                     MG_Util::ConvertGLEnumToString(pname) + ") is not supported."));
                return;
            }

            MG_State::pGLContext->SetPixelStoreParam(pixelStoreParam, param);
        }

        void LogicOp_State(GLenum opcode) {
            // TODO: implement
        }

        void LineWidth_State(GLfloat width) {
            // TODO: implement
        }

        GLboolean IsEnabledi_State(GLenum target, GLuint index) {
            CapabilityInput capInput = MG_Util::ConvertGLEnumToCapabilityInput(target);
            if (capInput == CapabilityInput::Unknown) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "IsEnabledi_State",
                                                 "Capability enum " +
                                                     MG_Util::ConvertCapabilityInputToString(capInput) + "(" +
                                                     MG_Util::ConvertGLEnumToString(target) + ") is not supported."));
                return GL_FALSE;
            }

            return MG_State::pGLContext->IsCapabilityEnabledIndexed(capInput, index) ? GL_TRUE : GL_FALSE;
        }

        GLboolean IsEnabled_State(GLenum cap) {
            CapabilityInput capInput = MG_Util::ConvertGLEnumToCapabilityInput(cap);
            if (capInput == CapabilityInput::Unknown) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>(
                                                "MG_Impl/GLImpl", "IsEnabled_State",
                                                "Capability enum " + MG_Util::ConvertCapabilityInputToString(capInput) +
                                                    "(" + MG_Util::ConvertGLEnumToString(cap) + ") is not supported."));
                return GL_FALSE;
            }

            return MG_State::pGLContext->IsCapabilityEnabled(capInput) ? GL_TRUE : GL_FALSE;
        }

        void Hint_State(GLenum target, GLenum mode) {
            // TODO: implement
        }

        void FrontFace_State(GLenum mode) {
            // TODO: implement
        }

        void Enable_State(GLenum cap) {
            CapabilityInput capInput = MG_Util::ConvertGLEnumToCapabilityInput(cap);
            if (capInput == CapabilityInput::Unknown) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>(
                                                "MG_Impl/GLImpl", "Enable_State",
                                                "Capability enum " + MG_Util::ConvertCapabilityInputToString(capInput) +
                                                    "(" + MG_Util::ConvertGLEnumToString(cap) + ") is not supported."));
                return;
            }

            MG_State::pGLContext->SetCapability(capInput, true);
        }

        void Disable_State(GLenum cap) {
            CapabilityInput capInput = MG_Util::ConvertGLEnumToCapabilityInput(cap);
            if (capInput == CapabilityInput::Unknown) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>(
                                                "MG_Impl/GLImpl", "Disable_State",
                                                "Capability enum " + MG_Util::ConvertCapabilityInputToString(capInput) +
                                                    "(" + MG_Util::ConvertGLEnumToString(cap) + ") is not supported."));
                return;
            }

            MG_State::pGLContext->SetCapability(capInput, false);
        }

        void DepthRange_State(GLclampd near_val, GLclampd far_val) {
            // TODO: implement
        }

        void DepthMask_State(GLboolean flag) {
            MG_State::pGLContext->SetDepthMask(flag == GL_TRUE);
        }

        void DepthFunc_State(GLenum func) {
            DepthTestFunc depthFunc = MG_Util::ConvertGLEnumToDepthTestFunc(func);
            if (depthFunc == DepthTestFunc::Unknown) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "DepthFunc_State",
                                                 "Depth function enum " +
                                                     MG_Util::ConvertDepthTestFuncToString(depthFunc) + "(" +
                                                     MG_Util::ConvertGLEnumToString(func) + ") is not supported."));
                return;
            }

            MG_State::pGLContext->SetDepthFunc(depthFunc);
        }

        void CullFace_State(GLenum mode) {
            CullFaceMode cullFaceMode = MG_Util::ConvertGLEnumToCullFaceMode(mode);
            if (cullFaceMode == CullFaceMode::Unknown) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "CullFace_State",
                                                 "Cull face mode enum " +
                                                     MG_Util::ConvertCullFaceModeToString(cullFaceMode) + "(" +
                                                     MG_Util::ConvertGLEnumToString(mode) + ") is not supported."));
                return;
            }

            MG_State::pGLContext->SetCullFaceMode(cullFaceMode);
        }

        void ColorMask_State(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
            MG_State::pGLContext->SetColorMask(
                BoolVec4(red == GL_TRUE, green == GL_TRUE, blue == GL_TRUE, alpha == GL_TRUE));
        }

        void ClampColor_State(GLenum target, GLenum clamp) {
            // TODO: implement
        }

        void BlendFuncSeparate_State(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
            BlendFactor srcRGB = MG_Util::ConvertGLEnumToBlendFactor(sfactorRGB);
            BlendFactor dstRGB = MG_Util::ConvertGLEnumToBlendFactor(dfactorRGB);
            BlendFactor srcAlpha = MG_Util::ConvertGLEnumToBlendFactor(sfactorAlpha);
            BlendFactor dstAlpha = MG_Util::ConvertGLEnumToBlendFactor(dfactorAlpha);

            if (srcRGB == BlendFactor::Unknown || dstRGB == BlendFactor::Unknown || srcAlpha == BlendFactor::Unknown ||
                dstAlpha == BlendFactor::Unknown) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "BlendFuncSeparate_State",
                                                 "One of the blend factor enums is not supported: srcRGB " +
                                                     MG_Util::ConvertBlendFactorToString(srcRGB) + "(" +
                                                     MG_Util::ConvertGLEnumToString(sfactorRGB) + "), dstRGB " +
                                                     MG_Util::ConvertBlendFactorToString(dstRGB) + "(" +
                                                     MG_Util::ConvertGLEnumToString(dfactorRGB) + "), srcAlpha " +
                                                     MG_Util::ConvertBlendFactorToString(srcAlpha) + "(" +
                                                     MG_Util::ConvertGLEnumToString(sfactorAlpha) + "), dstAlpha " +
                                                     MG_Util::ConvertBlendFactorToString(dstAlpha) + "(" +
                                                     MG_Util::ConvertGLEnumToString(dfactorAlpha) + ")."));
                return;
            }

            MG_State::pGLContext->SetBlendFunc(srcRGB, dstRGB, srcAlpha, dstAlpha);
        }

        void BlendFunc_State(GLenum sfactor, GLenum dfactor) {
            BlendFuncSeparate_State(sfactor, dfactor, sfactor, dfactor);
        }

        void BlendEquation_State(GLenum mode) {
            // TODO: implement
        }

        void BlendColor_State(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
            // TODO: implement
        }

        void ClearStencil_State(GLint s) {
            // TODO: implement
        }

        void ClearDepth_State(GLclampd depth) {
            MG_State::pGLContext->SetClearDepth(static_cast<Float>(depth));
        }

        void ClearColor_State(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
            MG_State::pGLContext->SetClearColor(FloatVec4(red, green, blue, alpha));
        }

        void BlendFuncSeparatei_State(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
            if (buf >= MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>(
                        "MG_Impl/GLImpl", "BlendFuncSeparatei_State",
                        "Buffer index " + std::to_string(buf) + " is out of range. Max supported is " +
                            std::to_string(MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS - 1) + "."));
                return;
            }

            BlendFactor srcRGBM = MG_Util::ConvertGLEnumToBlendFactor(srcRGB);
            BlendFactor dstRGBM = MG_Util::ConvertGLEnumToBlendFactor(dstRGB);
            BlendFactor srcAlphaM = MG_Util::ConvertGLEnumToBlendFactor(srcAlpha);
            BlendFactor dstAlphaM = MG_Util::ConvertGLEnumToBlendFactor(dstAlpha);
            MG_State::pGLContext->SetBlendFuncIndexed(buf, srcRGBM, dstRGBM, srcAlphaM, dstAlphaM);
        }

        void Disablei_State(GLenum target, GLuint index) {
            auto capInput = MG_Util::ConvertGLEnumToCapabilityInput(target);
            if (capInput == CapabilityInput::Unknown) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "Disablei_State",
                                                 "Capability enum " +
                                                     MG_Util::ConvertCapabilityInputToString(capInput) + "(" +
                                                     MG_Util::ConvertGLEnumToString(target) + ") is not supported."));
                return;
            }

            MG_State::pGLContext->SetCapabilityIndexed(capInput, index, false);
        }

        void Enablei_State(GLenum target, GLuint index) {
            auto capInput = MG_Util::ConvertGLEnumToCapabilityInput(target);
            if (capInput == CapabilityInput::Unknown) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "Enablei_State",
                                                 "Capability enum " +
                                                     MG_Util::ConvertCapabilityInputToString(capInput) + "(" +
                                                     MG_Util::ConvertGLEnumToString(target) + ") is not supported."));
                return;
            }

            MG_State::pGLContext->SetCapabilityIndexed(capInput, index, true);
        }

        /* @INSERTION_POINT:FUNCTION_IMPLEMENTATION@ */
        void BlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
            BlendFuncSeparatei_State(buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
        }

        void Disablei(GLenum target, GLuint index) {
            Disablei_State(target, index);
        }

        void Enablei(GLenum target, GLuint index) {
            Enablei_State(target, index);
        }

        void BlendFunc(GLenum sfactor, GLenum dfactor) {
            BlendFunc_State(sfactor, dfactor);
        }

        void Viewport(GLint x, GLint y, GLsizei width, GLsizei height) {
            Viewport_State(x, y, width, height);
        }

        void StencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
            StencilOpSeparate_State(face, sfail, dpfail, dppass);
        }

        void StencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
            StencilOp_State(fail, zfail, zpass);
        }

        void StencilMaskSeparate(GLenum face, GLuint mask) {
            StencilMaskSeparate_State(face, mask);
        }

        void StencilMask(GLuint mask) {
            StencilMask_State(mask);
        }

        void StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) {
            StencilFuncSeparate_State(face, func, ref, mask);
        }

        void StencilFunc(GLenum func, GLint ref, GLuint mask) {
            StencilFunc_State(func, ref, mask);
        }

        void Scissor(GLint x, GLint y, GLsizei width, GLsizei height) {
            Scissor_State(x, y, width, height);
        }

        void SampleCoverage(GLfloat value, GLboolean invert) {
            SampleCoverage_State(value, invert);
        }

        void PolygonOffset(GLfloat factor, GLfloat units) {
            PolygonOffset_State(factor, units);
        }

        void PolygonMode(GLenum face, GLenum mode) {
            PolygonMode_State(face, mode);
        }

        void PointSize(GLfloat size) {
            PointSize_State(size);
        }

        void PointParameterf(GLenum pname, GLfloat param) {
            PointParameterf_State(pname, param);
        }

        void PointParameteri(GLenum pname, GLint param) {
            PointParameteri_State(pname, param);
        }

        void PixelStorei(GLenum pname, GLint param) {
            PixelStorei_State(pname, param);
        }

        void LogicOp(GLenum opcode) {
            LogicOp_State(opcode);
        }

        void LineWidth(GLfloat width) {
            LineWidth_State(width);
        }

        GLboolean IsEnabledi(GLenum target, GLuint index) {
            return IsEnabledi_State(target, index);
        }

        GLboolean IsEnabled(GLenum cap) {
            return IsEnabled_State(cap);
        }

        void Hint(GLenum target, GLenum mode) {
            Hint_State(target, mode);
        }

        void FrontFace(GLenum mode) {
            FrontFace_State(mode);
        }

        void Enable(GLenum cap) {
            Enable_State(cap);
        }

        void Disable(GLenum cap) {
            Disable_State(cap);
        }

        void DepthRange(GLclampd near_val, GLclampd far_val) {
            DepthRange_State(near_val, far_val);
        }

        void DepthMask(GLboolean flag) {
            DepthMask_State(flag);
        }

        void DepthFunc(GLenum func) {
            DepthFunc_State(func);
        }

        void CullFace(GLenum mode) {
            CullFace_State(mode);
        }

        void ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
            ColorMask_State(red, green, blue, alpha);
        }

        void ClampColor(GLenum target, GLenum clamp) {
            ClampColor_State(target, clamp);
        }

        void BlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
            BlendFuncSeparate_State(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
        }

        void BlendEquation(GLenum mode) {
            BlendEquation_State(mode);
        }

        void BlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
            BlendColor_State(red, green, blue, alpha);
        }

        void ClearStencil(GLint s) {
            ClearStencil_State(s);
        }

        void ClearDepth(GLclampd depth) {
            ClearDepth_State(depth);
        }

        void ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
            ClearColor_State(red, green, blue, alpha);
        }
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
