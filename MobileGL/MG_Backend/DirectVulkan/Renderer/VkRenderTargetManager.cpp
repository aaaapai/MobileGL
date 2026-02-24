// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkFramebufferManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VkRenderTargetManager.h"

#include <MG_State/GLState/RenderbufferState/RenderbufferObject.h>
#include <MG_State/GLState/TextureState/TextureEnum.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    Bool VkRenderTargetManager::Initialize(const InitInfo& initInfo) {
        m_device = initInfo.device;
        m_physicalDevice = initInfo.physicalDevice;
        return m_device != VK_NULL_HANDLE && m_physicalDevice != VK_NULL_HANDLE;
    }

    void VkRenderTargetManager::Shutdown() {
        for (auto& [_, target] : m_offscreenColorTargets) {
            DestroyOffscreenColorTarget(target);
        }
        m_offscreenColorTargets.clear();
        m_device = VK_NULL_HANDLE;
        m_physicalDevice = VK_NULL_HANDLE;
    }

    Bool VkRenderTargetManager::EnsureOffscreenColorTarget(Uint glFboExternalIndex,
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

    Bool VkRenderTargetManager::Transition(VkCommandBuffer commandBuffer, TransitionResource resource,
                                          TransitionUsage usage, Uint externalIndex) {
        auto resolveDepthStencilAspectMask = [](VkFormat format) {
            VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
                aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            return aspectMask;
        };

        if (resource == TransitionResource::OffscreenColorTexture) {
            if (usage != TransitionUsage::ShaderRead) {
                MGLOG_W("VkFramebufferManager::Transition skipped: unsupported usage %d for texture %u",
                        static_cast<Int>(usage), externalIndex);
                return false;
            }

            for (auto& [_, target] : m_offscreenColorTargets) {
                if (target.colorTextureExternalIndex != externalIndex || target.image == VK_NULL_HANDLE) {
                    continue;
                }
                const Bool fromUndefined = (target.layout == VK_IMAGE_LAYOUT_UNDEFINED);
                return TransitionImageLayout(
                    commandBuffer, target.image, target.layout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    fromUndefined
                        ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
                        : (VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT),
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    fromUndefined ? 0 : (VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT),
                    VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
            }
            return false;
        }

        auto it = m_offscreenColorTargets.find(externalIndex);
        if (it == m_offscreenColorTargets.end()) {
            MGLOG_W("VkFramebufferManager::Transition skipped: FBO %u not found", externalIndex);
            return false;
        }
        auto& target = it->second;

        if (resource == TransitionResource::OffscreenColor) {
            switch (usage) {
            case TransitionUsage::Attachment: {
                if (!TransitionImageLayout(
                        commandBuffer, target.image, target.layout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
                        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                        VK_IMAGE_ASPECT_COLOR_BIT)) {
                    return false;
                }
                if (target.depthStencilImage == VK_NULL_HANDLE) {
                    return true;
                }
                const VkImageAspectFlags aspectMask = resolveDepthStencilAspectMask(target.depthStencilFormat);
                return TransitionImageLayout(
                    commandBuffer, target.depthStencilImage, target.depthStencilLayout,
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, 0,
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    aspectMask);
            }
            case TransitionUsage::TransferSrc:
                return TransitionImageLayout(commandBuffer, target.image, target.layout,
                                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                             VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
            case TransitionUsage::TransferDst:
                return TransitionImageLayout(commandBuffer, target.image, target.layout,
                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                             VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
            case TransitionUsage::General:
                return TransitionImageLayout(commandBuffer, target.image, target.layout, VK_IMAGE_LAYOUT_GENERAL,
                                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                             VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT,
                                             VK_IMAGE_ASPECT_COLOR_BIT);
            case TransitionUsage::ShaderRead:
                return TransitionImageLayout(commandBuffer, target.image, target.layout,
                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                             0, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
            default:
                return false;
            }
        }

        if (resource == TransitionResource::OffscreenDepthStencil) {
            if (target.depthStencilImage == VK_NULL_HANDLE) {
                return false;
            }
            const VkImageAspectFlags aspectMask = resolveDepthStencilAspectMask(target.depthStencilFormat);
            switch (usage) {
            case TransitionUsage::TransferSrc:
                return TransitionImageLayout(commandBuffer, target.depthStencilImage, target.depthStencilLayout,
                                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                             VK_ACCESS_TRANSFER_READ_BIT, aspectMask);
            case TransitionUsage::TransferDst:
                return TransitionImageLayout(commandBuffer, target.depthStencilImage, target.depthStencilLayout,
                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                             VK_ACCESS_TRANSFER_WRITE_BIT, aspectMask);
            case TransitionUsage::General:
                return TransitionImageLayout(commandBuffer, target.depthStencilImage, target.depthStencilLayout,
                                             VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                             VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT, aspectMask);
            case TransitionUsage::Attachment:
                return TransitionImageLayout(
                    commandBuffer, target.depthStencilImage, target.depthStencilLayout,
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, 0,
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    aspectMask);
            case TransitionUsage::ShaderRead:
                return false;
            default:
                return false;
            }
        }

        return false;
    }

    Bool VkRenderTargetManager::GetOffscreenColorImage(Uint glFboExternalIndex, VkImage& outImage,
                                                      VkExtent2D& outExtent) const {
        auto it = m_offscreenColorTargets.find(glFboExternalIndex);
        if (it == m_offscreenColorTargets.end() || it->second.image == VK_NULL_HANDLE) {
            return false;
        }
        outImage = it->second.image;
        outExtent = it->second.extent;
        return true;
    }

    Bool VkRenderTargetManager::GetOffscreenDepthStencilImage(Uint glFboExternalIndex, VkImage& outImage,
                                                             VkExtent2D& outExtent, VkFormat& outFormat) const {
        auto it = m_offscreenColorTargets.find(glFboExternalIndex);
        if (it == m_offscreenColorTargets.end() || it->second.depthStencilImage == VK_NULL_HANDLE) {
            return false;
        }
        outImage = it->second.depthStencilImage;
        outExtent = it->second.extent;
        outFormat = it->second.depthStencilFormat;
        return true;
    }

    Bool VkRenderTargetManager::GetOffscreenColorViewByTexture(Uint textureExternalIndex,
                                                              VkImageView& outImageView) const {
        for (const auto& [_, target] : m_offscreenColorTargets) {
            if (target.colorTextureExternalIndex != textureExternalIndex || target.imageView == VK_NULL_HANDLE) {
                continue;
            }
            outImageView = target.imageView;
            return true;
        }
        return false;
    }

    Bool VkRenderTargetManager::GetOffscreenRenderSurface(Uint glFboExternalIndex, VkImageView& outColorView,
                                                         VkFormat& outColorFormat, VkImageView& outDepthStencilView,
                                                         VkFormat& outDepthStencilFormat, VkExtent2D& outExtent) const {
        auto it = m_offscreenColorTargets.find(glFboExternalIndex);
        if (it == m_offscreenColorTargets.end() || it->second.imageView == VK_NULL_HANDLE) {
            return false;
        }
        outColorView = it->second.imageView;
        outColorFormat = it->second.format;
        outDepthStencilView = it->second.depthStencilImageView;
        outExtent = it->second.extent;
        outDepthStencilFormat = it->second.depthStencilFormat;
        return true;
    }

    Bool VkRenderTargetManager::RecreateOffscreenColorTarget(
        OffscreenColorTarget& target, const MG_State::GLState::FramebufferObject& glFbo,
        const MG_State::GLState::FramebufferAttachmentObject& colorAttachment, Uint16 glObjectVersion) {
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
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                          VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_VERIFY(vkCreateImage(m_device, &imageInfo, nullptr, &target.image), "vkCreateImage(offscreen color)");

        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(m_device, target.image, &memoryRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex =
            FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
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
        VK_VERIFY(vkCreateImageView(m_device, &viewInfo, nullptr, &target.imageView),
                  "vkCreateImageView(offscreen color)");

        const auto& depthAttachment = glFbo.GetAttachment(FramebufferAttachmentType::Depth);
        const auto& stencilAttachment = glFbo.GetAttachment(FramebufferAttachmentType::Stencil);
        const Bool requestedDepthStencil = (depthAttachment.IsValid() && !depthAttachment.IsEmpty()) ||
                                           (stencilAttachment.IsValid() && !stencilAttachment.IsEmpty());
        VkFormat depthStencilFormat = ResolveDepthStencilFormat(depthAttachment, stencilAttachment);
        if (depthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT) {
            depthStencilFormat =
                FindSupportedDepthStencilFormat({VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT});
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
            if (depthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
                depthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT) {
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

        target.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        target.extent = {static_cast<Uint32>(size.x()), static_cast<Uint32>(size.y())};
        target.format = format;
        target.depthStencilLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        target.depthStencilFormat = depthStencilFormat;
        target.glObjectVersion = glObjectVersion;
        target.colorTextureExternalIndex = (colorAttachment.IsTexture() && colorAttachment.GetTexture())
                                               ? colorAttachment.GetTexture()->GetExternalIndex()
                                               : 0;
        return true;
    }

    void VkRenderTargetManager::DestroyOffscreenColorTarget(OffscreenColorTarget& target) {
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
        target.colorTextureExternalIndex = 0;
    }

    Bool VkRenderTargetManager::TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
                                                     VkImageLayout& trackedLayout, VkImageLayout newLayout,
                                                     VkPipelineStageFlags srcStageMask,
                                                     VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask,
                                                     VkAccessFlags dstAccessMask, VkImageAspectFlags aspectMask) {
        if (image == VK_NULL_HANDLE) {
            return false;
        }
        if (trackedLayout == newLayout) {
            return true;
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = srcAccessMask;
        barrier.dstAccessMask = dstAccessMask;
        barrier.oldLayout = trackedLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = aspectMask;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        trackedLayout = newLayout;
        return true;
    }

    Uint32 VkRenderTargetManager::FindMemoryType(Uint32 typeFilter, VkMemoryPropertyFlags properties) const {
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

    VkFormat VkRenderTargetManager::ResolveColorFormat(
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

    VkFormat VkRenderTargetManager::ResolveDepthStencilFormat(
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

    VkFormat VkRenderTargetManager::FindSupportedDepthStencilFormat(const Vector<VkFormat>& candidates) const {
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
