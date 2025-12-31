// MobileGL - MobileGL/MG_Util/Converters/MGToGL/RenderStateEnumConverter.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "RenderStateEnumConverter.h"

namespace MobileGL {
    namespace MG_Util {

        GLenum ConvertBlendFactorToGLEnum(BlendFactor v) {
            switch (v) {
            case BlendFactor::Zero:
                return GL_ZERO;
            case BlendFactor::One:
                return GL_ONE;
            case BlendFactor::SrcColor:
                return GL_SRC_COLOR;
            case BlendFactor::OneMinusSrcColor:
                return GL_ONE_MINUS_SRC_COLOR;
            case BlendFactor::DstColor:
                return GL_DST_COLOR;
            case BlendFactor::OneMinusDstColor:
                return GL_ONE_MINUS_DST_COLOR;
            case BlendFactor::SrcAlpha:
                return GL_SRC_ALPHA;
            case BlendFactor::OneMinusSrcAlpha:
                return GL_ONE_MINUS_SRC_ALPHA;
            case BlendFactor::DstAlpha:
                return GL_DST_ALPHA;
            case BlendFactor::OneMinusDstAlpha:
                return GL_ONE_MINUS_DST_ALPHA;
            case BlendFactor::ConstantColor:
                return GL_CONSTANT_COLOR;
            case BlendFactor::OneMinusConstantColor:
                return GL_ONE_MINUS_CONSTANT_COLOR;
            case BlendFactor::ConstantAlpha:
                return GL_CONSTANT_ALPHA;
            case BlendFactor::OneMinusConstantAlpha:
                return GL_ONE_MINUS_CONSTANT_ALPHA;
            default:
                return GL_UNKNOWN_MGL;
            }
        }

        GLenum ConvertDepthTestFuncToGLEnum(DepthTestFunc v) {
            switch (v) {
            case DepthTestFunc::Never:
                return GL_NEVER;
            case DepthTestFunc::Less:
                return GL_LESS;
            case DepthTestFunc::Equal:
                return GL_EQUAL;
            case DepthTestFunc::LessEqual:
                return GL_LEQUAL;
            case DepthTestFunc::Greater:
                return GL_GREATER;
            case DepthTestFunc::NotEqual:
                return GL_NOTEQUAL;
            case DepthTestFunc::GreaterEqual:
                return GL_GEQUAL;
            case DepthTestFunc::Always:
                return GL_ALWAYS;
            default:
                return GL_UNKNOWN_MGL;
            }
        }

        GLenum ConvertPixelStoreParamToGLEnum(PixelStoreParam v) {
            switch (v) {
            case PixelStoreParam::PackAlignment:
                return GL_PACK_ALIGNMENT;
            case PixelStoreParam::PackRowLength:
                return GL_PACK_ROW_LENGTH;
            case PixelStoreParam::PackImageHeight:
                return GL_PACK_IMAGE_HEIGHT;
            case PixelStoreParam::PackSkipRows:
                return GL_PACK_SKIP_ROWS;
            case PixelStoreParam::PackSkipPixels:
                return GL_PACK_SKIP_PIXELS;
            case PixelStoreParam::PackSkipImages:
                return GL_PACK_SKIP_IMAGES;
            case PixelStoreParam::PackSwapBytes:
                return GL_PACK_SWAP_BYTES;
            case PixelStoreParam::PackLsbFirst:
                return GL_PACK_LSB_FIRST;
            case PixelStoreParam::UnpackAlignment:
                return GL_UNPACK_ALIGNMENT;
            case PixelStoreParam::UnpackRowLength:
                return GL_UNPACK_ROW_LENGTH;
            case PixelStoreParam::UnpackImageHeight:
                return GL_UNPACK_IMAGE_HEIGHT;
            case PixelStoreParam::UnpackSkipRows:
                return GL_UNPACK_SKIP_ROWS;
            case PixelStoreParam::UnpackSkipPixels:
                return GL_UNPACK_SKIP_PIXELS;
            case PixelStoreParam::UnpackSkipImages:
                return GL_UNPACK_SKIP_IMAGES;
            case PixelStoreParam::UnpackSwapBytes:
                return GL_UNPACK_SWAP_BYTES;
            case PixelStoreParam::UnpackLsbFirst:
                return GL_UNPACK_LSB_FIRST;
            default:
                return GL_UNKNOWN_MGL;
            }
        }

        GLenum ConvertCullFaceModeToGLEnum(CullFaceMode v) {
            switch (v) {
            case CullFaceMode::Front:
                return GL_FRONT;
            case CullFaceMode::Back:
                return GL_BACK;
            case CullFaceMode::FrontAndBack:
                return GL_FRONT_AND_BACK;
            default:
                return GL_UNKNOWN_MGL;
            }
        }

        GLenum ConvertCapabilityInputToGLEnum(CapabilityInput v) {
            switch (v) {
            case CapabilityInput::Blend:
                return GL_BLEND;
            case CapabilityInput::ClipDistance0:
                return GL_CLIP_DISTANCE0;
            case CapabilityInput::ClipDistance1:
                return GL_CLIP_DISTANCE1;
            case CapabilityInput::ClipDistance2:
                return GL_CLIP_DISTANCE2;
            case CapabilityInput::ClipDistance3:
                return GL_CLIP_DISTANCE3;
            case CapabilityInput::ClipDistance4:
                return GL_CLIP_DISTANCE4;
            case CapabilityInput::ClipDistance5:
                return GL_CLIP_DISTANCE5;
            case CapabilityInput::ClipDistance6:
                return GL_CLIP_DISTANCE6;
            case CapabilityInput::ClipDistance7:
                return GL_CLIP_DISTANCE7;
            case CapabilityInput::ColorLogicOp:
                return GL_COLOR_LOGIC_OP;
            case CapabilityInput::CullFace:
                return GL_CULL_FACE;
            case CapabilityInput::DebugOutput:
                return GL_DEBUG_OUTPUT;
            case CapabilityInput::DebugOutputSynchronous:
                return GL_DEBUG_OUTPUT_SYNCHRONOUS;
            case CapabilityInput::DepthClamp:
                return GL_DEPTH_CLAMP;
            case CapabilityInput::DepthTest:
                return GL_DEPTH_TEST;
            case CapabilityInput::Dither:
                return GL_DITHER;
            case CapabilityInput::FramebufferSrgb:
                return GL_FRAMEBUFFER_SRGB;
            case CapabilityInput::LineSmooth:
                return GL_LINE_SMOOTH;
            case CapabilityInput::Multisample:
                return GL_MULTISAMPLE;
            case CapabilityInput::PolygonOffsetFill:
                return GL_POLYGON_OFFSET_FILL;
            case CapabilityInput::PolygonOffsetLine:
                return GL_POLYGON_OFFSET_LINE;
            case CapabilityInput::PolygonOffsetPoint:
                return GL_POLYGON_OFFSET_POINT;
            case CapabilityInput::PolygonSmooth:
                return GL_POLYGON_SMOOTH;
            case CapabilityInput::PrimitiveRestart:
                return GL_PRIMITIVE_RESTART;
            case CapabilityInput::PrimitiveRestartFixedIndex:
                return GL_PRIMITIVE_RESTART_FIXED_INDEX;
            case CapabilityInput::RasterizerDiscard:
                return GL_RASTERIZER_DISCARD;
            case CapabilityInput::SampleAlphaToCoverage:
                return GL_SAMPLE_ALPHA_TO_COVERAGE;
            case CapabilityInput::SampleAlphaToOne:
                return GL_SAMPLE_ALPHA_TO_ONE;
            case CapabilityInput::SampleCoverage:
                return GL_SAMPLE_COVERAGE;
            case CapabilityInput::SampleShading:
                return GL_SAMPLE_SHADING;
            case CapabilityInput::SampleMask:
                return GL_SAMPLE_MASK;
            case CapabilityInput::ScissorTest:
                return GL_SCISSOR_TEST;
            case CapabilityInput::StencilTest:
                return GL_STENCIL_TEST;
            case CapabilityInput::TextureCubeMapSeamless:
                return GL_TEXTURE_CUBE_MAP_SEAMLESS;
            case CapabilityInput::ProgramPointSize:
                return GL_PROGRAM_POINT_SIZE;
            default:
                return GL_UNKNOWN_MGL;
            }
        }
    } // namespace MG_Util
} // namespace MobileGL