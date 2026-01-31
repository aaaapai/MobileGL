#include "FrameContext.h"
#include "VulkanContext.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    FrameContext::~FrameContext() = default;

    void FrameContext::Initialize(VulkanContext& ctx, VkCommandPool pool) {
        // allocate cmd
        VkCommandBufferAllocateInfo abci{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        abci.commandPool = pool;
        abci.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        abci.commandBufferCount = 1;
        ThrowIfFailed(vkAllocateCommandBuffers(ctx.GetDevice(), &abci, &CommandBuffer), "vkAllocateCommandBuffers");

        // semaphores
        VkSemaphoreCreateInfo sci{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        ThrowIfFailed(vkCreateSemaphore(ctx.GetDevice(), &sci, nullptr, &ImageAvailable),
                      "vkCreateSemaphore ImageAvailable");
        ThrowIfFailed(vkCreateSemaphore(ctx.GetDevice(), &sci, nullptr, &RenderFinished),
                      "vkCreateSemaphore RenderFinished");

        // fence (start signaled)
        VkFenceCreateInfo fci{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        ThrowIfFailed(vkCreateFence(ctx.GetDevice(), &fci, nullptr, &InFlightFence), "vkCreateFence");
    }

    void FrameContext::Cleanup(VulkanContext& ctx) {
        if (ImageAvailable != VK_NULL_HANDLE) {
            vkDestroySemaphore(ctx.GetDevice(), ImageAvailable, nullptr);
            ImageAvailable = VK_NULL_HANDLE;
        }
        if (RenderFinished != VK_NULL_HANDLE) {
            vkDestroySemaphore(ctx.GetDevice(), RenderFinished, nullptr);
            RenderFinished = VK_NULL_HANDLE;
        }
        if (InFlightFence != VK_NULL_HANDLE) {
            vkDestroyFence(ctx.GetDevice(), InFlightFence, nullptr);
            InFlightFence = VK_NULL_HANDLE;
        }

        CommandBuffer = VK_NULL_HANDLE;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
