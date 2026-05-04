// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "Config.h"
#include "../VkIncludes.h"
#include <Includes.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    class PipelineFactory {
    public:
        using HashType = Uint64;

        struct PipelineCreatePayload {
            HashType programHash = 0;
            HashType vertexInputHash = 0;
            VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
            VkRenderPass renderPass = VK_NULL_HANDLE;
            Uint32 colorAttachmentCount = 1;
            Uint32 subpass = 0;
            VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
            VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
            Bool depthTestEnable = false;
            Bool depthWriteEnable = false;
            VkCompareOp depthCompareOp = VK_COMPARE_OP_ALWAYS;
            Bool blendEnable = false;
            VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            VkColorComponentFlags colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            const Vector<VkPipelineShaderStageCreateInfo>* stages = nullptr;
            const VkPipelineVertexInputStateCreateInfo* vertexInputState = nullptr;
        };

        explicit PipelineFactory(VkDevice device, const VulkanRendererConfig& config):
            m_device(device), m_config(config) {}
        ~PipelineFactory();
        PipelineFactory(const PipelineFactory&) = delete;

        HashType ComputeHash(const PipelineCreatePayload& payload) const;
        VkPipeline GetOrCreatePipeline(const PipelineCreatePayload& payload);
        void DestroyAll();

    private:
        VkPipeline CreatePipeline(const PipelineCreatePayload& payload) const;

        VkDevice m_device = VK_NULL_HANDLE;
        const VulkanRendererConfig& m_config;
        UnorderedMap<HashType, VkPipeline> m_cache;
        static inline XXH64_state_t* m_hashState = XXH64_createState();
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
