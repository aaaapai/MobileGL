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
        commandBuffers.resize(frameCount, VK_NULL_HANDLE);
        imageAvailableSemaphores.resize(frameCount, VK_NULL_HANDLE);
        renderFinishedSemaphores.resize(frameCount, VK_NULL_HANDLE);
        imageInFlightFences.resize(frameCount, VK_NULL_HANDLE);
        hasCommandBufferRecorded.assign(frameCount, false);
        currentFrameIndex = 0;

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = frameCount;
        VkResult result = vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
        if (result != VK_SUCCESS) {
            return result;
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
        const Uint32 frameCount = static_cast<Uint32>(commandBuffers.size());
        for (Uint32 i = 0; i < frameCount; ++i) {
            DestroySyncObjectsForFrame(device, i);
        }
        if (device != VK_NULL_HANDLE && commandPool != VK_NULL_HANDLE && !commandBuffers.empty()) {
            vkFreeCommandBuffers(device, commandPool, frameCount, commandBuffers.data());
        }
        commandBuffers.clear();
        imageAvailableSemaphores.clear();
        renderFinishedSemaphores.clear();
        imageInFlightFences.clear();
        hasCommandBufferRecorded.clear();
        currentFrameIndex = 0;
    }

    void FrameContext::AdvanceFrame() {
        MOBILEGL_ASSERT(!commandBuffers.empty(), "FrameContext is not initialized");
        currentFrameIndex = (currentFrameIndex + 1) % static_cast<Uint32>(commandBuffers.size());
    }

    Uint32 FrameContext::GetCurrentFrameIndex() const {
        return currentFrameIndex;
    }

    Uint32 FrameContext::GetFrameCount() const {
        return static_cast<Uint32>(commandBuffers.size());
    }

    VkCommandBuffer& FrameContext::GetCommandBuffer(Uint32 frameIndex) {
        AssertValidFrameIndex(frameIndex);
        return commandBuffers[frameIndex];
    }

    VkSemaphore& FrameContext::GetImageAvailableSemaphore(Uint32 frameIndex) {
        AssertValidFrameIndex(frameIndex);
        return imageAvailableSemaphores[frameIndex];
    }

    VkSemaphore& FrameContext::GetRenderFinishedSemaphore(Uint32 frameIndex) {
        AssertValidFrameIndex(frameIndex);
        return renderFinishedSemaphores[frameIndex];
    }

    VkFence& FrameContext::GetImageInFlightFence(Uint32 frameIndex) {
        AssertValidFrameIndex(frameIndex);
        return imageInFlightFences[frameIndex];
    }

    void FrameContext::SetHasCommandBufferRecorded(Uint32 frameIndex, Bool value) {
        AssertValidFrameIndex(frameIndex);
        hasCommandBufferRecorded[frameIndex] = value;
    }

    void FrameContext::SetCurrentCommandBuffer(VkCommandBuffer value) {
        SetCurrentHasCommandBufferRecorded(false);
        GetCommandBuffer(GetCurrentFrameIndex()) = value;
    }

    void FrameContext::SetCurrentImageAvailableSemaphore(VkSemaphore value) {
        GetImageAvailableSemaphore(GetCurrentFrameIndex()) = value;
    }

    void FrameContext::SetCurrentRenderFinishedSemaphore(VkSemaphore value) {
        GetRenderFinishedSemaphore(GetCurrentFrameIndex()) = value;
    }

    void FrameContext::SetCurrentImageInFlightFence(VkFence value) {
        GetImageInFlightFence(GetCurrentFrameIndex()) = value;
    }

    void FrameContext::SetCurrentHasCommandBufferRecorded(Bool value) {
        SetHasCommandBufferRecorded(GetCurrentFrameIndex(), value);
    }

    VkCommandBuffer& FrameContext::GetCurrentCommandBuffer() {
        return GetCommandBuffer(GetCurrentFrameIndex());
    }

    VkSemaphore& FrameContext::GetCurrentImageAvailableSemaphore() {
        return GetImageAvailableSemaphore(GetCurrentFrameIndex());
    }

    VkSemaphore& FrameContext::GetCurrentRenderFinishedSemaphore() {
        return GetRenderFinishedSemaphore(GetCurrentFrameIndex());
    }

    VkFence& FrameContext::GetCurrentImageInFlightFence() {
        return GetImageInFlightFence(GetCurrentFrameIndex());
    }

    Bool FrameContext::GetCurrentHasCommandBufferRecorded() const {
        return GetHasCommandBufferRecorded(GetCurrentFrameIndex());
    }

    const VkCommandBuffer& FrameContext::GetCommandBuffer(Uint32 frameIndex) const {
        AssertValidFrameIndex(frameIndex);
        return commandBuffers[frameIndex];
    }

    const VkSemaphore& FrameContext::GetImageAvailableSemaphore(Uint32 frameIndex) const {
        AssertValidFrameIndex(frameIndex);
        return imageAvailableSemaphores[frameIndex];
    }

    const VkSemaphore& FrameContext::GetRenderFinishedSemaphore(Uint32 frameIndex) const {
        AssertValidFrameIndex(frameIndex);
        return renderFinishedSemaphores[frameIndex];
    }

    const VkFence& FrameContext::GetImageInFlightFence(Uint32 frameIndex) const {
        AssertValidFrameIndex(frameIndex);
        return imageInFlightFences[frameIndex];
    }

    Bool FrameContext::GetHasCommandBufferRecorded(Uint32 frameIndex) const {
        AssertValidFrameIndex(frameIndex);
        return hasCommandBufferRecorded[frameIndex];
    }

    const VkCommandBuffer& FrameContext::GetCurrentCommandBuffer() const {
        return GetCommandBuffer(GetCurrentFrameIndex());
    }

    const VkSemaphore& FrameContext::GetCurrentImageAvailableSemaphore() const {
        return GetImageAvailableSemaphore(GetCurrentFrameIndex());
    }

    const VkSemaphore& FrameContext::GetCurrentRenderFinishedSemaphore() const {
        return GetRenderFinishedSemaphore(GetCurrentFrameIndex());
    }

    const VkFence& FrameContext::GetCurrentImageInFlightFence() const {
        return GetImageInFlightFence(GetCurrentFrameIndex());
    }

    void FrameContext::ResetPerFrameState() {
        for (SizeT i = 0; i < hasCommandBufferRecorded.size(); ++i) {
            hasCommandBufferRecorded[i] = false;
        }
    }

    void FrameContext::AssertValidFrameIndex(Uint32 frameIndex) const {
        MOBILEGL_ASSERT(frameIndex < commandBuffers.size(), "FrameContext index out of range");
    }

    VkResult FrameContext::CreateSyncObjectsForFrame(VkDevice device, Uint32 frameIndex,
                                                     const VkSemaphoreCreateInfo& semaphoreInfo,
                                                     const VkFenceCreateInfo& fenceInfo) {
        DestroySyncObjectsForFrame(device, frameIndex);

        VkResult result =
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[frameIndex]);
        if (result != VK_SUCCESS) {
            return result;
        }

        result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[frameIndex]);
        if (result != VK_SUCCESS) {
            vkDestroySemaphore(device, imageAvailableSemaphores[frameIndex], nullptr);
            imageAvailableSemaphores[frameIndex] = VK_NULL_HANDLE;
            return result;
        }

        result = vkCreateFence(device, &fenceInfo, nullptr, &imageInFlightFences[frameIndex]);
        if (result != VK_SUCCESS) {
            vkDestroySemaphore(device, renderFinishedSemaphores[frameIndex], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[frameIndex], nullptr);
            renderFinishedSemaphores[frameIndex] = VK_NULL_HANDLE;
            imageAvailableSemaphores[frameIndex] = VK_NULL_HANDLE;
            return result;
        }

        hasCommandBufferRecorded[frameIndex] = false;
        return VK_SUCCESS;
    }

    void FrameContext::DestroySyncObjectsForFrame(VkDevice device, Uint32 frameIndex) {
        if (device != VK_NULL_HANDLE && imageInFlightFences[frameIndex] != VK_NULL_HANDLE) {
            vkDestroyFence(device, imageInFlightFences[frameIndex], nullptr);
        }
        imageInFlightFences[frameIndex] = VK_NULL_HANDLE;

        if (device != VK_NULL_HANDLE && renderFinishedSemaphores[frameIndex] != VK_NULL_HANDLE) {
            vkDestroySemaphore(device, renderFinishedSemaphores[frameIndex], nullptr);
        }
        renderFinishedSemaphores[frameIndex] = VK_NULL_HANDLE;

        if (device != VK_NULL_HANDLE && imageAvailableSemaphores[frameIndex] != VK_NULL_HANDLE) {
            vkDestroySemaphore(device, imageAvailableSemaphores[frameIndex], nullptr);
        }
        imageAvailableSemaphores[frameIndex] = VK_NULL_HANDLE;
        hasCommandBufferRecorded[frameIndex] = false;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
