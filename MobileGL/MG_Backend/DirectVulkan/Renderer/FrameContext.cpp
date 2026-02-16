// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/FrameContext.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "FrameContext.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    VkResult FrameContext::Initialize(VkDevice device, VkCommandPool commandPool, Uint32 frameCount) {
        Destroy(device, commandPool);
        m_frames.assign(frameCount, {});
        currentFrameIndex = 0;

        Vector<VkCommandBuffer> commandBuffers(frameCount, VK_NULL_HANDLE);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = frameCount;
        VkResult result = vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
        if (result != VK_SUCCESS) {
            return result;
        }
        for (Uint32 i = 0; i < frameCount; ++i) {
            m_frames[i].commandBuffer = commandBuffers[i];
        }

        VkSemaphoreCreateInfo semaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (Uint32 i = 0; i < frameCount; ++i) {
            result = CreateSyncObjectsForFrame(device, i, semaphoreInfo, fenceInfo);
            if (result != VK_SUCCESS) {
                Destroy(device, commandPool);
                return result;
            }
        }

        return VK_SUCCESS;
    }

    void FrameContext::Destroy(VkDevice device, VkCommandPool commandPool) {
        const Uint32 frameCount = static_cast<Uint32>(m_frames.size());
        Vector<VkCommandBuffer> commandBuffers(frameCount, VK_NULL_HANDLE);
        for (Uint32 i = 0; i < frameCount; ++i) {
            commandBuffers[i] = m_frames[i].commandBuffer;
        }

        for (Uint32 i = 0; i < frameCount; ++i) {
            DestroySyncObjectsForFrame(device, i);
        }
        if (device != VK_NULL_HANDLE && commandPool != VK_NULL_HANDLE && !m_frames.empty()) {
            vkFreeCommandBuffers(device, commandPool, frameCount, commandBuffers.data());
        }
        m_frames.clear();
        currentFrameIndex = 0;
    }

    FrameContext::FrameData& FrameContext::GetCurrent() {
        MOBILEGL_ASSERT(!m_frames.empty(), "FrameContext is not initialized");
        return m_frames[currentFrameIndex];
    }

    const FrameContext::FrameData& FrameContext::GetCurrent() const {
        MOBILEGL_ASSERT(!m_frames.empty(), "FrameContext is not initialized");
        return m_frames[currentFrameIndex];
    }

    void FrameContext::AdvanceToNext() {
        MOBILEGL_ASSERT(!m_frames.empty(), "FrameContext is not initialized");
        currentFrameIndex = (currentFrameIndex + 1) % static_cast<Uint32>(m_frames.size());
        GetCurrent().hasCommandBufferRecorded = false;
    }

    Bool FrameContext::TransitionToPresent(VkImage image, VkImageLayout oldLayout, VkImageLayout presentLayout) {
        auto& frame = GetCurrent();
        if (frame.hasCommandBufferRecorded || oldLayout == presentLayout ||
            oldLayout == VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR) {
            return false;
        }

        VK_VERIFY(vkResetCommandBuffer(frame.commandBuffer, 0), "TransitionToPresent, vkResetCommandBuffer");

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VK_VERIFY(vkBeginCommandBuffer(frame.commandBuffer, &beginInfo), "TransitionToPresent, vkBeginCommandBuffer");

        VkImageMemoryBarrier presentBarrier{};
        presentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        presentBarrier.srcAccessMask = 0;
        presentBarrier.dstAccessMask = 0;
        presentBarrier.oldLayout = oldLayout;
        presentBarrier.newLayout = presentLayout;
        presentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        presentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        presentBarrier.image = image;
        presentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        presentBarrier.subresourceRange.baseMipLevel = 0;
        presentBarrier.subresourceRange.levelCount = 1;
        presentBarrier.subresourceRange.baseArrayLayer = 0;
        presentBarrier.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(frame.commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &presentBarrier);

        VK_VERIFY(vkEndCommandBuffer(frame.commandBuffer), "TransitionToPresent, vkEndCommandBuffer");
        return true;
    }

    FrameContext::SubmitInfoPacket FrameContext::GetSubmitInfo(Bool shouldSubmitCommandBuffer) const {
        const auto& frame = GetCurrent();
        SubmitInfoPacket packet{};
        packet.waitSemaphore = frame.imageAvailableSemaphore;
        packet.signalSemaphore = frame.renderFinishedSemaphore;
        packet.commandBuffer = frame.commandBuffer;

        packet.submitInfo.waitSemaphoreCount = 1;
        packet.submitInfo.pWaitSemaphores = &packet.waitSemaphore;
        packet.submitInfo.pWaitDstStageMask = &packet.waitDstStageMask;
        packet.submitInfo.commandBufferCount = shouldSubmitCommandBuffer ? 1U : 0U;
        packet.submitInfo.pCommandBuffers = shouldSubmitCommandBuffer ? &packet.commandBuffer : nullptr;
        packet.submitInfo.signalSemaphoreCount = 1;
        packet.submitInfo.pSignalSemaphores = &packet.signalSemaphore;
        return packet;
    }

    FrameContext::PresentInfoPacket FrameContext::GetPresentInfo(VkSwapchainKHR swapchain, const Uint32& imageIndex) const {
        const auto& frame = GetCurrent();
        PresentInfoPacket packet{};
        packet.waitSemaphore = frame.renderFinishedSemaphore;
        packet.swapchain = swapchain;
        packet.imageIndex = &imageIndex;

        packet.presentInfo.waitSemaphoreCount = 1;
        packet.presentInfo.pWaitSemaphores = &packet.waitSemaphore;
        packet.presentInfo.swapchainCount = 1;
        packet.presentInfo.pSwapchains = &packet.swapchain;
        packet.presentInfo.pImageIndices = packet.imageIndex;
        packet.presentInfo.pResults = nullptr;
        return packet;
    }

    VkResult FrameContext::WaitAndAcquireNextImage(VkDevice device, VkSwapchainKHR swapchain, Uint32& outImageIndex,
                                                   Uint64 timeout, VkFence acquireFence) {
        auto& frame = GetCurrent();
        VkResult result = vkWaitForFences(device, 1, &frame.imageInFlightFence, VK_TRUE, timeout);
        if (result != VK_SUCCESS) {
            return result;
        }

        result = vkResetFences(device, 1, &frame.imageInFlightFence);
        if (result != VK_SUCCESS) {
            return result;
        }

        return vkAcquireNextImageKHR(device, swapchain, timeout, frame.imageAvailableSemaphore, acquireFence,
                                     &outImageIndex);
    }

    Uint32 FrameContext::GetCurrentFrameIndex() const {
        return currentFrameIndex;
    }

    Uint32 FrameContext::GetFrameCount() const {
        return static_cast<Uint32>(m_frames.size());
    }

    void FrameContext::AssertValidFrameIndex(Uint32 frameIndex) const {
        MOBILEGL_ASSERT(frameIndex < m_frames.size(), "FrameContext index out of range");
    }

    VkResult FrameContext::CreateSyncObjectsForFrame(VkDevice device, Uint32 frameIndex,
                                                     const VkSemaphoreCreateInfo& semaphoreInfo,
                                                     const VkFenceCreateInfo& fenceInfo) {
        AssertValidFrameIndex(frameIndex);
        DestroySyncObjectsForFrame(device, frameIndex);

        auto& frame = m_frames[frameIndex];
        VkResult result =
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frame.imageAvailableSemaphore);
        if (result != VK_SUCCESS) {
            return result;
        }

        result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frame.renderFinishedSemaphore);
        if (result != VK_SUCCESS) {
            vkDestroySemaphore(device, frame.imageAvailableSemaphore, nullptr);
            frame.imageAvailableSemaphore = VK_NULL_HANDLE;
            return result;
        }

        result = vkCreateFence(device, &fenceInfo, nullptr, &frame.imageInFlightFence);
        if (result != VK_SUCCESS) {
            vkDestroySemaphore(device, frame.renderFinishedSemaphore, nullptr);
            vkDestroySemaphore(device, frame.imageAvailableSemaphore, nullptr);
            frame.renderFinishedSemaphore = VK_NULL_HANDLE;
            frame.imageAvailableSemaphore = VK_NULL_HANDLE;
            return result;
        }

        frame.hasCommandBufferRecorded = false;
        return VK_SUCCESS;
    }

    void FrameContext::DestroySyncObjectsForFrame(VkDevice device, Uint32 frameIndex) {
        AssertValidFrameIndex(frameIndex);
        auto& frame = m_frames[frameIndex];
        if (device != VK_NULL_HANDLE && frame.imageInFlightFence != VK_NULL_HANDLE) {
            vkDestroyFence(device, frame.imageInFlightFence, nullptr);
        }
        frame.imageInFlightFence = VK_NULL_HANDLE;

        if (device != VK_NULL_HANDLE && frame.renderFinishedSemaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(device, frame.renderFinishedSemaphore, nullptr);
        }
        frame.renderFinishedSemaphore = VK_NULL_HANDLE;

        if (device != VK_NULL_HANDLE && frame.imageAvailableSemaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(device, frame.imageAvailableSemaphore, nullptr);
        }
        frame.imageAvailableSemaphore = VK_NULL_HANDLE;
        frame.hasCommandBufferRecorded = false;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
