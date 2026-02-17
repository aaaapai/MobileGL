// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkTextureSamplerManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VkTextureSamplerManager.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    Bool VkTextureSamplerManager::Initialize(const InitInfo& initInfo) {
        Shutdown();
        m_device = initInfo.device;
        m_physicalDevice = initInfo.physicalDevice;
        m_commandPool = initInfo.commandPool;
        m_graphicsQueue = initInfo.graphicsQueue;

        if (m_device == VK_NULL_HANDLE || m_physicalDevice == VK_NULL_HANDLE || m_commandPool == VK_NULL_HANDLE ||
            m_graphicsQueue == VK_NULL_HANDLE) {
            MGLOG_E("VkTextureSamplerManager::Initialize failed: invalid init handles");
            Shutdown();
            return false;
        }

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = 1;
        imageInfo.extent.height = 1;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_VERIFY(vkCreateImage(m_device, &imageInfo, nullptr, &m_fallbackImage),
                  "VkTextureSamplerManager::Initialize, vkCreateImage");

        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(m_device, m_fallbackImage, &memoryRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_VERIFY(vkAllocateMemory(m_device, &allocInfo, nullptr, &m_fallbackImageMemory),
                  "VkTextureSamplerManager::Initialize, vkAllocateMemory");
        VK_VERIFY(vkBindImageMemory(m_device, m_fallbackImage, m_fallbackImageMemory, 0),
                  "VkTextureSamplerManager::Initialize, vkBindImageMemory");

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_fallbackImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        VK_VERIFY(vkCreateImageView(m_device, &viewInfo, nullptr, &m_fallbackImageView),
                  "VkTextureSamplerManager::Initialize, vkCreateImageView");

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        VK_VERIFY(vkCreateSampler(m_device, &samplerInfo, nullptr, &m_fallbackSampler),
                  "VkTextureSamplerManager::Initialize, vkCreateSampler");

        if (!UploadFallbackTexture()) {
            Shutdown();
            return false;
        }
        return true;
    }

    void VkTextureSamplerManager::Shutdown() {
        if (m_device != VK_NULL_HANDLE && m_fallbackSampler != VK_NULL_HANDLE) {
            vkDestroySampler(m_device, m_fallbackSampler, nullptr);
        }
        m_fallbackSampler = VK_NULL_HANDLE;

        if (m_device != VK_NULL_HANDLE && m_fallbackImageView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device, m_fallbackImageView, nullptr);
        }
        m_fallbackImageView = VK_NULL_HANDLE;

        if (m_device != VK_NULL_HANDLE && m_fallbackImage != VK_NULL_HANDLE) {
            vkDestroyImage(m_device, m_fallbackImage, nullptr);
        }
        m_fallbackImage = VK_NULL_HANDLE;

        if (m_device != VK_NULL_HANDLE && m_fallbackImageMemory != VK_NULL_HANDLE) {
            vkFreeMemory(m_device, m_fallbackImageMemory, nullptr);
        }
        m_fallbackImageMemory = VK_NULL_HANDLE;

        m_device = VK_NULL_HANDLE;
        m_physicalDevice = VK_NULL_HANDLE;
        m_commandPool = VK_NULL_HANDLE;
        m_graphicsQueue = VK_NULL_HANDLE;
    }

    Bool VkTextureSamplerManager::GetFallbackDescriptor(VkDescriptorImageInfo& outImageInfo) const {
        if (m_fallbackSampler == VK_NULL_HANDLE || m_fallbackImageView == VK_NULL_HANDLE) {
            return false;
        }
        outImageInfo.sampler = m_fallbackSampler;
        outImageInfo.imageView = m_fallbackImageView;
        outImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        return true;
    }

    Uint32 VkTextureSamplerManager::FindMemoryType(Uint32 typeFilter, VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties memoryProperties{};
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);
        for (Uint32 i = 0; i < memoryProperties.memoryTypeCount; ++i) {
            if ((typeFilter & (1U << i)) &&
                (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        MOBILEGL_ASSERT(false, "VkTextureSamplerManager::FindMemoryType failed");
        return 0;
    }

    Bool VkTextureSamplerManager::UploadFallbackTexture() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VK_VERIFY(vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer),
                  "VkTextureSamplerManager::UploadFallbackTexture, vkAllocateCommandBuffers");

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_VERIFY(vkBeginCommandBuffer(commandBuffer, &beginInfo),
                  "VkTextureSamplerManager::UploadFallbackTexture, vkBeginCommandBuffer");

        VkImageMemoryBarrier toTransfer{};
        toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        toTransfer.srcAccessMask = 0;
        toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toTransfer.image = m_fallbackImage;
        toTransfer.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        toTransfer.subresourceRange.baseMipLevel = 0;
        toTransfer.subresourceRange.levelCount = 1;
        toTransfer.subresourceRange.baseArrayLayer = 0;
        toTransfer.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                             nullptr, 0, nullptr, 1, &toTransfer);

        VkClearColorValue clearColor{};
        clearColor.float32[0] = 1.0f;
        clearColor.float32[1] = 1.0f;
        clearColor.float32[2] = 1.0f;
        clearColor.float32[3] = 1.0f;
        VkImageSubresourceRange clearRange{};
        clearRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clearRange.baseMipLevel = 0;
        clearRange.levelCount = 1;
        clearRange.baseArrayLayer = 0;
        clearRange.layerCount = 1;
        vkCmdClearColorImage(commandBuffer, m_fallbackImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1,
                             &clearRange);

        VkImageMemoryBarrier toShaderRead{};
        toShaderRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        toShaderRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        toShaderRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        toShaderRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        toShaderRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        toShaderRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toShaderRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toShaderRead.image = m_fallbackImage;
        toShaderRead.subresourceRange = clearRange;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr, 0, nullptr, 1, &toShaderRead);

        VK_VERIFY(vkEndCommandBuffer(commandBuffer),
                  "VkTextureSamplerManager::UploadFallbackTexture, vkEndCommandBuffer");

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        VK_VERIFY(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE),
                  "VkTextureSamplerManager::UploadFallbackTexture, vkQueueSubmit");
        VK_VERIFY(vkQueueWaitIdle(m_graphicsQueue),
                  "VkTextureSamplerManager::UploadFallbackTexture, vkQueueWaitIdle");

        vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
        return true;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan

