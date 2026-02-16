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
        struct FrameData {
            VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
            VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
            VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
            VkFence imageInFlightFence = VK_NULL_HANDLE;
            Bool hasCommandBufferRecorded = false;
        };

        VkResult Initialize(VkDevice device, VkCommandPool commandPool, Uint32 frameCount);
        void Destroy(VkDevice device, VkCommandPool commandPool);
        FrameData& GetCurrent();
        const FrameData& GetCurrent() const;
        void AdvanceToNext();

        Uint32 GetCurrentFrameIndex() const;
        Uint32 GetFrameCount() const;

    private:
        void AssertValidFrameIndex(Uint32 frameIndex) const;

        VkResult CreateSyncObjectsForFrame(VkDevice device, Uint32 frameIndex,
                                           const VkSemaphoreCreateInfo& semaphoreInfo,
                                           const VkFenceCreateInfo& fenceInfo);
        void DestroySyncObjectsForFrame(VkDevice device, Uint32 frameIndex);

        Vector<FrameData> m_frames;
        Uint32 currentFrameIndex = 0;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
