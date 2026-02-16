// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/FrameContext.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "../VkIncludes.h"
#include <Includes.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    class FrameContext {
    public:
        VkResult Initialize(VkDevice device, VkCommandPool commandPool, Uint32 frameCount);
        void Destroy(VkDevice device, VkCommandPool commandPool);
        void AdvanceFrame();
        void ResetPerFrameState();

        Uint32 GetCurrentFrameIndex() const;
        Uint32 GetFrameCount() const;

        VkCommandBuffer& GetCommandBuffer(Uint32 frameIndex);
        VkSemaphore& GetImageAvailableSemaphore(Uint32 frameIndex);
        VkSemaphore& GetRenderFinishedSemaphore(Uint32 frameIndex);
        VkFence& GetImageInFlightFence(Uint32 frameIndex);
        void SetHasCommandBufferRecorded(Uint32 frameIndex, Bool value);
        void SetCurrentCommandBuffer(VkCommandBuffer value);
        void SetCurrentImageAvailableSemaphore(VkSemaphore value);
        void SetCurrentRenderFinishedSemaphore(VkSemaphore value);
        void SetCurrentImageInFlightFence(VkFence value);
        void SetCurrentCommandBufferRecorded(Bool value);

        VkCommandBuffer& GetCurrentCommandBuffer();
        VkSemaphore& GetCurrentImageAvailableSemaphore();
        VkSemaphore& GetCurrentRenderFinishedSemaphore();
        VkFence& GetCurrentImageInFlightFence();
        Bool HasCurrentCommandBufferRecorded() const;

        const VkCommandBuffer& GetCommandBuffer(Uint32 frameIndex) const;
        const VkSemaphore& GetImageAvailableSemaphore(Uint32 frameIndex) const;
        const VkSemaphore& GetRenderFinishedSemaphore(Uint32 frameIndex) const;
        const VkFence& GetImageInFlightFence(Uint32 frameIndex) const;
        Bool HasCommandBufferRecorded(Uint32 frameIndex) const;
        const VkCommandBuffer& GetCurrentCommandBuffer() const;
        const VkSemaphore& GetCurrentImageAvailableSemaphore() const;
        const VkSemaphore& GetCurrentRenderFinishedSemaphore() const;
        const VkFence& GetCurrentImageInFlightFence() const;

    private:
        void AssertValidFrameIndex(Uint32 frameIndex) const;

        VkResult CreateSyncObjectsForFrame(VkDevice device, Uint32 frameIndex,
                                           const VkSemaphoreCreateInfo& semaphoreInfo,
                                           const VkFenceCreateInfo& fenceInfo);
        void DestroySyncObjectsForFrame(VkDevice device, Uint32 frameIndex);

        Vector<VkCommandBuffer> commandBuffers;
        Vector<VkSemaphore> imageAvailableSemaphores;
        Vector<VkSemaphore> renderFinishedSemaphores;
        Vector<VkFence> imageInFlightFences;
        Vector<Bool> hasCommandBufferRecorded;
        Uint32 currentFrameIndex = 0;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
