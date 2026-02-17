// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkFramebufferManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VkFramebufferManager.h"

#include <MG_State/GLState/RenderbufferState/RenderbufferObject.h>
#include <MG_State/GLState/TextureState/TextureEnum.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    Bool VkFramebufferManager::Initialize(const InitInfo& initInfo) {
        m_device = initInfo.device;
        m_physicalDevice = initInfo.physicalDevice;
        return m_device != VK_NULL_HANDLE && m_physicalDevice != VK_NULL_HANDLE;
    }

    void VkFramebufferManager::Shutdown() {
        for (auto& [_, target] : m_offscreenColorTargets) {
            DestroyOffscreenColorTarget(target);
        }
        m_offscreenColorTargets.clear();
        m_device = VK_NULL_HANDLE;
        m_physicalDevice = VK_NULL_HANDLE;
    }

    Bool VkFramebufferManager::EnsureOffscreenColorTarget(Uint glFboExternalIndex,
                                                          const MG_State::GLState::FramebufferObject& glFbo) {
        const auto& colorAttachment = glFbo.GetAttachment(FramebufferAttachmentType::Color0);
        if (!colorAttachment.IsValid() || colorAttachment.IsEmpty()) {
            MGLOG_W("VkFramebufferManager: FBO %u has no valid COLOR0 attachment", glFboExternalIndex);
            return false;
        }

        const auto objectVersion = glFbo.GetObjectVersion();
        auto& target = m_offscreenColorTargets[glFboExternalIndex];
        if (target.image != VK_NULL_HANDLE && target.glObjectVersion == objectVersion) {
            return true;
        }

        return RecreateOffscreenColorTarget(target, colorAttachment, objectVersion);
    }

    Bool VkFramebufferManager::ClearColor(VkCommandBuffer commandBuffer, Uint glFboExternalIndex,
                                          const VkClearColorValue& clearColor) {
        auto it = m_offscreenColorTargets.find(glFboExternalIndex);
        if (it == m_offscreenColorTargets.end()) {
            MGLOG_W("VkFramebufferManager::ClearColor skipped: no offscreen target for FBO %u", glFboExternalIndex);
            return false;
        }

        auto& target = it->second;
        if (!TransitionColorTargetForClear(commandBuffer, target)) {
            return false;
        }

        VkImageSubresourceRange subresourceRange{};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;
        vkCmdClearColorImage(commandBuffer, target.image, target.layout, &clearColor, 1, &subresourceRange);
        return true;
    }

    Bool VkFramebufferManager::TransitionOffscreenColorToTransferSrc(VkCommandBuffer commandBuffer,
                                                                     Uint glFboExternalIndex) {
        auto it = m_offscreenColorTargets.find(glFboExternalIndex);
        if (it == m_offscreenColorTargets.end()) {
            MGLOG_W("VkFramebufferManager::TransitionOffscreenColorToTransferSrc skipped: FBO %u not found",
                    glFboExternalIndex);
            return false;
        }
        return TransitionColorTargetForBlitSrc(commandBuffer, it->second);
    }

    Bool VkFramebufferManager::GetOffscreenColorImage(Uint glFboExternalIndex, VkImage& outImage,
                                                      VkExtent2D& outExtent) const {
        auto it = m_offscreenColorTargets.find(glFboExternalIndex);
        if (it == m_offscreenColorTargets.end() || it->second.image == VK_NULL_HANDLE) {
            return false;
        }
        outImage = it->second.image;
        outExtent = it->second.extent;
        return true;
    }

    Bool VkFramebufferManager::RecreateOffscreenColorTarget(
        OffscreenColorTarget& target, const MG_State::GLState::FramebufferAttachmentObject& colorAttachment,
        Uint16 glObjectVersion) {
        DestroyOffscreenColorTarget(target);

        const auto size = colorAttachment.GetSize();
        if (size.x() <= 0 || size.y() <= 0) {
            MGLOG_W("VkFramebufferManager: COLOR0 attachment size is invalid (%d, %d)", size.x(), size.y());
            return false;
        }

        const VkFormat format = ResolveColorFormat(colorAttachment);
        if (format == VK_FORMAT_UNDEFINED) {
            MGLOG_W("VkFramebufferManager: COLOR0 attachment format is unsupported for Vulkan clear");
            return false;
        }

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<Uint32>(size.x());
        imageInfo.extent.height = static_cast<Uint32>(size.y());
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_VERIFY(vkCreateImage(m_device, &imageInfo, nullptr, &target.image), "vkCreateImage(offscreen color)");

        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(m_device, target.image, &memoryRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_VERIFY(vkAllocateMemory(m_device, &allocInfo, nullptr, &target.memory), "vkAllocateMemory(offscreen color)");
        VK_VERIFY(vkBindImageMemory(m_device, target.image, target.memory, 0), "vkBindImageMemory(offscreen color)");

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = target.image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        VK_VERIFY(vkCreateImageView(m_device, &viewInfo, nullptr, &target.imageView), "vkCreateImageView(offscreen color)");

        target.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        target.extent = {static_cast<Uint32>(size.x()), static_cast<Uint32>(size.y())};
        target.format = format;
        target.glObjectVersion = glObjectVersion;
        return true;
    }

    void VkFramebufferManager::DestroyOffscreenColorTarget(OffscreenColorTarget& target) {
        if (target.imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device, target.imageView, nullptr);
            target.imageView = VK_NULL_HANDLE;
        }
        if (target.image != VK_NULL_HANDLE) {
            vkDestroyImage(m_device, target.image, nullptr);
            target.image = VK_NULL_HANDLE;
        }
        if (target.memory != VK_NULL_HANDLE) {
            vkFreeMemory(m_device, target.memory, nullptr);
            target.memory = VK_NULL_HANDLE;
        }
        target.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        target.extent = {0, 0};
        target.format = VK_FORMAT_UNDEFINED;
        target.glObjectVersion = 0;
    }

    Bool VkFramebufferManager::TransitionColorTargetForClear(VkCommandBuffer commandBuffer, OffscreenColorTarget& target) {
        if (target.layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            return true;
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.oldLayout = target.layout;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = target.image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);
        target.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        return true;
    }

    Bool VkFramebufferManager::TransitionColorTargetForBlitSrc(VkCommandBuffer commandBuffer,
                                                               OffscreenColorTarget& target) {
        if (target.layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
            return true;
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.oldLayout = target.layout;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = target.image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);
        target.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        return true;
    }

    Uint32 VkFramebufferManager::FindMemoryType(Uint32 typeFilter, VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties memoryProperties{};
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);
        for (Uint32 i = 0; i < memoryProperties.memoryTypeCount; ++i) {
            if ((typeFilter & (1U << i)) &&
                (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        MOBILEGL_ASSERT(false, "VkFramebufferManager::FindMemoryType failed");
        return 0;
    }

    VkFormat VkFramebufferManager::ResolveColorFormat(
        const MG_State::GLState::FramebufferAttachmentObject& colorAttachment) {
        TextureInternalFormat internalFormat = TextureInternalFormat::Unknown;
        if (colorAttachment.IsTexture()) {
            const auto texture = colorAttachment.GetTexture();
            internalFormat = texture ? texture->GetFormat() : TextureInternalFormat::Unknown;
        } else if (colorAttachment.IsRenderbuffer()) {
            const auto renderbuffer = colorAttachment.GetRenderbuffer();
            internalFormat = renderbuffer ? renderbuffer->GetInternalFormat() : TextureInternalFormat::Unknown;
        }

        switch (internalFormat) {
        case TextureInternalFormat::RGBA:
        case TextureInternalFormat::RGBA8:
        case TextureInternalFormat::SRGB8Alpha8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        default:
            return VK_FORMAT_UNDEFINED;
        }
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
