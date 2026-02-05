// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/PipelineManager.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    class VulkanContext;

    class PipelineManager {
    public:
        PipelineManager(VulkanContext& ctx);
        ~PipelineManager();

        VkPipeline CreateGraphicsPipelineFromSpv(const std::string& key, const std::vector<uint32_t>& vsSpv,
                                                 const std::vector<uint32_t>& fsSpv, VkRenderPass renderPass,
                                                 VkExtent2D extent);

        VkPipeline GetPipeline(const std::string& key) const;
        void DestroyPipeline(const std::string& key);
        void Cleanup();

    private:
        VulkanContext& Ctx;
        VkPipelineLayout PipelineLayout = VK_NULL_HANDLE; // TODO: per-pipeline layouts?
        std::unordered_map<std::string, VkPipeline> Pipelines;

        void EnsurePipelineLayout();
    };
} // namespace MobileGL::MG_Backend::DirectVulkan