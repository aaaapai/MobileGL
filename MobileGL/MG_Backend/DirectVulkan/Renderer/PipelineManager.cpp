// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "PipelineManager.h"
#include "VulkanContext.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    PipelineManager::PipelineManager(VulkanContext& ctx) : Ctx(ctx) {}
    PipelineManager::~PipelineManager() {
        Cleanup();
    }

    void PipelineManager::EnsurePipelineLayout() {
        if (PipelineLayout != VK_NULL_HANDLE) return;
        VkPipelineLayoutCreateInfo plci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        VK_VERIFY(vkCreatePipelineLayout(Ctx.GetDevice(), &plci, nullptr, &PipelineLayout), "vkCreatePipelineLayout");
    }

    VkPipeline PipelineManager::CreateGraphicsPipelineFromSpv(const std::string& key,
                                                              const std::vector<uint32_t>& vsSpv,
                                                              const std::vector<uint32_t>& fsSpv,
                                                              VkRenderPass renderPass, VkExtent2D extent) {
        if (Pipelines.find(key) != Pipelines.end()) {
            MGLOG_W("Pipeline key '%s' already exists - returning existing", key.c_str());
            return Pipelines[key];
        }
        EnsurePipelineLayout();

        VkShaderModuleCreateInfo smci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        smci.codeSize = vsSpv.size() * sizeof(uint32_t);
        smci.pCode = vsSpv.data();
        VkShaderModule vs;
        VK_VERIFY(vkCreateShaderModule(Ctx.GetDevice(), &smci, nullptr, &vs), "vkCreateShaderModule VS");

        smci.codeSize = fsSpv.size() * sizeof(uint32_t);
        smci.pCode = fsSpv.data();
        VkShaderModule fs;
        VK_VERIFY(vkCreateShaderModule(Ctx.GetDevice(), &smci, nullptr, &fs), "vkCreateShaderModule FS");

        VkPipelineShaderStageCreateInfo stages[2]{};
        stages[0] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = vs;
        stages[0].pName = "main";
        stages[1] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = fs;
        stages[1].pName = "main";

        VkPipelineVertexInputStateCreateInfo vertexInput{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        vertexInput.vertexBindingDescriptionCount = 0;
        vertexInput.vertexAttributeDescriptionCount = 0;

        VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkViewport vp{};
        vp.x = 0;
        vp.y = 0;
        vp.width = (float)extent.width;
        vp.height = (float)extent.height;
        vp.minDepth = 0;
        vp.maxDepth = 1;
        VkRect2D scissor{{0, 0}, extent};
        VkPipelineViewportStateCreateInfo vpci{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        vpci.viewportCount = 1;
        vpci.pViewports = &vp;
        vpci.scissorCount = 1;
        vpci.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        raster.polygonMode = VK_POLYGON_MODE_FILL;
        raster.cullMode = VK_CULL_MODE_NONE;
        raster.frontFace = VK_FRONT_FACE_CLOCKWISE;
        raster.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorAttach{};
        colorAttach.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorAttach.blendEnable = VK_FALSE;
        VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        blend.attachmentCount = 1;
        blend.pAttachments = &colorAttach;

        VkGraphicsPipelineCreateInfo gpi{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        gpi.stageCount = 2;
        gpi.pStages = stages;
        gpi.pVertexInputState = &vertexInput;
        gpi.pInputAssemblyState = &ia;
        gpi.pViewportState = &vpci;
        gpi.pRasterizationState = &raster;
        gpi.pMultisampleState = &ms;
        gpi.pColorBlendState = &blend;
        gpi.layout = PipelineLayout;
        gpi.renderPass = renderPass;
        gpi.subpass = 0;

        VkPipeline pipeline;
        VK_VERIFY(vkCreateGraphicsPipelines(Ctx.GetDevice(), VK_NULL_HANDLE, 1, &gpi, nullptr, &pipeline),
                  "vkCreateGraphicsPipelines");

        vkDestroyShaderModule(Ctx.GetDevice(), vs, nullptr);
        vkDestroyShaderModule(Ctx.GetDevice(), fs, nullptr);

        Pipelines.emplace(key, pipeline);
        MGLOG_D("Pipeline '%s' created", key.c_str());
        return pipeline;
    }

    VkPipeline PipelineManager::GetPipeline(const std::string& key) const {
        auto it = Pipelines.find(key);
        if (it == Pipelines.end()) return VK_NULL_HANDLE;
        return it->second;
    }

    void PipelineManager::DestroyPipeline(const std::string& key) {
        auto it = Pipelines.find(key);
        if (it != Pipelines.end()) {
            vkDestroyPipeline(Ctx.GetDevice(), it->second, nullptr);
            Pipelines.erase(it);
        }
    }

    void PipelineManager::Cleanup() {
        for (auto& kv : Pipelines)
            vkDestroyPipeline(Ctx.GetDevice(), kv.second, nullptr);
        Pipelines.clear();
        if (PipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(Ctx.GetDevice(), PipelineLayout, nullptr);
            PipelineLayout = VK_NULL_HANDLE;
        }
    }
} // namespace MobileGL::MG_Backend::DirectVulkan