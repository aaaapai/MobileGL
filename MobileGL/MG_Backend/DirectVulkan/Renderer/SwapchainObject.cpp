// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/SwapchainObject.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "SwapchainObject.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    SwapchainObject::SwapchainCapabilities SwapchainObject::GetSwapchainCapabilities(VkPhysicalDevice physicalDevice,
                                                                                      VkSurfaceKHR surface) {
        SwapchainCapabilities swapchainCapabilities{};

        VK_VERIFY(
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapchainCapabilities.capabilities));

        Uint32 formatCount = 0;
        VK_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr));
        if (formatCount != 0) {
            swapchainCapabilities.surfaceFormats.resize(formatCount);
            VK_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(
                physicalDevice, surface, &formatCount, swapchainCapabilities.surfaceFormats.data()));
        }

        Uint32 presentModeCount = 0;
        VK_VERIFY(
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
        if (presentModeCount != 0) {
            swapchainCapabilities.presentModes.resize(presentModeCount);
            VK_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(
                physicalDevice, surface, &presentModeCount, swapchainCapabilities.presentModes.data()));
        }

        return swapchainCapabilities;
    }

    VkSurfaceFormatKHR SwapchainObject::ChooseSwapchainSurfaceFormat(
        const Vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        // TODO: Properly rank other formats
        return availableFormats[0];
    }

    VkPresentModeKHR SwapchainObject::ChooseSwapchainPresentMode(
        const Vector<VkPresentModeKHR>& availablePresentModes) {
        for (auto desiredPresentMode : s_desiredPresentModes) {
            for (const auto& presentMode : availablePresentModes) {
                if (presentMode == desiredPresentMode) {
                    return presentMode;
                }
            }
        }

        // TODO: Properly rank other modes
        return availablePresentModes[0];
    }

    void SwapchainObject::Create(VkDevice device, const VkSwapchainCreateInfoKHR& createInfo) {
        m_surfaceFormat = {createInfo.imageFormat, createInfo.imageColorSpace};
        m_extent = createInfo.imageExtent;

        VK_VERIFY(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapchain));

        Uint32 imageCount = 0;
        VK_VERIFY(vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, nullptr));

        m_images.resize(imageCount, VK_NULL_HANDLE);
        VK_VERIFY(vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, m_images.data()));
        m_imageLayouts.assign(imageCount, VK_IMAGE_LAYOUT_UNDEFINED);

        CreateImageViews(device);

        MGLOG_I("Swapchain created, extent = %dx%d, swapchain imageCount = %d", m_extent.width, m_extent.height,
                imageCount);
    }

    void SwapchainObject::Shutdown(VkDevice device) {
        for (auto imageView : m_imageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }
        m_imageViews.clear();

        if (m_swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device, m_swapchain, nullptr);
            m_swapchain = VK_NULL_HANDLE;
        }

        m_images.clear();
        m_imageLayouts.clear();
    }

    VkImage SwapchainObject::GetImage(Uint32 index) const {
        MOBILEGL_ASSERT(index < m_images.size(), "Swapchain image index out of range");
        return m_images[index];
    }

    VkImageLayout SwapchainObject::GetImageLayout(Uint32 index) const {
        MOBILEGL_ASSERT(index < m_imageLayouts.size(), "Swapchain image layout index out of range");
        return m_imageLayouts[index];
    }

    void SwapchainObject::SetImageLayout(Uint32 index, VkImageLayout layout) {
        MOBILEGL_ASSERT(index < m_imageLayouts.size(), "Swapchain image layout index out of range");
        m_imageLayouts[index] = layout;
    }

    void SwapchainObject::CreateImageViews(VkDevice device) {
        m_imageViews.resize(m_images.size(), VK_NULL_HANDLE);
        for (SizeT i = 0; i < m_imageViews.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_surfaceFormat.format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            VK_VERIFY(vkCreateImageView(device, &createInfo, nullptr, &m_imageViews[i]));
        }
        MGLOG_I("Swapchain image views created");
    }

#undef VK_VERIFY
} // namespace MobileGL::MG_Backend::DirectVulkan
