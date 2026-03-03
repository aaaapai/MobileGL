// MobileGL - MobileGL/MG_Util/Converters/MGToVk/RenderStateEnumConverter.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "RenderStateEnumConverter.h"

namespace MobileGL {
    namespace MG_Util {
        VkPrimitiveTopology ConvertPrimitiveModeToVkEnum(GLenum mode) {
            switch (mode) {
            case GL_POINTS:
                return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            case GL_LINES:
                return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case GL_LINE_STRIP:
                return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            case GL_TRIANGLES:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case GL_TRIANGLE_STRIP:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            case GL_TRIANGLE_FAN:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            case GL_LINE_LOOP:
            default:
                MGLOG_W("Unrecognized primitive topology");
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            }
        }

        VkCullModeFlags ConvertCullFaceModeToVkEnum(CullFaceMode v) {
            switch (v) {
            case CullFaceMode::Front:
                return VK_CULL_MODE_FRONT_BIT;
            case CullFaceMode::Back:
                return VK_CULL_MODE_BACK_BIT;
            case CullFaceMode::FrontAndBack:
                return VK_CULL_MODE_FRONT_AND_BACK;
            case CullFaceMode::Unknown:
            case CullFaceMode::CullFaceModeCount:
            default:
                MGLOG_W("Unrecognized cull face mode");
                return VK_CULL_MODE_BACK_BIT;
            }
        }

        VkCompareOp ConvertDepthTestFuncToVkEnum(DepthTestFunc v) {
            switch (v) {
            case DepthTestFunc::Never:
                return VK_COMPARE_OP_NEVER;
            case DepthTestFunc::Less:
                return VK_COMPARE_OP_LESS;
            case DepthTestFunc::Equal:
                return VK_COMPARE_OP_EQUAL;
            case DepthTestFunc::LessEqual:
                return VK_COMPARE_OP_LESS_OR_EQUAL;
            case DepthTestFunc::Greater:
                return VK_COMPARE_OP_GREATER;
            case DepthTestFunc::NotEqual:
                return VK_COMPARE_OP_NOT_EQUAL;
            case DepthTestFunc::GreaterEqual:
                return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case DepthTestFunc::Always:
            default:
                return VK_COMPARE_OP_ALWAYS;
            }
        }

        VkBlendFactor ConvertBlendFactorToVkEnum(BlendFactor v) {
            switch (v) {
            case BlendFactor::Zero:
                return VK_BLEND_FACTOR_ZERO;
            case BlendFactor::One:
                return VK_BLEND_FACTOR_ONE;
            case BlendFactor::SrcColor:
                return VK_BLEND_FACTOR_SRC_COLOR;
            case BlendFactor::OneMinusSrcColor:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            case BlendFactor::DstColor:
                return VK_BLEND_FACTOR_DST_COLOR;
            case BlendFactor::OneMinusDstColor:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            case BlendFactor::SrcAlpha:
                return VK_BLEND_FACTOR_SRC_ALPHA;
            case BlendFactor::OneMinusSrcAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case BlendFactor::DstAlpha:
                return VK_BLEND_FACTOR_DST_ALPHA;
            case BlendFactor::OneMinusDstAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            case BlendFactor::ConstantColor:
                return VK_BLEND_FACTOR_CONSTANT_COLOR;
            case BlendFactor::OneMinusConstantColor:
                return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
            case BlendFactor::ConstantAlpha:
                return VK_BLEND_FACTOR_CONSTANT_ALPHA;
            case BlendFactor::OneMinusConstantAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
            default:
                return VK_BLEND_FACTOR_ONE;
            }
        }
    } // namespace MG_Util
} // namespace MobileGL
