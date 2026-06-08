// MobileGL - MobileGL/MG_State/GLState/RenderState/RenderState.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_Util/Math/VectorTypes.h>
#include <MG_State/GLState/FramebufferState/FramebufferObject.h>

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

    enum class BlendEquation {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max,
        BlendEquationCount,
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
        PackLSBFirst,

        // Unpack Parameters
        UnpackAlignment,
        UnpackRowLength,
        UnpackImageHeight,
        UnpackSkipRows,
        UnpackSkipPixels,
        UnpackSkipImages,
        UnpackSwapBytes,
        UnpackLSBFirst,

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

    enum class FrontFaceMode {
        CounterClockwise,
        Clockwise,
        FrontFaceModeCount,
        Unknown = -1
    };

    enum class ProvokingVertexMode {
        FirstVertex,
        LastVertex,
        ProvokingVertexModeCount,
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

    struct PixelStoreParameters {
        Bool SwapBytes = false;
        Bool LSBFirst = false;
        Int RowLength = 0;
        Int ImageHeight = 0;
        Int SkipPixels = 0;
        Int SkipRows = 0;
        Int SkipImages = 0;
        Int Alignment = 4;
    };

    struct PerBufferBlendState {
        Bool Enabled = false;
        BlendFactor SrcFactorRGB = BlendFactor::One;
        BlendFactor DstFactorRGB = BlendFactor::Zero;
        BlendFactor SrcFactorAlpha = BlendFactor::One;
        BlendFactor DstFactorAlpha = BlendFactor::Zero;
        BlendEquation ColorEquation = BlendEquation::Add;
        BlendEquation AlphaEquation = BlendEquation::Add;
    };

    struct RenderStateParameters {
        // Rasterization
        IntVec4 Viewport = IntVec4(0, 0, 0, 0); // x, y, width, height

        // Blending
        Array<PerBufferBlendState, MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS> BlendStates;

        // Depth
        Bool DepthTestEnabled = false;
        DepthTestFunc DepthFunc = DepthTestFunc::Less;
        Bool DepthMask = true;

        // Color Mask
        BoolVec4 ColorMask = BoolVec4(true, true, true, true);

        // Clear State
        FloatVec4 ClearColor = FloatVec4(0.0f, 0.0f, 0.0f, 1.0f);
        Float ClearDepth = 1.0f;
        Uint32 ClearStencil = 0;

        // Cull Face
        Bool CullFaceEnabled = false;
        CullFaceMode CullFaceModeSetting = CullFaceMode::Back;
        FrontFaceMode FrontFaceModeSetting = FrontFaceMode::CounterClockwise;
        ProvokingVertexMode ProvokingVertexModeSetting = ProvokingVertexMode::LastVertex;

        // Scissor
        Bool ScissorTestEnabled = false;
        IntVec4 ScissorBox = IntVec4(0, 0, 0, 0); // x, y, width, height
    };

    namespace MG_State {
        namespace GLState {
            class RenderState {
            public:
                RenderState();

                Uint GetVersion() const;
                const RenderStateParameters& GetAllParameters() const;

                // Rasterization
                void SetViewport(IntVec4 viewport); // x, y, width, height
                const IntVec4& GetViewport() const; // x, y, width, height

                // Capabilities
                void SetCapability(CapabilityInput cap, Bool enabled);
                Bool IsCapabilityEnabled(CapabilityInput cap) const;
                void SetCapabilityIndexed(CapabilityInput cap, Uint index, Bool enabled);
                Bool IsCapabilityEnabledIndexed(CapabilityInput cap, Uint index) const;

                // Blending
                void SetBlendFunc(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha, BlendFactor dstAlpha);
                void GetBlendFunc(BlendFactor& srcRGB, BlendFactor& dstRGB, BlendFactor& srcAlpha,
                                  BlendFactor& dstAlpha) const;
                void SetBlendFuncIndexed(Uint index, BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha,
                                         BlendFactor dstAlpha);
                void GetBlendFuncIndexed(Uint index, BlendFactor& srcRGB, BlendFactor& dstRGB, BlendFactor& srcAlpha,
                                         BlendFactor& dstAlpha) const;
                void SetBlendEquation(BlendEquation color, BlendEquation alpha);
                void GetBlendEquation(BlendEquation& color, BlendEquation& alpha) const;
                void SetBlendEquationIndexed(Uint index, BlendEquation color, BlendEquation alpha);
                void GetBlendEquationIndexed(Uint index, BlendEquation& color, BlendEquation& alpha) const;

                // Depth
                void SetDepthFunc(DepthTestFunc func);
                DepthTestFunc GetDepthFunc() const;
                void SetDepthMask(Bool flag);
                Bool GetDepthMask() const;

                // Color Mask
                void SetColorMask(BoolVec4 mask);
                BoolVec4 GetColorMask() const;

                // Clear State
                void SetClearColor(FloatVec4 color);
                const FloatVec4& GetClearColor() const;
                void SetClearDepth(Float depth);
                Float GetClearDepth() const;
                void SetClearStencil(Int stencil);
                Uint32 GetClearStencil() const;

                // Pixel Store
                void SetPixelStoreParam(PixelStoreParam param, Int value);
                Int GetPixelStoreParam(PixelStoreParam param) const;
                PixelStoreParameters GetPixelStoreParameters(Bool isUnpack) const;

                // Cull Face
                void SetCullFaceMode(CullFaceMode mode);
                CullFaceMode GetCullFaceMode() const;
                void SetFrontFaceMode(FrontFaceMode mode);
                FrontFaceMode GetFrontFaceMode() const;
                void SetProvokingVertexMode(ProvokingVertexMode mode);
                ProvokingVertexMode GetProvokingVertexMode() const;

                // Scissor
                void SetScissorBox(IntVec4 box);      // x, y, width, height
                const IntVec4& GetScissorBox() const; // x, y, width, height

            private:
                Uint16 m_version = 0;
                RenderStateParameters m_parameters;

                // Pixel Store
                PixelStoreParameters m_pixelStorePackParameters;
                PixelStoreParameters m_pixelStoreUnpackParameters;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
