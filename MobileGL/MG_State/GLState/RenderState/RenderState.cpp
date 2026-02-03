// MobileGL - MobileGL/MG_State/GLState/RenderState/RenderState.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "RenderState.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            RenderState::RenderState() {}

            // -------------------- Rasterization --------------------
            void RenderState::SetViewport(IntVec4 viewport) {
                m_parameters.Viewport = viewport;
            }

            const IntVec4& RenderState::GetViewport() const {
                return m_parameters.Viewport;
            }

            // -------------------- Capabilities --------------------
            void RenderState::SetCapability(CapabilityInput cap, Bool enabled) {
                switch (cap) {
                case CapabilityInput::Blend:
                    m_parameters.BlendEnabled = enabled;
                    break;
                case CapabilityInput::DepthTest:
                    m_parameters.DepthTestEnabled = enabled;
                    break;
                case CapabilityInput::CullFace:
                    m_parameters.CullFaceEnabled = enabled;
                    break;
                case CapabilityInput::ScissorTest:
                    m_parameters.ScissorTestEnabled = enabled;
                    break;
                default: // not supported currently
                    break;
                }
            }

            Bool RenderState::IsCapabilityEnabled(CapabilityInput cap) const {
                switch (cap) {
                case CapabilityInput::Blend:
                    return m_parameters.BlendEnabled;
                case CapabilityInput::DepthTest:
                    return m_parameters.DepthTestEnabled;
                case CapabilityInput::CullFace:
                    return m_parameters.CullFaceEnabled;
                case CapabilityInput::ScissorTest:
                    return m_parameters.ScissorTestEnabled;
                default:
                    return false;
                }
            }

            // -------------------- Blending --------------------
            void RenderState::SetBlendFunc(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha,
                                           BlendFactor dstAlpha) {
                m_parameters.SrcFactorRGB = srcRGB;
                m_parameters.DstFactorRGB = dstRGB;
                m_parameters.SrcFactorAlpha = srcAlpha;
                m_parameters.DstFactorAlpha = dstAlpha;
            }

            void RenderState::GetBlendFunc(BlendFactor& srcRGB, BlendFactor& dstRGB, BlendFactor& srcAlpha,
                                           BlendFactor& dstAlpha) const {
                srcRGB = m_parameters.SrcFactorRGB;
                dstRGB = m_parameters.DstFactorRGB;
                srcAlpha = m_parameters.SrcFactorAlpha;
                dstAlpha = m_parameters.DstFactorAlpha;
            }

            // -------------------- Depth --------------------
            void RenderState::SetDepthFunc(DepthTestFunc func) {
                m_parameters.DepthFunc = func;
            }

            DepthTestFunc RenderState::GetDepthFunc() const {
                return m_parameters.DepthFunc;
            }

            void RenderState::SetDepthMask(Bool flag) {
                m_parameters.DepthMask = flag;
            }

            Bool RenderState::GetDepthMask() const {
                return m_parameters.DepthMask;
            }

            // -------------------- Color Mask --------------------
            void RenderState::SetColorMask(BoolVec4 mask) {
                m_parameters.ColorMask = mask;
            }

            const BoolVec4 RenderState::GetColorMask() const {
                return m_parameters.ColorMask;
            }

            // -------------------- Clear State --------------------
            void RenderState::SetClearColor(FloatVec4 color) {
                m_parameters.ClearColor = color;
            }

            const FloatVec4& RenderState::GetClearColor() const {
                return m_parameters.ClearColor;
            }

            void RenderState::SetClearDepth(Float depth) {
                m_parameters.ClearDepth = depth;
            }

            Float RenderState::GetClearDepth() const {
                return m_parameters.ClearDepth;
            }

            // -------------------- Pixel Store --------------------
            void RenderState::SetPixelStoreParam(PixelStoreParam param, Int value) {
                switch (param) {
                case PixelStoreParam::PackAlignment:
                    m_parameters.PackParameters.Alignment = value;
                    break;
                case PixelStoreParam::PackRowLength:
                    m_parameters.PackParameters.RowLength = value;
                    break;
                case PixelStoreParam::PackImageHeight:
                    m_parameters.PackParameters.ImageHeight = value;
                    break;
                case PixelStoreParam::PackSkipPixels:
                    m_parameters.PackParameters.SkipPixels = value;
                    break;
                case PixelStoreParam::PackSkipRows:
                    m_parameters.PackParameters.SkipRows = value;
                    break;
                case PixelStoreParam::PackSkipImages:
                    m_parameters.PackParameters.SkipImages = value;
                    break;
                case PixelStoreParam::PackSwapBytes:
                    m_parameters.PackParameters.SwapBytes = value != 0;
                    break;
                case PixelStoreParam::PackLsbFirst:
                    m_parameters.PackParameters.LSBFirst = value != 0;
                    break;
                case PixelStoreParam::UnpackAlignment:
                    m_parameters.UnpackParameters.Alignment = value;
                    break;
                case PixelStoreParam::UnpackRowLength:
                    m_parameters.UnpackParameters.RowLength = value;
                    break;
                case PixelStoreParam::UnpackImageHeight:
                    m_parameters.UnpackParameters.ImageHeight = value;
                    break;
                case PixelStoreParam::UnpackSkipPixels:
                    m_parameters.UnpackParameters.SkipPixels = value;
                    break;
                case PixelStoreParam::UnpackSkipRows:
                    m_parameters.UnpackParameters.SkipRows = value;
                    break;
                case PixelStoreParam::UnpackSkipImages:
                    m_parameters.UnpackParameters.SkipImages = value;
                    break;
                case PixelStoreParam::UnpackSwapBytes:
                    m_parameters.UnpackParameters.SwapBytes = value != 0;
                    MGLOG_D("%s: SwapBytes = %s", __func__, value ? "true" : "false");
                    break;
                case PixelStoreParam::UnpackLsbFirst:
                    m_parameters.UnpackParameters.LSBFirst = value != 0;
                    break;
                default:
                    MOBILEGL_ASSERT(false, "Invalid PixelStoreParam enum: %d", static_cast<int>(param));
                    return;
                }
            }

            Int RenderState::GetPixelStoreParam(PixelStoreParam param) const {
                switch (param) {
                case PixelStoreParam::PackAlignment:
                    return m_parameters.PackParameters.Alignment;
                case PixelStoreParam::PackRowLength:
                    return m_parameters.PackParameters.RowLength;
                case PixelStoreParam::PackImageHeight:
                    return m_parameters.PackParameters.ImageHeight;
                case PixelStoreParam::PackSkipPixels:
                    return m_parameters.PackParameters.SkipPixels;
                case PixelStoreParam::PackSkipRows:
                    return m_parameters.PackParameters.SkipRows;
                case PixelStoreParam::PackSkipImages:
                    return m_parameters.PackParameters.SkipImages;
                case PixelStoreParam::PackSwapBytes:
                    return m_parameters.PackParameters.SwapBytes ? 1 : 0;
                case PixelStoreParam::PackLsbFirst:
                    return m_parameters.PackParameters.LSBFirst ? 1 : 0;
                case PixelStoreParam::UnpackAlignment:
                    return m_parameters.UnpackParameters.Alignment;
                case PixelStoreParam::UnpackRowLength:
                    return m_parameters.UnpackParameters.RowLength;
                case PixelStoreParam::UnpackImageHeight:
                    return m_parameters.UnpackParameters.ImageHeight;
                case PixelStoreParam::UnpackSkipPixels:
                    return m_parameters.UnpackParameters.SkipPixels;
                case PixelStoreParam::UnpackSkipRows:
                    return m_parameters.UnpackParameters.SkipRows;
                case PixelStoreParam::UnpackSkipImages:
                    return m_parameters.UnpackParameters.SkipImages;
                case PixelStoreParam::UnpackSwapBytes:
                    return m_parameters.UnpackParameters.SwapBytes ? 1 : 0;
                case PixelStoreParam::UnpackLsbFirst:
                    return m_parameters.UnpackParameters.LSBFirst ? 1 : 0;
                default:
                    MOBILEGL_ASSERT(false, "Invalid PixelStoreParam enum: %d", static_cast<int>(param));
                    return 0;
                }
            }

            PixelStoreParameters RenderState::GetPixelStoreParameters(Bool isUnpack) const {
                return isUnpack ? m_parameters.UnpackParameters : m_parameters.PackParameters;
            }

            // -------------------- Cull Face --------------------
            void RenderState::SetCullFaceMode(CullFaceMode mode) {
                m_parameters.CullFaceModeSetting = mode;
            }

            CullFaceMode RenderState::GetCullFaceMode() const {
                return m_parameters.CullFaceModeSetting;
            }

            // --------------------- Scissor ---------------------
            void RenderState::SetScissorBox(IntVec4 box) {
                m_parameters.ScissorBox = box;
            }

            const IntVec4& RenderState::GetScissorBox() const {
                return m_parameters.ScissorBox;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
