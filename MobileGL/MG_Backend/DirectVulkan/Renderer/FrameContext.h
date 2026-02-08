// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/FrameContext.h
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

    struct FrameContext {
        FrameContext() = default;
        ~FrameContext();

        void Initialize(VulkanContext& ctx, VkCommandPool pool);
        void Cleanup(VulkanContext& ctx);

        VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
        VkSemaphore ImageAvailable = VK_NULL_HANDLE;
        VkSemaphore RenderFinished = VK_NULL_HANDLE;
        VkFence InFlightFence = VK_NULL_HANDLE;
        Uint32 CurrentImageIndex = 0;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan