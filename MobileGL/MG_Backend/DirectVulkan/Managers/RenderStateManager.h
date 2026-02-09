// MobileGL - MobileGL/MG_Backend/DirectVulkan/Managers/RenderStateManager.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include "Includes.h"
#include "MG_State/GLState/RenderState/RenderState.h"
#include "Config.h"
#include "xxhash.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    class RenderStateManager {
    public:
        using RenderStateParameters = MobileGL::RenderStateParameters;
        using HashType = XXH64_hash_t;

        struct RenderStateInfo {
            VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
            Vector<VkPipelineColorBlendAttachmentState> ColorBlendAttachmentStates;
            VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
            VkPipelineViewportStateCreateInfo ViewportStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
            VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
            VkViewport Viewport{};
            VkRect2D Scissor{};
        };

        RenderStateManager();
        ~RenderStateManager();

        RenderStateInfo& CreatePipelineRenderState(const RenderStateParameters& renderState, Uint colorAttachmentCount);
        RenderStateInfo* GetPipelineRenderState(HashType hash);
        RenderStateInfo* GetPipelineRenderState(const RenderStateParameters& renderState, Uint colorAttachmentCount);

    private:
        void Cleanup();
        void PatchCreateInfoPointers(RenderStateInfo& stateInfo);
        HashType GetHash(const RenderStateParameters& renderState, Uint colorAttachmentCount);
        VkBlendFactor ResolveBlendFactor(BlendFactor factor) const;
        VkCompareOp ResolveDepthCompareOp(DepthTestFunc func) const;
        VkCullModeFlags ResolveCullMode(CullFaceMode mode) const;
        VkColorComponentFlags ResolveColorWriteMask(const BoolVec4& mask) const;

        UnorderedMap<HashType, RenderStateInfo> m_renderStateInfo;
        XXH64_state_t* const m_hashState = XXH64_createState();
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
