// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/FrameContext.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

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
        VK_VERIFY(vkAllocateCommandBuffers(ctx.GetDevice(), &abci, &CommandBuffer), "vkAllocateCommandBuffers");

        // semaphores
        VkSemaphoreCreateInfo sci{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VK_VERIFY(vkCreateSemaphore(ctx.GetDevice(), &sci, nullptr, &ImageAvailable),
                  "vkCreateSemaphore ImageAvailable");
        VK_VERIFY(vkCreateSemaphore(ctx.GetDevice(), &sci, nullptr, &RenderFinished),
                  "vkCreateSemaphore RenderFinished");

        // fence (start signaled)
        VkFenceCreateInfo fci{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_VERIFY(vkCreateFence(ctx.GetDevice(), &fci, nullptr, &InFlightFence), "vkCreateFence");
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
