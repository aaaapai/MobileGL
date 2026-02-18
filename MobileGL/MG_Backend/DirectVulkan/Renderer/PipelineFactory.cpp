// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineFactory.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "PipelineFactory.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    PipelineFactory::~PipelineFactory() {
        DestroyAll();
    }

    PipelineFactory::HashType PipelineFactory::ComputeHash(const PipelineCreatePayload& payload) const {
        XXHASH_VERIFY(XXH64_reset(m_hashState, m_config.CacheVersion));
        XXHASH_VERIFY(XXH64_update(m_hashState, &payload.programHash, sizeof(payload.programHash)));
        XXHASH_VERIFY(XXH64_update(m_hashState, &payload.vertexInputHash, sizeof(payload.vertexInputHash)));
        XXHASH_VERIFY(XXH64_update(m_hashState, &payload.pipelineLayout, sizeof(payload.pipelineLayout)));
        XXHASH_VERIFY(XXH64_update(m_hashState, &payload.renderPass, sizeof(payload.renderPass)));
        XXHASH_VERIFY(XXH64_update(m_hashState, &payload.subpass, sizeof(payload.subpass)));
        XXHASH_VERIFY(XXH64_update(m_hashState, &payload.topology, sizeof(payload.topology)));
        XXHASH_VERIFY(XXH64_update(m_hashState, &payload.depthTestEnable, sizeof(payload.depthTestEnable)));
        XXHASH_VERIFY(XXH64_update(m_hashState, &payload.depthWriteEnable, sizeof(payload.depthWriteEnable)));
        XXHASH_VERIFY(XXH64_update(m_hashState, &payload.depthCompareOp, sizeof(payload.depthCompareOp)));
        return XXH64_digest(m_hashState);
    }

    VkPipeline PipelineFactory::GetOrCreatePipeline(const PipelineCreatePayload& payload) {
        const HashType hash = ComputeHash(payload);
        auto it = m_cache.find(hash);
        if (it != m_cache.end()) {
            return it->second;
        }

        VkPipeline pipeline = CreatePipeline(payload);
        m_cache.emplace(hash, pipeline);
        return pipeline;
    }

    void PipelineFactory::DestroyAll() {
        for (auto& pair : m_cache) {
            if (pair.second != VK_NULL_HANDLE) {
                vkDestroyPipeline(m_device, pair.second, nullptr);
            }
        }
        m_cache.clear();
    }

    VkPipeline PipelineFactory::CreatePipeline(const PipelineCreatePayload& payload) const {
        MOBILEGL_ASSERT(payload.stages != nullptr && !payload.stages->empty(), "PipelineFactory: stages are empty");
        MOBILEGL_ASSERT(payload.vertexInputState != nullptr, "PipelineFactory: vertexInputState is null");
        MOBILEGL_ASSERT(payload.pipelineLayout != VK_NULL_HANDLE, "PipelineFactory: pipelineLayout is null");
        MOBILEGL_ASSERT(payload.renderPass != VK_NULL_HANDLE, "PipelineFactory: renderPass is null");

        static constexpr VkDynamicState kDynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(std::size(kDynamicStates));
        dynamicState.pDynamicStates = kDynamicStates;

        VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        ia.topology = payload.topology;

        VkPipelineViewportStateCreateInfo vpci{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        vpci.viewportCount = 1;
        vpci.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        raster.polygonMode = VK_POLYGON_MODE_FILL;
        raster.cullMode = VK_CULL_MODE_NONE;
        raster.frontFace = VK_FRONT_FACE_CLOCKWISE;
        raster.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        depthStencil.depthTestEnable = payload.depthTestEnable ? VK_TRUE : VK_FALSE;
        depthStencil.depthWriteEnable = payload.depthWriteEnable ? VK_TRUE : VK_FALSE;
        depthStencil.depthCompareOp = payload.depthCompareOp;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorAttach{};
        colorAttach.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorAttach.blendEnable = VK_FALSE;
        VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        blend.attachmentCount = 1;
        blend.pAttachments = &colorAttach;

        VkGraphicsPipelineCreateInfo gpi{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        gpi.stageCount = static_cast<Uint32>(payload.stages->size());
        gpi.pStages = payload.stages->data();
        gpi.pVertexInputState = payload.vertexInputState;
        gpi.pInputAssemblyState = &ia;
        gpi.pViewportState = &vpci;
        gpi.pRasterizationState = &raster;
        gpi.pMultisampleState = &ms;
        gpi.pDepthStencilState = &depthStencil;
        gpi.pColorBlendState = &blend;
        gpi.pDynamicState = &dynamicState;
        gpi.layout = payload.pipelineLayout;
        gpi.renderPass = payload.renderPass;
        gpi.subpass = payload.subpass;

        VkPipeline pipeline = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &gpi, nullptr, &pipeline),
                  "vkCreateGraphicsPipelines");
        return pipeline;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
