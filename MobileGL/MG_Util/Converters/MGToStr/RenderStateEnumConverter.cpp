#include "RenderStateEnumConverter.h"

namespace MobileGL {
    namespace MG_Util {
        String ConvertBlendFactorToString(BlendFactor v) {
            switch (v) {
            case BlendFactor::Zero:
                return "Zero";
            case BlendFactor::One:
                return "One";
            case BlendFactor::SrcColor:
                return "SrcColor";
            case BlendFactor::OneMinusSrcColor:
                return "OneMinusSrcColor";
            case BlendFactor::DstColor:
                return "DstColor";
            case BlendFactor::OneMinusDstColor:
                return "OneMinusDstColor";
            case BlendFactor::SrcAlpha:
                return "SrcAlpha";
            case BlendFactor::OneMinusSrcAlpha:
                return "OneMinusSrcAlpha";
            case BlendFactor::DstAlpha:
                return "DstAlpha";
            case BlendFactor::OneMinusDstAlpha:
                return "OneMinusDstAlpha";
            case BlendFactor::ConstantColor:
                return "ConstantColor";
            case BlendFactor::OneMinusConstantColor:
                return "OneMinusConstantColor";
            case BlendFactor::ConstantAlpha:
                return "ConstantAlpha";
            case BlendFactor::OneMinusConstantAlpha:
                return "OneMinusConstantAlpha";
            default:
                return "Unknown";
            }
        }

        String ConvertDepthFuncToString(DepthFunc v) {
            switch (v) {
            case DepthFunc::Never:
                return "Never";
            case DepthFunc::Less:
                return "Less";
            case DepthFunc::Equal:
                return "Equal";
            case DepthFunc::LessEqual:
                return "LessEqual";
            case DepthFunc::Greater:
                return "Greater";
            case DepthFunc::NotEqual:
                return "NotEqual";
            case DepthFunc::GreaterEqual:
                return "GreaterEqual";
            case DepthFunc::Always:
                return "Always";
            default:
                return "Unknown";
            }
        }

        String ConvertPixelStoreParamToString(PixelStoreParam v) {
            switch (v) {
            case PixelStoreParam::PackAlignment:
                return "PackAlignment";
            case PixelStoreParam::PackRowLength:
                return "PackRowLength";
            case PixelStoreParam::PackImageHeight:
                return "PackImageHeight";
            case PixelStoreParam::PackSkipRows:
                return "PackSkipRows";
            case PixelStoreParam::PackSkipPixels:
                return "PackSkipPixels";
            case PixelStoreParam::PackSkipImages:
                return "PackSkipImages";
            case PixelStoreParam::PackSwapBytes:
                return "PackSwapBytes";
            case PixelStoreParam::PackLsbFirst:
                return "PackLsbFirst";
            case PixelStoreParam::UnpackAlignment:
                return "UnpackAlignment";
            case PixelStoreParam::UnpackRowLength:
                return "UnpackRowLength";
            case PixelStoreParam::UnpackImageHeight:
                return "UnpackImageHeight";
            case PixelStoreParam::UnpackSkipRows:
                return "UnpackSkipRows";
            case PixelStoreParam::UnpackSkipPixels:
                return "UnpackSkipPixels";
            case PixelStoreParam::UnpackSkipImages:
                return "UnpackSkipImages";
            case PixelStoreParam::UnpackSwapBytes:
                return "UnpackSwapBytes";
            case PixelStoreParam::UnpackLsbFirst:
                return "UnpackLsbFirst";
            default:
                return "Unknown";
            }
        }

        String ConvertCullFaceModeToString(CullFaceMode v) {
            switch (v) {
            case CullFaceMode::Front:
                return "Front";
            case CullFaceMode::Back:
                return "Back";
            case CullFaceMode::FrontAndBack:
                return "FrontAndBack";
            default:
                return "Unknown";
            }
        }

        String ConvertCapabilityInputToString(CapabilityInput v) {
            switch (v) {
            case CapabilityInput::Blend:
                return "Blend";
            case CapabilityInput::ClipDistance0:
                return "ClipDistance0";
            case CapabilityInput::ClipDistance1:
                return "ClipDistance1";
            case CapabilityInput::ClipDistance2:
                return "ClipDistance2";
            case CapabilityInput::ClipDistance3:
                return "ClipDistance3";
            case CapabilityInput::ClipDistance4:
                return "ClipDistance4";
            case CapabilityInput::ClipDistance5:
                return "ClipDistance5";
            case CapabilityInput::ClipDistance6:
                return "ClipDistance6";
            case CapabilityInput::ClipDistance7:
                return "ClipDistance7";
            case CapabilityInput::ColorLogicOp:
                return "ColorLogicOp";
            case CapabilityInput::CullFace:
                return "CullFace";
            case CapabilityInput::DebugOutput:
                return "DebugOutput";
            case CapabilityInput::DebugOutputSynchronous:
                return "DebugOutputSynchronous";
            case CapabilityInput::DepthClamp:
                return "DepthClamp";
            case CapabilityInput::DepthTest:
                return "DepthTest";
            case CapabilityInput::Dither:
                return "Dither";
            case CapabilityInput::FramebufferSrgb:
                return "FramebufferSrgb";
            case CapabilityInput::LineSmooth:
                return "LineSmooth";
            case CapabilityInput::Multisample:
                return "Multisample";
            case CapabilityInput::PolygonOffsetFill:
                return "PolygonOffsetFill";
            case CapabilityInput::PolygonOffsetLine:
                return "PolygonOffsetLine";
            case CapabilityInput::PolygonOffsetPoint:
                return "PolygonOffsetPoint";
            case CapabilityInput::PolygonSmooth:
                return "PolygonSmooth";
            case CapabilityInput::PrimitiveRestart:
                return "PrimitiveRestart";
            case CapabilityInput::PrimitiveRestartFixedIndex:
                return "PrimitiveRestartFixedIndex";
            case CapabilityInput::RasterizerDiscard:
                return "RasterizerDiscard";
            case CapabilityInput::SampleAlphaToCoverage:
                return "SampleAlphaToCoverage";
            case CapabilityInput::SampleAlphaToOne:
                return "SampleAlphaToOne";
            case CapabilityInput::SampleCoverage:
                return "SampleCoverage";
            case CapabilityInput::SampleShading:
                return "SampleShading";
            case CapabilityInput::SampleMask:
                return "SampleMask";
            case CapabilityInput::ScissorTest:
                return "ScissorTest";
            case CapabilityInput::StencilTest:
                return "StencilTest";
            case CapabilityInput::TextureCubeMapSeamless:
                return "TextureCubeMapSeamless";
            case CapabilityInput::ProgramPointSize:
                return "ProgramPointSize";
            default:
                return "Unknown";
            }
        }
    } // namespace MG_Util
} // namespace MobileGL