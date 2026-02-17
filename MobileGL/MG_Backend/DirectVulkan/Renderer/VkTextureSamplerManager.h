// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkTextureSamplerManager.h
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
    class VkTextureSamplerManager {
    public:
        struct InitInfo {
            VkDevice device = VK_NULL_HANDLE;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkCommandPool commandPool = VK_NULL_HANDLE;
            VkQueue graphicsQueue = VK_NULL_HANDLE;
        };

        Bool Initialize(const InitInfo& initInfo);
        void Shutdown();

        Bool GetFallbackDescriptor(VkDescriptorImageInfo& outImageInfo) const;

    private:
        Uint32 FindMemoryType(Uint32 typeFilter, VkMemoryPropertyFlags properties) const;
        Bool UploadFallbackTexture();

        VkDevice m_device = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkCommandPool m_commandPool = VK_NULL_HANDLE;
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;

        VkImage m_fallbackImage = VK_NULL_HANDLE;
        VkDeviceMemory m_fallbackImageMemory = VK_NULL_HANDLE;
        VkImageView m_fallbackImageView = VK_NULL_HANDLE;
        VkSampler m_fallbackSampler = VK_NULL_HANDLE;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan

