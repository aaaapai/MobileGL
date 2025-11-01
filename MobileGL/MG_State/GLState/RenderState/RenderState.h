#pragma once
#include "MG_Util/Math/VectorTypes.h"
#include "MG_Util/Types.h"
#include <Includes.h>

namespace MobileGL {
    enum class BlendFactor {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        BlendFactorCount,
        Unknown = -1
    };

    enum class DepthTestFunc {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always,
        DepthTestFuncCount,
        Unknown = -1
    };

    enum class PixelStoreParam {
        // Pack Parameters
        PackAlignment,
        PackRowLength,
        PackImageHeight,
        PackSkipRows,
        PackSkipPixels,
        PackSkipImages,
        PackSwapBytes,
        PackLsbFirst,

        // Unpack Parameters
        UnpackAlignment,
        UnpackRowLength,
        UnpackImageHeight,
        UnpackSkipRows,
        UnpackSkipPixels,
        UnpackSkipImages,
        UnpackSwapBytes,
        UnpackLsbFirst,

        PixelStoreParamCount,
        Unknown = -1
    };

    enum class CullFaceMode {
        Front,
        Back,
        FrontAndBack,
        CullFaceModeCount,
        Unknown = -1
    };

    enum class CapabilityInput {
        Blend,
        ClipDistance0,
        ClipDistance1,
        ClipDistance2,
        ClipDistance3,
        ClipDistance4,
        ClipDistance5,
        ClipDistance6,
        ClipDistance7,
        ColorLogicOp,
        CullFace,
        DebugOutput,
        DebugOutputSynchronous,
        DepthClamp,
        DepthTest,
        Dither,
        FramebufferSrgb,
        LineSmooth,
        Multisample,
        PolygonOffsetFill,
        PolygonOffsetLine,
        PolygonOffsetPoint,
        PolygonSmooth,
        PrimitiveRestart,
        PrimitiveRestartFixedIndex,
        RasterizerDiscard,
        SampleAlphaToCoverage,
        SampleAlphaToOne,
        SampleCoverage,
        SampleShading,
        SampleMask,
        ScissorTest,
        StencilTest,
        TextureCubeMapSeamless,
        ProgramPointSize,
        CapabilityInputCount,
        Unknown = -1
    };

    namespace MG_State {
        namespace GLState {
            class RenderState {
            public:
                RenderState();

                // Rasterization
                void SetViewport(IntVec4 viewport); // x, y, width, height
                const IntVec4& GetViewport() const; // x, y, width, height

                // Capabilities
                void SetCapability(CapabilityInput cap, Bool enabled);
                Bool IsCapabilityEnabled(CapabilityInput cap) const;

                // Blending
                void SetBlendFunc(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha, BlendFactor dstAlpha);
                void GetBlendFunc(BlendFactor& srcRGB, BlendFactor& dstRGB, BlendFactor& srcAlpha,
                                  BlendFactor& dstAlpha) const;

                // Depth
                void SetDepthFunc(DepthTestFunc func);
                DepthTestFunc GetDepthFunc() const;
                void SetDepthMask(Bool flag);
                Bool GetDepthMask() const;

                // Color Mask
                void SetColorMask(BoolVec4 mask);
                const BoolVec4 GetColorMask() const;

                // Clear State
                void SetClearColor(FloatVec4 color);
                const FloatVec4& GetClearColor() const;
                void SetClearDepth(Float depth);
                Float GetClearDepth() const;

                // Pixel Store
                void SetPixelStoreParam(PixelStoreParam param, Int value);
                Int GetPixelStoreParam(PixelStoreParam param) const;

                // Cull Face
                void SetCullFaceMode(CullFaceMode mode);
                CullFaceMode GetCullFaceMode() const;

                // Scissor
                void SetScissorBox(IntVec4 box);      // x, y, width, height
                const IntVec4& GetScissorBox() const; // x, y, width, height

            private:
                // Rasterization
                IntVec4 m_viewport = IntVec4(0, 0, 0, 0); // x, y, width, height

                // Blending
                Bool m_blendEnabled = false;
                BlendFactor m_srcFactorRGB = BlendFactor::One;
                BlendFactor m_dstFactorRGB = BlendFactor::Zero;
                BlendFactor m_srcFactorAlpha = BlendFactor::One;
                BlendFactor m_dstFactorAlpha = BlendFactor::Zero;

                // Depth
                Bool m_depthTestEnabled = false;
                DepthTestFunc m_depthFunc = DepthTestFunc::Less;
                Bool m_depthMask = true;

                // Color Mask
                BoolVec4 m_colorMask = BoolVec4(true, true, true, true);

                // Clear State
                FloatVec4 m_clearColor = FloatVec4(0.0f, 0.0f, 0.0f, 1.0f);
                Float m_clearDepth = 1.0f;

                // Pixel Store
                Array<Int, static_cast<SizeT>(PixelStoreParam::PixelStoreParamCount)> m_pixelStoreParams;

                // Cull Face
                Bool m_cullFaceEnabled = false;
                CullFaceMode m_cullFaceMode = CullFaceMode::Back;

                // Scissor
                Bool m_scissorTestEnabled = false;
                IntVec4 m_scissorBox = IntVec4(0, 0, 0, 0); // x, y, width, height
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL