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