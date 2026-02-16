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
