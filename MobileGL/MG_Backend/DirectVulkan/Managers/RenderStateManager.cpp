// MobileGL - MobileGL/MG_Backend/DirectVulkan/Managers/RenderStateManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "RenderStateManager.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    RenderStateManager::RenderStateManager() {}

    RenderStateManager::~RenderStateManager() {
        Cleanup();
    }

    void RenderStateManager::Cleanup() {
        m_renderStateInfo.clear();
        XXH64_freeState(m_hashState);
    }

    void RenderStateManager::PatchCreateInfoPointers(RenderStateInfo& stateInfo) {
        stateInfo.ColorBlendStateCreateInfo.attachmentCount =
            static_cast<Uint>(stateInfo.ColorBlendAttachmentStates.size());
        stateInfo.ColorBlendStateCreateInfo.pAttachments =
            stateInfo.ColorBlendAttachmentStates.empty() ? nullptr : stateInfo.ColorBlendAttachmentStates.data();

        stateInfo.ViewportStateCreateInfo.viewportCount = 1;
        stateInfo.ViewportStateCreateInfo.pViewports = &stateInfo.Viewport;
        stateInfo.ViewportStateCreateInfo.scissorCount = 1;
        stateInfo.ViewportStateCreateInfo.pScissors = &stateInfo.Scissor;
    }

    RenderStateManager::RenderStateInfo&
    RenderStateManager::CreatePipelineRenderState(const RenderStateParameters& renderState, Uint colorAttachmentCount) {
        auto hash = GetHash(renderState, colorAttachmentCount);
        MOBILEGL_ASSERT(m_renderStateInfo.find(hash) == m_renderStateInfo.end(),
                        "A render state with the same hash has already been created");
        MOBILEGL_ASSERT(colorAttachmentCount > 0, "colorAttachmentCount must be greater than zero");
        MOBILEGL_ASSERT(colorAttachmentCount <= MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS,
                        "colorAttachmentCount exceeds max draw buffers");

        RenderStateInfo stateInfo;

        MOBILEGL_ASSERT(renderState.Viewport.z() >= 0 && renderState.Viewport.w() >= 0,
                        "Viewport width/height must be non-negative");
        stateInfo.Viewport.x = static_cast<Float>(renderState.Viewport.x());
        stateInfo.Viewport.y = static_cast<Float>(renderState.Viewport.y());
        stateInfo.Viewport.width = static_cast<Float>(renderState.Viewport.z());
        stateInfo.Viewport.height = static_cast<Float>(renderState.Viewport.w());
        stateInfo.Viewport.minDepth = 0.0f;
        stateInfo.Viewport.maxDepth = 1.0f;

        if (renderState.ScissorTestEnabled) {
            MOBILEGL_ASSERT(renderState.ScissorBox.z() >= 0 && renderState.ScissorBox.w() >= 0,
                            "Scissor width/height must be non-negative");
            stateInfo.Scissor.offset.x = renderState.ScissorBox.x();
            stateInfo.Scissor.offset.y = renderState.ScissorBox.y();
            stateInfo.Scissor.extent.width = static_cast<Uint>(renderState.ScissorBox.z());
            stateInfo.Scissor.extent.height = static_cast<Uint>(renderState.ScissorBox.w());
        } else {
            stateInfo.Scissor.offset.x = renderState.Viewport.x();
            stateInfo.Scissor.offset.y = renderState.Viewport.y();
            stateInfo.Scissor.extent.width = static_cast<Uint>(renderState.Viewport.z());
            stateInfo.Scissor.extent.height = static_cast<Uint>(renderState.Viewport.w());
        }

        stateInfo.RasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        stateInfo.RasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        stateInfo.RasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        stateInfo.RasterizationStateCreateInfo.cullMode =
            renderState.CullFaceEnabled ? ResolveCullMode(renderState.CullFaceModeSetting) : VK_CULL_MODE_NONE;
        stateInfo.RasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        stateInfo.RasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
        stateInfo.RasterizationStateCreateInfo.lineWidth = 1.0f;

        stateInfo.DepthStencilStateCreateInfo.depthTestEnable = renderState.DepthTestEnabled ? VK_TRUE : VK_FALSE;
        stateInfo.DepthStencilStateCreateInfo.depthWriteEnable = renderState.DepthMask ? VK_TRUE : VK_FALSE;
        stateInfo.DepthStencilStateCreateInfo.depthCompareOp = ResolveDepthCompareOp(renderState.DepthFunc);
        stateInfo.DepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        stateInfo.DepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
        stateInfo.DepthStencilStateCreateInfo.minDepthBounds = 0.0f;
        stateInfo.DepthStencilStateCreateInfo.maxDepthBounds = 1.0f;

        stateInfo.ColorBlendAttachmentStates.reserve(colorAttachmentCount);
        for (Uint i = 0; i < colorAttachmentCount; ++i) {
            const auto& blendState = renderState.BlendStates[i];
            VkPipelineColorBlendAttachmentState attachment{};
            attachment.blendEnable = blendState.Enabled ? VK_TRUE : VK_FALSE;
            attachment.srcColorBlendFactor = ResolveBlendFactor(blendState.SrcFactorRGB);
            attachment.dstColorBlendFactor = ResolveBlendFactor(blendState.DstFactorRGB);
            attachment.colorBlendOp = VK_BLEND_OP_ADD;
            attachment.srcAlphaBlendFactor = ResolveBlendFactor(blendState.SrcFactorAlpha);
            attachment.dstAlphaBlendFactor = ResolveBlendFactor(blendState.DstFactorAlpha);
            attachment.alphaBlendOp = VK_BLEND_OP_ADD;
            attachment.colorWriteMask = ResolveColorWriteMask(renderState.ColorMask);
            stateInfo.ColorBlendAttachmentStates.emplace_back(attachment);
        }

        stateInfo.ColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        stateInfo.ColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
        stateInfo.ColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
        stateInfo.ColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
        stateInfo.ColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
        stateInfo.ColorBlendStateCreateInfo.blendConstants[3] = 0.0f;

        PatchCreateInfoPointers(stateInfo);

        auto [it, inserted] = m_renderStateInfo.emplace(hash, Move(stateInfo));
        MOBILEGL_ASSERT(inserted, "Failed to cache pipeline render state create info");
        PatchCreateInfoPointers(it->second);
        return it->second;
    }

    RenderStateManager::RenderStateInfo* RenderStateManager::GetPipelineRenderState(HashType hash) {
        auto it = m_renderStateInfo.find(hash);
        if (it == m_renderStateInfo.end()) return nullptr;
        return &it->second;
    }

    RenderStateManager::RenderStateInfo*
    RenderStateManager::GetPipelineRenderState(const RenderStateParameters& renderState, Uint colorAttachmentCount) {
        return GetPipelineRenderState(GetHash(renderState, colorAttachmentCount));
    }

    RenderStateManager::HashType RenderStateManager::GetHash(const RenderStateParameters& renderState,
                                                             Uint colorAttachmentCount) {
        MOBILEGL_ASSERT(m_hashState != nullptr, "Hash state should already be initialized");
        MOBILEGL_ASSERT(colorAttachmentCount > 0, "colorAttachmentCount must be greater than zero");
        MOBILEGL_ASSERT(colorAttachmentCount <= MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS,
                        "colorAttachmentCount exceeds max draw buffers");

        XXH64_hash_t const seed = MG_Config::CacheVersion;
        auto errc = XXH64_reset(m_hashState, seed);
        MOBILEGL_ASSERT(errc == XXH_OK, "Hash state reset failed");

        const Int viewport[4] = {renderState.Viewport.x(), renderState.Viewport.y(), renderState.Viewport.z(),
                                 renderState.Viewport.w()};
        errc = XXH64_update(m_hashState, viewport, sizeof(viewport));
        MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");

        for (Uint i = 0; i < colorAttachmentCount; ++i) {
            const auto& blendState = renderState.BlendStates[i];
            errc = XXH64_update(m_hashState, &blendState.Enabled, sizeof(blendState.Enabled));
            MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
            errc = XXH64_update(m_hashState, &blendState.SrcFactorRGB, sizeof(blendState.SrcFactorRGB));
            MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
            errc = XXH64_update(m_hashState, &blendState.DstFactorRGB, sizeof(blendState.DstFactorRGB));
            MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
            errc = XXH64_update(m_hashState, &blendState.SrcFactorAlpha, sizeof(blendState.SrcFactorAlpha));
            MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
            errc = XXH64_update(m_hashState, &blendState.DstFactorAlpha, sizeof(blendState.DstFactorAlpha));
            MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
        }

        errc = XXH64_update(m_hashState, &renderState.DepthTestEnabled, sizeof(renderState.DepthTestEnabled));
        MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
        errc = XXH64_update(m_hashState, &renderState.DepthFunc, sizeof(renderState.DepthFunc));
        MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
        errc = XXH64_update(m_hashState, &renderState.DepthMask, sizeof(renderState.DepthMask));
        MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");

        const Bool colorMask[4] = {renderState.ColorMask.x(), renderState.ColorMask.y(), renderState.ColorMask.z(),
                                   renderState.ColorMask.w()};
        errc = XXH64_update(m_hashState, colorMask, sizeof(colorMask));
        MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");

        errc = XXH64_update(m_hashState, &renderState.CullFaceEnabled, sizeof(renderState.CullFaceEnabled));
        MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
        errc = XXH64_update(m_hashState, &renderState.CullFaceModeSetting, sizeof(renderState.CullFaceModeSetting));
        MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");

        errc = XXH64_update(m_hashState, &renderState.ScissorTestEnabled, sizeof(renderState.ScissorTestEnabled));
        MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
        const Int scissor[4] = {renderState.ScissorBox.x(), renderState.ScissorBox.y(), renderState.ScissorBox.z(),
                                renderState.ScissorBox.w()};
        errc = XXH64_update(m_hashState, scissor, sizeof(scissor));
        MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");

        errc = XXH64_update(m_hashState, &colorAttachmentCount, sizeof(colorAttachmentCount));
        MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");

        return XXH64_digest(m_hashState);
    }

    VkBlendFactor RenderStateManager::ResolveBlendFactor(BlendFactor factor) {
        switch (factor) {
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
            MOBILEGL_ASSERT(false, "Unsupported blend factor");
            return VK_BLEND_FACTOR_ONE;
        }
    }

    VkCompareOp RenderStateManager::ResolveDepthCompareOp(DepthTestFunc func) {
        switch (func) {
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
            return VK_COMPARE_OP_ALWAYS;
        default:
            MOBILEGL_ASSERT(false, "Unsupported depth compare function");
            return VK_COMPARE_OP_LESS;
        }
    }

    VkCullModeFlags RenderStateManager::ResolveCullMode(CullFaceMode mode) {
        switch (mode) {
        case CullFaceMode::Front:
            return VK_CULL_MODE_FRONT_BIT;
        case CullFaceMode::Back:
            return VK_CULL_MODE_BACK_BIT;
        case CullFaceMode::FrontAndBack:
            return VK_CULL_MODE_FRONT_AND_BACK;
        default:
            MOBILEGL_ASSERT(false, "Unsupported cull face mode");
            return VK_CULL_MODE_BACK_BIT;
        }
    }

    VkColorComponentFlags RenderStateManager::ResolveColorWriteMask(const BoolVec4& mask) {
        VkColorComponentFlags vkMask = 0;
        if (mask.x()) vkMask |= VK_COLOR_COMPONENT_R_BIT;
        if (mask.y()) vkMask |= VK_COLOR_COMPONENT_G_BIT;
        if (mask.z()) vkMask |= VK_COLOR_COMPONENT_B_BIT;
        if (mask.w()) vkMask |= VK_COLOR_COMPONENT_A_BIT;
        return vkMask;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
