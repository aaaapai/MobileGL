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

        return RecreateOffscreenColorTarget(target, glFbo, colorAttachment, objectVersion);
    }

    Bool VkFramebufferManager::TransitionOffscreenColorToAttachment(VkCommandBuffer commandBuffer,
                                                                    Uint glFboExternalIndex) {
        auto it = m_offscreenColorTargets.find(glFboExternalIndex);
        if (it == m_offscreenColorTargets.end()) {
            MGLOG_W("VkFramebufferManager::TransitionOffscreenColorToAttachment skipped: FBO %u not found",
                    glFboExternalIndex);
            return false;
        }
        return TransitionColorTargetForAttachment(commandBuffer, it->second) &&
               TransitionDepthStencilTargetForAttachment(commandBuffer, it->second);
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

    Bool VkFramebufferManager::GetOffscreenRenderTarget(Uint glFboExternalIndex, VkRenderPass& outRenderPass,
                                                        VkFramebuffer& outFramebuffer, VkExtent2D& outExtent,
                                                        VkFormat& outDepthStencilFormat) const {
        auto it = m_offscreenColorTargets.find(glFboExternalIndex);
        if (it == m_offscreenColorTargets.end() || it->second.framebuffer == VK_NULL_HANDLE ||
            it->second.renderPassLoad == VK_NULL_HANDLE) {
            return false;
        }
        outRenderPass = it->second.renderPassLoad;
        outFramebuffer = it->second.framebuffer;
        outExtent = it->second.extent;
        outDepthStencilFormat = it->second.depthStencilFormat;
        return true;
    }

    Bool VkFramebufferManager::RecreateOffscreenColorTarget(
        OffscreenColorTarget& target, const MG_State::GLState::FramebufferObject& glFbo,
        const MG_State::GLState::FramebufferAttachmentObject& colorAttachment,
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

        const auto& depthAttachment = glFbo.GetAttachment(FramebufferAttachmentType::Depth);
        const auto& stencilAttachment = glFbo.GetAttachment(FramebufferAttachmentType::Stencil);
        const Bool requestedDepthStencil =
            (depthAttachment.IsValid() && !depthAttachment.IsEmpty()) ||
            (stencilAttachment.IsValid() && !stencilAttachment.IsEmpty());
        VkFormat depthStencilFormat = ResolveDepthStencilFormat(depthAttachment, stencilAttachment);
        if (depthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT) {
            depthStencilFormat = FindSupportedDepthStencilFormat({VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT});
        } else if (depthStencilFormat == VK_FORMAT_D32_SFLOAT) {
            depthStencilFormat = FindSupportedDepthStencilFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM});
        }
        const Bool hasDepthStencil = (depthStencilFormat != VK_FORMAT_UNDEFINED);
        if (requestedDepthStencil && !hasDepthStencil) {
            MGLOG_W("VkFramebufferManager: FBO %u depth/stencil attachment exists but format is unsupported",
                    glFbo.GetExternalIndex());
        }
        if (hasDepthStencil) {
            VkImageCreateInfo depthImageInfo{};
            depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
            depthImageInfo.extent.width = static_cast<Uint32>(size.x());
            depthImageInfo.extent.height = static_cast<Uint32>(size.y());
            depthImageInfo.extent.depth = 1;
            depthImageInfo.mipLevels = 1;
            depthImageInfo.arrayLayers = 1;
            depthImageInfo.format = depthStencilFormat;
            depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            VK_VERIFY(vkCreateImage(m_device, &depthImageInfo, nullptr, &target.depthStencilImage),
                      "vkCreateImage(offscreen depth/stencil)");

            VkMemoryRequirements depthMemoryRequirements{};
            vkGetImageMemoryRequirements(m_device, target.depthStencilImage, &depthMemoryRequirements);
            VkMemoryAllocateInfo depthAllocInfo{};
            depthAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            depthAllocInfo.allocationSize = depthMemoryRequirements.size;
            depthAllocInfo.memoryTypeIndex =
                FindMemoryType(depthMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VK_VERIFY(vkAllocateMemory(m_device, &depthAllocInfo, nullptr, &target.depthStencilMemory),
                      "vkAllocateMemory(offscreen depth/stencil)");
            VK_VERIFY(vkBindImageMemory(m_device, target.depthStencilImage, target.depthStencilMemory, 0),
                      "vkBindImageMemory(offscreen depth/stencil)");

            VkImageAspectFlags depthAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (depthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT || depthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT) {
                depthAspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            VkImageViewCreateInfo depthViewInfo{};
            depthViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            depthViewInfo.image = target.depthStencilImage;
            depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            depthViewInfo.format = depthStencilFormat;
            depthViewInfo.subresourceRange.aspectMask = depthAspectMask;
            depthViewInfo.subresourceRange.baseMipLevel = 0;
            depthViewInfo.subresourceRange.levelCount = 1;
            depthViewInfo.subresourceRange.baseArrayLayer = 0;
            depthViewInfo.subresourceRange.layerCount = 1;
            VK_VERIFY(vkCreateImageView(m_device, &depthViewInfo, nullptr, &target.depthStencilImageView),
                      "vkCreateImageView(offscreen depth/stencil)");
        }

        VkAttachmentDescription colorAttachmentDesc{};
        colorAttachmentDesc.format = format;
        colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkAttachmentDescription depthAttachmentDesc{};
        VkAttachmentReference depthAttachmentRef{};
        Array<VkAttachmentDescription, 2> attachments{};
        attachments[0] = colorAttachmentDesc;
        Uint32 attachmentCount = 1;
        if (hasDepthStencil) {
            depthAttachmentDesc.format = depthStencilFormat;
            depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            depthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            depthAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            depthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
            depthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            subpass.pDepthStencilAttachment = &depthAttachmentRef;

            attachments[1] = depthAttachmentDesc;
            attachmentCount = 2;
        }

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = attachmentCount;
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        VK_VERIFY(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &target.renderPassLoad),
                  "vkCreateRenderPass(offscreen)");

        Array<VkImageView, 2> framebufferAttachments{};
        framebufferAttachments[0] = target.imageView;
        Uint32 framebufferAttachmentCount = 1;
        if (hasDepthStencil) {
            framebufferAttachments[1] = target.depthStencilImageView;
            framebufferAttachmentCount = 2;
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = target.renderPassLoad;
        framebufferInfo.attachmentCount = framebufferAttachmentCount;
        framebufferInfo.pAttachments = framebufferAttachments.data();
        framebufferInfo.width = static_cast<Uint32>(size.x());
        framebufferInfo.height = static_cast<Uint32>(size.y());
        framebufferInfo.layers = 1;
        VK_VERIFY(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &target.framebuffer),
                  "vkCreateFramebuffer(offscreen)");

        target.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        target.extent = {static_cast<Uint32>(size.x()), static_cast<Uint32>(size.y())};
        target.format = format;
        target.depthStencilLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        target.depthStencilFormat = depthStencilFormat;
        target.glObjectVersion = glObjectVersion;
        return true;
    }

    void VkFramebufferManager::DestroyOffscreenColorTarget(OffscreenColorTarget& target) {
        if (target.framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(m_device, target.framebuffer, nullptr);
            target.framebuffer = VK_NULL_HANDLE;
        }
        if (target.renderPassLoad != VK_NULL_HANDLE) {
            vkDestroyRenderPass(m_device, target.renderPassLoad, nullptr);
            target.renderPassLoad = VK_NULL_HANDLE;
        }
        if (target.imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device, target.imageView, nullptr);
            target.imageView = VK_NULL_HANDLE;
        }
        if (target.depthStencilImageView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device, target.depthStencilImageView, nullptr);
            target.depthStencilImageView = VK_NULL_HANDLE;
        }
        if (target.image != VK_NULL_HANDLE) {
            vkDestroyImage(m_device, target.image, nullptr);
            target.image = VK_NULL_HANDLE;
        }
        if (target.depthStencilImage != VK_NULL_HANDLE) {
            vkDestroyImage(m_device, target.depthStencilImage, nullptr);
            target.depthStencilImage = VK_NULL_HANDLE;
        }
        if (target.memory != VK_NULL_HANDLE) {
            vkFreeMemory(m_device, target.memory, nullptr);
            target.memory = VK_NULL_HANDLE;
        }
        if (target.depthStencilMemory != VK_NULL_HANDLE) {
            vkFreeMemory(m_device, target.depthStencilMemory, nullptr);
            target.depthStencilMemory = VK_NULL_HANDLE;
        }
        target.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        target.depthStencilLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        target.extent = {0, 0};
        target.format = VK_FORMAT_UNDEFINED;
        target.depthStencilFormat = VK_FORMAT_UNDEFINED;
        target.glObjectVersion = 0;
    }

    Bool VkFramebufferManager::TransitionColorTargetForAttachment(VkCommandBuffer commandBuffer,
                                                                  OffscreenColorTarget& target) {
        if (target.layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            return true;
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.oldLayout = target.layout;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = target.image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);
        target.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        return true;
    }

    Bool VkFramebufferManager::TransitionDepthStencilTargetForAttachment(VkCommandBuffer commandBuffer,
                                                                         OffscreenColorTarget& target) {
        if (target.depthStencilImage == VK_NULL_HANDLE) {
            return true;
        }
        if (target.depthStencilLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            return true;
        }

        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (target.depthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
            target.depthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT) {
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.oldLayout = target.depthStencilLayout;
        barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = target.depthStencilImage;
        barrier.subresourceRange.aspectMask = aspectMask;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);
        target.depthStencilLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
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

    VkFormat VkFramebufferManager::ResolveDepthStencilFormat(
        const MG_State::GLState::FramebufferAttachmentObject& depthAttachment,
        const MG_State::GLState::FramebufferAttachmentObject& stencilAttachment) {
        const auto resolveAttachmentFormat = [](const MG_State::GLState::FramebufferAttachmentObject& attachment) {
            TextureInternalFormat internalFormat = TextureInternalFormat::Unknown;
            if (attachment.IsTexture()) {
                const auto texture = attachment.GetTexture();
                internalFormat = texture ? texture->GetFormat() : TextureInternalFormat::Unknown;
            } else if (attachment.IsRenderbuffer()) {
                const auto renderbuffer = attachment.GetRenderbuffer();
                internalFormat = renderbuffer ? renderbuffer->GetInternalFormat() : TextureInternalFormat::Unknown;
            }
            return internalFormat;
        };

        const auto depthFormat = resolveAttachmentFormat(depthAttachment);
        const auto stencilFormat = resolveAttachmentFormat(stencilAttachment);

        switch (depthFormat) {
        case TextureInternalFormat::Depth24Stencil8:
        case TextureInternalFormat::Depth32FStencil8:
        case TextureInternalFormat::DepthStencil:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case TextureInternalFormat::DepthComponent16:
            return VK_FORMAT_D16_UNORM;
        case TextureInternalFormat::DepthComponent24:
        case TextureInternalFormat::DepthComponent32:
        case TextureInternalFormat::DepthComponent32F:
        case TextureInternalFormat::DepthComponent:
            return VK_FORMAT_D32_SFLOAT;
        default:
            break;
        }

        switch (stencilFormat) {
        case TextureInternalFormat::Depth24Stencil8:
        case TextureInternalFormat::Depth32FStencil8:
        case TextureInternalFormat::DepthStencil:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        default:
            return VK_FORMAT_UNDEFINED;
        }
    }

    VkFormat VkFramebufferManager::FindSupportedDepthStencilFormat(const Vector<VkFormat>& candidates) const {
        for (auto format : candidates) {
            VkFormatProperties properties{};
            vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &properties);
            if ((properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
                return format;
            }
        }
        return VK_FORMAT_UNDEFINED;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
