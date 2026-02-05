// MobileGL - MobileGL/MG_State/GLState/RenderState/RenderState.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "RenderState.h"
#include "MG_Util/Types.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            RenderState::RenderState() {}

            Uint RenderState::GetVersion() const {
                return m_version;
            }

            const RenderStateParameters& RenderState::GetAllParameters() const {
                return m_parameters;
            }

            // -------------------- Rasterization --------------------
            void RenderState::SetViewport(IntVec4 viewport) {
                if (m_parameters.Viewport == viewport) return;

                m_parameters.Viewport = viewport;
                ++m_version;
            }

            const IntVec4& RenderState::GetViewport() const {
                return m_parameters.Viewport;
            }

            // -------------------- Capabilities --------------------
            void RenderState::SetCapability(CapabilityInput cap, Bool enabled) {
#define SET_CAPABILITY(capability, flag)                                                                               \
    case CapabilityInput::capability:                                                                                  \
        if (m_parameters.capability##Enabled == flag) break;                                                           \
        m_parameters.capability##Enabled = flag;                                                                       \
        ++m_version;                                                                                                   \
        break;

                switch (cap) {
                    SET_CAPABILITY(DepthTest, enabled);
                    SET_CAPABILITY(CullFace, enabled);
                    SET_CAPABILITY(ScissorTest, enabled);
                case CapabilityInput::Blend: {
                    Bool stateChanged = false;
                    for (auto& blendState : m_parameters.BlendStates) {
                        if (blendState.Enabled == enabled) continue;
                        blendState.Enabled = enabled;
                        stateChanged = true;
                    }
                    if (stateChanged) ++m_version;
                    break;
                }
                default: // not supported currently
                    break;
                }
#undef SET_CAPABILITY
            }

            Bool RenderState::IsCapabilityEnabled(CapabilityInput cap) const {
#define RETURN_CAPABILITY(capability)                                                                                  \
    case CapabilityInput::capability:                                                                                  \
        return m_parameters.capability##Enabled;
                switch (cap) {
                    RETURN_CAPABILITY(DepthTest);
                    RETURN_CAPABILITY(CullFace);
                    RETURN_CAPABILITY(ScissorTest);
                case CapabilityInput::Blend:
                    return m_parameters.BlendStates[0].Enabled;
                default:
                    return false;
                }
            }

            void RenderState::SetCapabilityIndexed(CapabilityInput cap, Uint index, Bool enabled) {
                // Only for BlendState currently
                if (cap != CapabilityInput::Blend) {
                    THROW_UNIMPL_EXCEPTION;
                    return;
                }
                if (index >= MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS) {
                    MOBILEGL_ASSERT(false, "Blend capability index out of range: %d", index);
                    return;
                }
                if (m_parameters.BlendStates[index].Enabled == enabled) return;

                m_parameters.BlendStates[index].Enabled = enabled;
                ++m_version;
            }

            Bool RenderState::IsCapabilityEnabledIndexed(CapabilityInput cap, Uint index) const {
                // Only for BlendState currently
                if (cap != CapabilityInput::Blend) {
                    THROW_UNIMPL_EXCEPTION;
                    return false;
                }
                if (index >= MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS) {
                    MOBILEGL_ASSERT(false, "Blend capability index out of range: %d", index);
                    return false;
                }
                return m_parameters.BlendStates[index].Enabled;
            }

            // -------------------- Blending --------------------
            void RenderState::SetBlendFunc(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha,
                                           BlendFactor dstAlpha) {
                Bool stateChanged = false;
                for (auto& blendState : m_parameters.BlendStates) {
                    if (blendState.SrcFactorRGB == srcRGB && blendState.DstFactorRGB == dstRGB &&
                        blendState.SrcFactorAlpha == srcAlpha && blendState.DstFactorAlpha == dstAlpha) {
                        continue;
                    }
                    blendState.SrcFactorRGB = srcRGB;
                    blendState.DstFactorRGB = dstRGB;
                    blendState.SrcFactorAlpha = srcAlpha;
                    blendState.DstFactorAlpha = dstAlpha;
                    stateChanged = true;
                }
                if (!stateChanged) return;
                ++m_version;
            }

            void RenderState::GetBlendFunc(BlendFactor& srcRGB, BlendFactor& dstRGB, BlendFactor& srcAlpha,
                                           BlendFactor& dstAlpha) const {
                srcRGB = m_parameters.BlendStates[0].SrcFactorRGB;
                dstRGB = m_parameters.BlendStates[0].DstFactorRGB;
                srcAlpha = m_parameters.BlendStates[0].SrcFactorAlpha;
                dstAlpha = m_parameters.BlendStates[0].DstFactorAlpha;
            }

            void RenderState::SetBlendFuncIndexed(Uint index, BlendFactor srcRGB, BlendFactor dstRGB,
                                                  BlendFactor srcAlpha, BlendFactor dstAlpha) {
                if (index >= MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS) {
                    MOBILEGL_ASSERT(false, "Blend function index out of range: %d", index);
                    return;
                }
                PerBufferBlendState& blendState = m_parameters.BlendStates[index];
                if (blendState.SrcFactorRGB == srcRGB && blendState.DstFactorRGB == dstRGB &&
                    blendState.SrcFactorAlpha == srcAlpha && blendState.DstFactorAlpha == dstAlpha) {
                    return;
                }
                blendState.SrcFactorRGB = srcRGB;
                blendState.DstFactorRGB = dstRGB;
                blendState.SrcFactorAlpha = srcAlpha;
                blendState.DstFactorAlpha = dstAlpha;
                ++m_version;
            }

            void RenderState::GetBlendFuncIndexed(Uint index, BlendFactor& srcRGB, BlendFactor& dstRGB,
                                                  BlendFactor& srcAlpha, BlendFactor& dstAlpha) const {
                if (index >= MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS) {
                    MOBILEGL_ASSERT(false, "Blend function index out of range: %d", index);
                    return;
                }
                srcRGB = m_parameters.BlendStates[index].SrcFactorRGB;
                dstRGB = m_parameters.BlendStates[index].DstFactorRGB;
                srcAlpha = m_parameters.BlendStates[index].SrcFactorAlpha;
                dstAlpha = m_parameters.BlendStates[index].DstFactorAlpha;
            }

            // -------------------- Depth --------------------
            void RenderState::SetDepthFunc(DepthTestFunc func) {
                if (m_parameters.DepthFunc == func) return;

                m_parameters.DepthFunc = func;
                ++m_version;
            }

            DepthTestFunc RenderState::GetDepthFunc() const {
                return m_parameters.DepthFunc;
            }

            void RenderState::SetDepthMask(Bool flag) {
                if (m_parameters.DepthMask == flag) return;

                m_parameters.DepthMask = flag;
                ++m_version;
            }

            Bool RenderState::GetDepthMask() const {
                return m_parameters.DepthMask;
            }

            // -------------------- Color Mask --------------------
            void RenderState::SetColorMask(BoolVec4 mask) {
                if (m_parameters.ColorMask == mask) return;

                m_parameters.ColorMask = mask;
                ++m_version;
            }

            const BoolVec4 RenderState::GetColorMask() const {
                return m_parameters.ColorMask;
            }

            // -------------------- Clear State --------------------
            void RenderState::SetClearColor(FloatVec4 color) {
                if (m_parameters.ClearColor == color) return;

                m_parameters.ClearColor = color;
                ++m_version;
            }

            const FloatVec4& RenderState::GetClearColor() const {
                return m_parameters.ClearColor;
            }

            void RenderState::SetClearDepth(Float depth) {
                if (m_parameters.ClearDepth == depth) return;

                m_parameters.ClearDepth = depth;
                ++m_version;
            }

            Float RenderState::GetClearDepth() const {
                return m_parameters.ClearDepth;
            }

            // -------------------- Pixel Store --------------------
            void RenderState::SetPixelStoreParam(PixelStoreParam param, Int value) {
#define SET_PIXEL_STORE_PARAM(paramNameHead, paramNameTail, val)                                                       \
    case PixelStoreParam::paramNameHead##paramNameTail:                                                                \
        if (m_pixelStore##paramNameHead##Parameters.paramNameTail == val) break;                                       \
        m_pixelStore##paramNameHead##Parameters.paramNameTail = val;                                                   \
        break;

                switch (param) {
                    SET_PIXEL_STORE_PARAM(Pack, Alignment, value);
                    SET_PIXEL_STORE_PARAM(Pack, RowLength, value);
                    SET_PIXEL_STORE_PARAM(Pack, ImageHeight, value);
                    SET_PIXEL_STORE_PARAM(Pack, SkipPixels, value);
                    SET_PIXEL_STORE_PARAM(Pack, SkipRows, value);
                    SET_PIXEL_STORE_PARAM(Pack, SkipImages, value);
                    SET_PIXEL_STORE_PARAM(Pack, SwapBytes, value != 0);
                    SET_PIXEL_STORE_PARAM(Pack, LSBFirst, value != 0);
                    SET_PIXEL_STORE_PARAM(Unpack, Alignment, value);
                    SET_PIXEL_STORE_PARAM(Unpack, RowLength, value);
                    SET_PIXEL_STORE_PARAM(Unpack, ImageHeight, value);
                    SET_PIXEL_STORE_PARAM(Unpack, SkipPixels, value);
                    SET_PIXEL_STORE_PARAM(Unpack, SkipRows, value);
                    SET_PIXEL_STORE_PARAM(Unpack, SkipImages, value);
                    SET_PIXEL_STORE_PARAM(Unpack, SwapBytes, value != 0);
                    SET_PIXEL_STORE_PARAM(Unpack, LSBFirst, value != 0);
                default:
                    MOBILEGL_ASSERT(false, "Invalid PixelStoreParam enum: %d", static_cast<int>(param));
                    return;
                }
            }

            Int RenderState::GetPixelStoreParam(PixelStoreParam param) const {
#define RETURN_PIXEL_STORE_PARAM(paramNameHead, paramNameTail)                                                         \
    case PixelStoreParam::paramNameHead##paramNameTail:                                                                \
        return m_pixelStore##paramNameHead##Parameters.paramNameTail;
                switch (param) {
                    RETURN_PIXEL_STORE_PARAM(Pack, Alignment);
                    RETURN_PIXEL_STORE_PARAM(Pack, RowLength);
                    RETURN_PIXEL_STORE_PARAM(Pack, ImageHeight);
                    RETURN_PIXEL_STORE_PARAM(Pack, SkipPixels);
                    RETURN_PIXEL_STORE_PARAM(Pack, SkipRows);
                    RETURN_PIXEL_STORE_PARAM(Pack, SkipImages);
                    RETURN_PIXEL_STORE_PARAM(Pack, SwapBytes);
                    RETURN_PIXEL_STORE_PARAM(Pack, LSBFirst);
                    RETURN_PIXEL_STORE_PARAM(Unpack, Alignment);
                    RETURN_PIXEL_STORE_PARAM(Unpack, RowLength);
                    RETURN_PIXEL_STORE_PARAM(Unpack, ImageHeight);
                    RETURN_PIXEL_STORE_PARAM(Unpack, SkipPixels);
                    RETURN_PIXEL_STORE_PARAM(Unpack, SkipRows);
                    RETURN_PIXEL_STORE_PARAM(Unpack, SkipImages);
                    RETURN_PIXEL_STORE_PARAM(Unpack, SwapBytes);
                    RETURN_PIXEL_STORE_PARAM(Unpack, LSBFirst);
                default:
                    MOBILEGL_ASSERT(false, "Invalid PixelStoreParam enum: %d", static_cast<int>(param));
                    return 0;
                }
            }

            PixelStoreParameters RenderState::GetPixelStoreParameters(Bool isUnpack) const {
                return isUnpack ? m_pixelStoreUnpackParameters : m_pixelStorePackParameters;
            }

            // -------------------- Cull Face --------------------
            void RenderState::SetCullFaceMode(CullFaceMode mode) {
                if (m_parameters.CullFaceModeSetting == mode) return;

                m_parameters.CullFaceModeSetting = mode;
                ++m_version;
            }

            CullFaceMode RenderState::GetCullFaceMode() const {
                return m_parameters.CullFaceModeSetting;
            }

            // --------------------- Scissor ---------------------
            void RenderState::SetScissorBox(IntVec4 box) {
                if (m_parameters.ScissorBox == box) return;

                m_parameters.ScissorBox = box;
                ++m_version;
            }

            const IntVec4& RenderState::GetScissorBox() const {
                return m_parameters.ScissorBox;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
