#include "RenderState.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            RenderState::RenderState() {}

            // -------------------- Rasterization --------------------
            void RenderState::SetViewport(IntVec4 viewport) {
                m_viewport = viewport;
            }

            const IntVec4& RenderState::GetViewport() const {
                return m_viewport;
            }

            // -------------------- Capabilities --------------------
            void RenderState::SetCapability(CapabilityInput cap, Bool enabled) {
                switch (cap) {
                case CapabilityInput::Blend:
                    m_blendEnabled = enabled;
                    break;
                case CapabilityInput::DepthTest:
                    m_depthTestEnabled = enabled;
                    break;
                case CapabilityInput::CullFace:
                    m_cullFaceEnabled = enabled;
                    break;
                case CapabilityInput::ScissorTest:
                    m_scissorTestEnabled = enabled;
                    break;
                default: // not supported currently
                    break;
                }
            }

            Bool RenderState::IsCapabilityEnabled(CapabilityInput cap) const {
                switch (cap) {
                case CapabilityInput::Blend:
                    return m_blendEnabled;
                case CapabilityInput::DepthTest:
                    return m_depthTestEnabled;
                case CapabilityInput::CullFace:
                    return m_cullFaceEnabled;
                case CapabilityInput::ScissorTest:
                    return m_scissorTestEnabled;
                default:
                    return false;
                }
            }

            // -------------------- Blending --------------------
            void RenderState::SetBlendFunc(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha,
                                           BlendFactor dstAlpha) {
                m_srcFactorRGB = srcRGB;
                m_dstFactorRGB = dstRGB;
                m_srcFactorAlpha = srcAlpha;
                m_dstFactorAlpha = dstAlpha;
            }

            void RenderState::GetBlendFunc(BlendFactor& srcRGB, BlendFactor& dstRGB, BlendFactor& srcAlpha,
                                           BlendFactor& dstAlpha) const {
                srcRGB = m_srcFactorRGB;
                dstRGB = m_dstFactorRGB;
                srcAlpha = m_srcFactorAlpha;
                dstAlpha = m_dstFactorAlpha;
            }

            // -------------------- Depth --------------------
            void RenderState::SetDepthFunc(DepthTestFunc func) {
                m_depthFunc = func;
            }

            DepthTestFunc RenderState::GetDepthFunc() const {
                return m_depthFunc;
            }

            void RenderState::SetDepthMask(Bool flag) {
                m_depthMask = flag;
            }

            Bool RenderState::GetDepthMask() const {
                return m_depthMask;
            }

            // -------------------- Color Mask --------------------
            void RenderState::SetColorMask(BoolVec4 mask) {
                m_colorMask = mask;
            }

            const BoolVec4 RenderState::GetColorMask() const {
                return m_colorMask;
            }

            // -------------------- Clear State --------------------
            void RenderState::SetClearColor(FloatVec4 color) {
                m_clearColor = color;
            }

            const FloatVec4& RenderState::GetClearColor() const {
                return m_clearColor;
            }

            void RenderState::SetClearDepth(Float depth) {
                m_clearDepth = depth;
            }

            Float RenderState::GetClearDepth() const {
                return m_clearDepth;
            }

            // -------------------- Pixel Store --------------------
            void RenderState::SetPixelStoreParam(PixelStoreParam param, Int value) {
                switch (param) {
                case PixelStoreParam::PackAlignment:
                    m_packParameters.Alignment = value;
                    break;
                case PixelStoreParam::PackRowLength:
                    m_packParameters.RowLength = value;
                    break;
                case PixelStoreParam::PackImageHeight:
                    m_packParameters.ImageHeight = value;
                    break;
                case PixelStoreParam::PackSkipPixels:
                    m_packParameters.SkipPixels = value;
                    break;
                case PixelStoreParam::PackSkipRows:
                    m_packParameters.SkipRows = value;
                    break;
                case PixelStoreParam::PackSkipImages:
                    m_packParameters.SkipImages = value;
                    break;
                case PixelStoreParam::PackSwapBytes:
                    m_packParameters.SwapBytes = value != 0;
                    break;
                case PixelStoreParam::PackLsbFirst:
                    m_packParameters.LSBFirst = value != 0;
                    break;
                case PixelStoreParam::UnpackAlignment:
                    m_unpackParameters.Alignment = value;
                    break;
                case PixelStoreParam::UnpackRowLength:
                    m_unpackParameters.RowLength = value;
                    break;
                case PixelStoreParam::UnpackImageHeight:
                    m_unpackParameters.ImageHeight = value;
                    break;
                case PixelStoreParam::UnpackSkipPixels:
                    m_unpackParameters.SkipPixels = value;
                    break;
                case PixelStoreParam::UnpackSkipRows:
                    m_unpackParameters.SkipRows = value;
                    break;
                case PixelStoreParam::UnpackSkipImages:
                    m_unpackParameters.SkipImages = value;
                    break;
                case PixelStoreParam::UnpackSwapBytes:
                    m_unpackParameters.SwapBytes = value != 0;
                    break;
                case PixelStoreParam::UnpackLsbFirst:
                    m_unpackParameters.LSBFirst = value != 0;
                    break;
                default:
                    MGLOG_F("RenderState::SetPixelStoreParam: Invalid PixelStoreParam enum: %d",
                            static_cast<int>(param));
                    assert(false && "Invalid PixelStoreParam enum");
                    return;
                }
            }

            Int RenderState::GetPixelStoreParam(PixelStoreParam param) const {
                switch (param) {
                case PixelStoreParam::PackAlignment:
                    return m_packParameters.Alignment;
                case PixelStoreParam::PackRowLength:
                    return m_packParameters.RowLength;
                case PixelStoreParam::PackImageHeight:
                    return m_packParameters.ImageHeight;
                case PixelStoreParam::PackSkipPixels:
                    return m_packParameters.SkipPixels;
                case PixelStoreParam::PackSkipRows:
                    return m_packParameters.SkipRows;
                case PixelStoreParam::PackSkipImages:
                    return m_packParameters.SkipImages;
                case PixelStoreParam::PackSwapBytes:
                    return m_packParameters.SwapBytes ? 1 : 0;
                case PixelStoreParam::PackLsbFirst:
                    return m_packParameters.LSBFirst ? 1 : 0;
                case PixelStoreParam::UnpackAlignment:
                    return m_unpackParameters.Alignment;
                case PixelStoreParam::UnpackRowLength:
                    return m_unpackParameters.RowLength;
                case PixelStoreParam::UnpackImageHeight:
                    return m_unpackParameters.ImageHeight;
                case PixelStoreParam::UnpackSkipPixels:
                    return m_unpackParameters.SkipPixels;
                case PixelStoreParam::UnpackSkipRows:
                    return m_unpackParameters.SkipRows;
                case PixelStoreParam::UnpackSkipImages:
                    return m_unpackParameters.SkipImages;
                case PixelStoreParam::UnpackSwapBytes:
                    return m_unpackParameters.SwapBytes ? 1 : 0;
                case PixelStoreParam::UnpackLsbFirst:
                    return m_unpackParameters.LSBFirst ? 1 : 0;
                default:
                    MGLOG_F("RenderState::GetPixelStoreParam: Invalid PixelStoreParam enum: %d",
                            static_cast<int>(param));
                    assert(false && "Invalid PixelStoreParam enum");
                    return 0;
                }
            }

            PixelStoreParameters RenderState::GetPixelStoreParameters(Bool isUnpack) const {
                return isUnpack ? m_unpackParameters : m_packParameters;
            }

            // -------------------- Cull Face --------------------
            void RenderState::SetCullFaceMode(CullFaceMode mode) {
                m_cullFaceMode = mode;
            }

            CullFaceMode RenderState::GetCullFaceMode() const {
                return m_cullFaceMode;
            }

            // --------------------- Scissor ---------------------
            void RenderState::SetScissorBox(IntVec4 box) {
                m_scissorBox = box;
            }

            const IntVec4& RenderState::GetScissorBox() const {
                return m_scissorBox;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
