#include "RenderState.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            RenderState::RenderState() {
                for (int i = 0; i < static_cast<int>(PixelStoreParam::PixelStoreParamCount); ++i) {
                    switch (static_cast<PixelStoreParam>(i)) {
                    case PixelStoreParam::PackAlignment:
                    case PixelStoreParam::UnpackAlignment:
                        m_pixelStoreParams[i] = 4;
                        break;

                    default:
                        m_pixelStoreParams[i] = 0;
                        break;
                    }
                }
            }

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
            void RenderState::SetDepthFunc(DepthFunc func) {
                m_depthFunc = func;
            }

            DepthFunc RenderState::GetDepthFunc() const {
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
                m_pixelStoreParams[static_cast<SizeT>(param)] = value;
            }

            Int RenderState::GetPixelStoreParam(PixelStoreParam param) const {
                return m_pixelStoreParams[static_cast<SizeT>(param)];
            }

            // -------------------- Cull Face --------------------
            void RenderState::SetCullFaceMode(CullFaceMode mode) {
                m_cullFaceMode = mode;
            }

            CullFaceMode RenderState::GetCullFaceMode() const {
                return m_cullFaceMode;
            }

            void RenderState::SetCullFaceEnabled(Bool enabled) {
                m_cullFaceEnabled = enabled;
            }

            Bool RenderState::IsCullFaceEnabled() const {
                return m_cullFaceEnabled;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
