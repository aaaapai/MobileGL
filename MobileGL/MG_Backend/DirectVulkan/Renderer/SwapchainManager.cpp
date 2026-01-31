// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/SwapchainManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "SwapchainManager.h"
#include "VulkanContext.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    SwapchainManager::SwapchainManager(VulkanContext& ctx) : Ctx(ctx) {}
    SwapchainManager::~SwapchainManager() {
        Cleanup();
    }

    void SwapchainManager::Initialize() {
        CreateSwapchainInternal();
        CreateImageViews();
    }

    void SwapchainManager::Recreate() {
        DestroyImageViews();
        if (Swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(Ctx.GetDevice(), Swapchain, nullptr);
            Swapchain = VK_NULL_HANDLE;
        }
        CreateSwapchainInternal();
        CreateImageViews();
        Framebuffers.clear();
        MGLOG_D("Swapchain recreated");
    }

    void SwapchainManager::Cleanup() {
        for (auto fb : Framebuffers)
            if (fb != VK_NULL_HANDLE) vkDestroyFramebuffer(Ctx.GetDevice(), fb, nullptr);
        Framebuffers.clear();
        DestroyImageViews();
        if (Swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(Ctx.GetDevice(), Swapchain, nullptr);
            Swapchain = VK_NULL_HANDLE;
        }
    }

    void SwapchainManager::SetFramebuffers(std::vector<VkFramebuffer>&& fbs) {
        Framebuffers = std::move(fbs);
    }

    void SwapchainManager::CreateSwapchainInternal() {
        VkSurfaceCapabilitiesKHR caps;
        ThrowIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Ctx.GetPhysicalDevice(), Ctx.GetSurface(), &caps),
                      "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

        Extent = caps.currentExtent;
        ImageFormat = VK_FORMAT_R8G8B8A8_UNORM;

        VkSwapchainCreateInfoKHR sci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        sci.surface = Ctx.GetSurface();
        sci.minImageCount = std::max<uint32_t>(2, caps.minImageCount);
        sci.imageFormat = ImageFormat;
        sci.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        sci.imageExtent = Extent;
        sci.imageArrayLayers = 1;
        sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        sci.preTransform = caps.currentTransform;
        sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        sci.presentMode = VK_PRESENT_MODE_FIFO_KHR;

        ThrowIfFailed(vkCreateSwapchainKHR(Ctx.GetDevice(), &sci, nullptr, &Swapchain), "vkCreateSwapchainKHR");

        uint32_t count = 0;
        ThrowIfFailed(vkGetSwapchainImagesKHR(Ctx.GetDevice(), Swapchain, &count, nullptr),
                      "vkGetSwapchainImagesKHR count");
        Images.resize(count);
        ThrowIfFailed(vkGetSwapchainImagesKHR(Ctx.GetDevice(), Swapchain, &count, Images.data()),
                      "vkGetSwapchainImagesKHR images");
        MGLOG_D("Swapchain created (%u images)", count);
    }

    void SwapchainManager::CreateImageViews() {
        DestroyImageViews();
        ImageViews.resize(Images.size());
        for (size_t i = 0; i < Images.size(); ++i) {
            VkImageViewCreateInfo ivci{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            ivci.image = Images[i];
            ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            ivci.format = ImageFormat;
            ivci.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
                               VK_COMPONENT_SWIZZLE_A};
            ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            ivci.subresourceRange.baseMipLevel = 0;
            ivci.subresourceRange.levelCount = 1;
            ivci.subresourceRange.baseArrayLayer = 0;
            ivci.subresourceRange.layerCount = 1;
            ThrowIfFailed(vkCreateImageView(Ctx.GetDevice(), &ivci, nullptr, &ImageViews[i]), "vkCreateImageView");
        }
        MGLOG_D("ImageViews created");
    }

    void SwapchainManager::DestroyImageViews() {
        for (auto iv : ImageViews)
            if (iv != VK_NULL_HANDLE) vkDestroyImageView(Ctx.GetDevice(), iv, nullptr);
        ImageViews.clear();
    }
} // namespace MobileGL::MG_Backend::DirectVulkan