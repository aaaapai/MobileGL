// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/SwapchainManager.h
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

    class SwapchainManager {
    public:
        SwapchainManager(VulkanContext& ctx);
        ~SwapchainManager();

        void Initialize();
        void Recreate();
        void Cleanup();

        VkSwapchainKHR GetSwapchain() const { return Swapchain; }
        VkFormat GetFormat() const { return ImageFormat; }
        VkExtent2D GetExtent() const { return Extent; }
        const std::vector<VkImageView>& GetImageViews() const { return ImageViews; }
        const std::vector<VkFramebuffer>& GetFramebuffers() const { return Framebuffers; }

        void SetFramebuffers(std::vector<VkFramebuffer>&& fbs);

    private:
        VulkanContext& Ctx;
        VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
        VkFormat ImageFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D Extent{0, 0};
        std::vector<VkImage> Images;
        std::vector<VkImageView> ImageViews;
        std::vector<VkFramebuffer> Framebuffers;

        void CreateSwapchainInternal();
        void CreateImageViews();
        void DestroyImageViews();
    };
} // namespace MobileGL::MG_Backend::DirectVulkan