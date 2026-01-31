#pragma once
#include <Includes.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    class VulkanContext;

    struct FrameContext {
        FrameContext() = default;
        ~FrameContext();

        void Initialize(VulkanContext& ctx, VkCommandPool pool);
        void Cleanup(VulkanContext& ctx);

        VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
        VkSemaphore ImageAvailable = VK_NULL_HANDLE;
        VkSemaphore RenderFinished = VK_NULL_HANDLE;
        VkFence InFlightFence = VK_NULL_HANDLE;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan