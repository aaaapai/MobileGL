// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkTextureManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VkTextureManager.h"

#include "MG_State/GLState/Core.h"
#include "MG_Util/Converters/MGToVk/TextureEnumConverter.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    Bool VkTextureManager::Initialize(const InitInfo& initInfo) {
        Shutdown();

        m_device = initInfo.device;
        m_physicalDevice = initInfo.physicalDevice;
        m_allocator = initInfo.allocator;
        m_commandPool = initInfo.commandPool;
        m_graphicsQueue = initInfo.graphicsQueue;

        MOBILEGL_ASSERT(m_device != VK_NULL_HANDLE && m_physicalDevice != VK_NULL_HANDLE && m_allocator != nullptr &&
                            m_commandPool != VK_NULL_HANDLE && m_graphicsQueue != VK_NULL_HANDLE,
                        "VkTextureManager::Initialize failed: invalid initialization info");

        TextureResource::s_device = m_device;
        TextureResource::s_allocator = m_allocator;

        return true;
    }

    void VkTextureManager::Shutdown() {
        m_textureResources.clear();

        m_device = VK_NULL_HANDLE;
        m_physicalDevice = VK_NULL_HANDLE;
        m_allocator = nullptr;
        m_commandPool = VK_NULL_HANDLE;
        m_graphicsQueue = VK_NULL_HANDLE;
    }

    VkTextureManager::TextureResource* VkTextureManager::SyncTextureAndGetDescriptor(MG_State::GLState::ITextureObject& texture) {
        MOBILEGL_ASSERT(m_device != VK_NULL_HANDLE, "SyncTextureAndGetDescriptor: m_device == VK_NULL_HANDLE");

        auto it = m_textureResources.find(&texture);
        if (it == m_textureResources.end()) {
            TextureResource initial{};
            auto [insertIt, _] = m_textureResources.emplace(&texture, Move(initial));
            it = insertIt;
        }

        if (!SyncTexture(texture, it->second)) {
            MGLOG_D("%s: Syncing texture %d failed", __func__, texture.GetExternalIndex());
            return nullptr;
        }

        return &(it->second);
    }
    Bool VkTextureManager::TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
                                                 VkImageLayout& trackedLayout, VkImageLayout newLayout,
                                                 VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                 VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                                 VkImageAspectFlags aspectMask) {
        MOBILEGL_ASSERT(image != VK_NULL_HANDLE, "TransitionImageLayout: m_image == VK_NULL_HANDLE");

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

    SizeT VkTextureManager::CollectGarbage() {
        m_gcCounter++;
        if (m_gcCounter != 0) {
            return 0;
        }
        SizeT count = 0;
        for (const auto& [raw, weak]: m_aliveObjects) {
            if (weak.expired()) {
                count++;
                m_textureResources.erase(raw);
                m_aliveObjects.erase(raw);
            }
        }
        return count;
    }

    Bool VkTextureManager::SyncTexture(MG_State::GLState::ITextureObject &texture,
                                       TextureResource &outResource) {
        TextureUploadTarget uploadTarget = TextureUploadTarget::Unknown;
        IntVec3 texelSize{0, 0, 0};
        SizeT byteSize = 0;
        Uint32 mipLevelCount = 0;
        if (!CheckMipmapCompleteness(texture, uploadTarget, texelSize, byteSize, mipLevelCount)) {
            MGLOG_D("%s: mipmap not complete", __func__);
            return false;
        }

        auto* mipTexture = dynamic_cast<MG_State::GLState::TextureObjectMipmap*>(&texture);
        if (!mipTexture) {
            MGLOG_D("%s: not TextureObjectMipmap", __func__);
            return false;
        }

        if (!SyncTextureResource(texture, uploadTarget, texelSize, byteSize, mipLevelCount, outResource)) {
            MGLOG_D("%s: SyncTextureResource failed", __func__);
            return false;
        }

        Bool hasDirtyMipLevel = false;
        for (Uint32 level = 0; level < mipLevelCount; ++level) {
            if (mipTexture->IsStorageDirty(uploadTarget, level)) {
                hasDirtyMipLevel = true;
                break;
            }
        }
        if (!hasDirtyMipLevel) {
            return true;
        }

        if (!UploadDirtyMipLevels(*mipTexture, uploadTarget, outResource)) {
            MGLOG_D("%s: UploadDirtyMipLevels failed", __func__);
            return false;
        }
        return true;
    }

    Bool VkTextureManager::SyncTextureResource(const MG_State::GLState::ITextureObject &texture,
                                               TextureUploadTarget uploadTarget,
                                               const IntVec3 &texelSize, SizeT byteSize, Uint32 mipLevels,
                                               TextureResource &resource) {
        const VkFormat format = MG_Util::ConvertTextureInternalFormatToVkEnum(texture.GetFormat());
        if (format == VK_FORMAT_UNDEFINED) {
            MGLOG_D("%s: format == VK_FORMAT_UNDEFINED", __func__);
            return false;
        }
        if (texelSize.x() <= 0 || texelSize.y() <= 0 /*|| byteSize == 0*/) {
            MGLOG_D("%s: texelSize or byteSize is zero", __func__);
            return false;
        }
        if (mipLevels == 0) {
            MGLOG_D("%s: no mip levels", __func__);
            return false;
        }
        if (uploadTarget != TextureUploadTarget::Texture2D) {
            MGLOG_D("%s: not Texture2D, unsupported", __func__);
            return false;
        }

        const Bool compatible = resource.image != VK_NULL_HANDLE && resource.format == format &&
                                resource.extent.width == static_cast<Uint32>(texelSize.x()) &&
                                resource.extent.height == static_cast<Uint32>(texelSize.y()) &&
                                resource.mipLevels == mipLevels;
        if (compatible) {
            return true;
        }

        resource.~TextureResource();

        auto aspect = GetAspectMaskForFormat(format);

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<Uint32>(texelSize.x());
        imageInfo.extent.height = static_cast<Uint32>(texelSize.y());
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage =
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            ((aspect & VK_IMAGE_ASPECT_COLOR_BIT) ?
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0) |
            ((aspect & VK_IMAGE_ASPECT_DEPTH_BIT || aspect & VK_IMAGE_ASPECT_STENCIL_BIT) ?
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : 0);
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        VK_VERIFY(vmaCreateImage(m_allocator, &imageInfo, &allocationInfo, &resource.image, &resource.allocation, nullptr),
                  "vmaCreateImage(texture)");

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = resource.image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspect;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        VK_VERIFY(vkCreateImageView(m_device, &viewInfo, nullptr, &resource.view), "vkCreateImageView(texture)");

        resource.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        resource.extent = {static_cast<Uint32>(texelSize.x()), static_cast<Uint32>(texelSize.y())};
        resource.mipLevels = mipLevels;
        resource.format = format;
        resource.aspect = viewInfo.subresourceRange.aspectMask;
        return true;
    }

    Bool VkTextureManager::UploadDirtyMipLevels(MG_State::GLState::TextureObjectMipmap &mipmapTexture,
                                        TextureUploadTarget uploadTarget,
                                        TextureResource &outResource) {
        struct UploadItem {
            Uint32 level = 0;
            SizeT byteSize = 0;
            IntVec3 texelSize = {0, 0, 0};
            const void* source = nullptr;
            VkDeviceSize offset = 0;
        };

        Vector<UploadItem> uploadItems;
        uploadItems.reserve(outResource.mipLevels);

        VkDeviceSize stagingSize = 0;
        for (Uint32 level = 0; level < outResource.mipLevels; ++level) {
            if (!mipmapTexture.IsStorageDirty(uploadTarget, level)) {
                continue;
            }

            const auto texelSize = mipmapTexture.GetMipmapTexelSize(uploadTarget, level);
            const auto byteSize = mipmapTexture.GetMipmapByteSize(uploadTarget, level);
            if (texelSize.x() <= 0 || texelSize.y() <= 0 || byteSize == 0) {
                mipmapTexture.MarkStorageDirty(uploadTarget, level, false);
                continue;
            }

            const void* source = mipmapTexture.MapMipmapData(uploadTarget, level);
            if (source == nullptr) {
                MGLOG_D("%s: MapmipmapData failed at level %d", __func__, level);
                return false;
            }

            uploadItems.push_back({level, byteSize, texelSize, source, stagingSize});
            stagingSize += static_cast<VkDeviceSize>(byteSize);
        }

        if (uploadItems.empty()) {
            return true;
        }

        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VmaAllocation stagingAllocation = nullptr;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = stagingSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VmaAllocationCreateInfo stagingAllocationInfo{};
        stagingAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        stagingAllocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        stagingAllocationInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VK_VERIFY(vmaCreateBuffer(m_allocator, &bufferInfo, &stagingAllocationInfo, &stagingBuffer, &stagingAllocation, nullptr),
                  "vmaCreateBuffer(staging texture)");

        void* mapped = nullptr;
        VK_VERIFY(vmaMapMemory(m_allocator, stagingAllocation, &mapped), "vmaMapMemory(staging texture)");
        for (const auto& item : uploadItems) {
            std::memcpy(static_cast<Uint8*>(mapped) + item.offset, item.source, item.byteSize);
        }
        vmaUnmapMemory(m_allocator, stagingAllocation);

        // TODO: Could be uploaded asynchronously?
        const Bool ok = ExecuteCmdBufImmediate([&](VkCommandBuffer commandBuffer) {
            const VkImageAspectFlags aspectMask = GetAspectMaskForFormat(outResource.format);
            Bool ok = TransitionImageLayout(commandBuffer, outResource.image,
                                  outResource.layout,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ? VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  outResource.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ? VK_ACCESS_SHADER_READ_BIT : 0,
                                  VK_ACCESS_TRANSFER_WRITE_BIT,
                                  aspectMask);
            MOBILEGL_ASSERT(ok, "TransitionImageLayout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL failed");

            for (const auto& item : uploadItems) {
                VkBufferImageCopy copy{};
                copy.bufferOffset = item.offset;
                copy.bufferRowLength = 0;
                copy.bufferImageHeight = 0;
                copy.imageSubresource.aspectMask = aspectMask;
                copy.imageSubresource.mipLevel = item.level;
                copy.imageSubresource.baseArrayLayer = 0;
                copy.imageSubresource.layerCount = 1;
                copy.imageOffset = {0, 0, 0};
                copy.imageExtent = {static_cast<Uint32>(item.texelSize.x()), static_cast<Uint32>(item.texelSize.y()), 1};
                vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, outResource.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       1, &copy);
            }

            ok = TransitionImageLayout(commandBuffer, outResource.image,
                                            outResource.layout,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                            VK_ACCESS_TRANSFER_WRITE_BIT,
                                            VK_ACCESS_TRANSFER_READ_BIT,
                                            aspectMask);
            MOBILEGL_ASSERT(ok, "TransitionImageLayout to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL failed");
            outResource.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            return ok;
        });

        vmaDestroyBuffer(m_allocator, stagingBuffer, stagingAllocation);

        if (!ok) {
            MGLOG_D("%s: texture upload cmd failed", __func__);
            return false;
        }
        for (const auto& item : uploadItems) {
            mipmapTexture.MarkStorageDirty(uploadTarget, item.level, false);
        }
        outResource.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        return true;
    }

    Bool VkTextureManager::ExecuteCmdBufImmediate(const std::function<void(VkCommandBuffer)>& recorder) const {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VK_VERIFY(vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer), "vkAllocateCommandBuffers(texture)");

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_VERIFY(vkBeginCommandBuffer(commandBuffer, &beginInfo), "vkBeginCommandBuffer(texture)");

        recorder(commandBuffer);

        VK_VERIFY(vkEndCommandBuffer(commandBuffer), "vkEndCommandBuffer(texture)");

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        VK_VERIFY(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE), "vkQueueSubmit(texture)");
        VK_VERIFY(vkQueueWaitIdle(m_graphicsQueue), "vkQueueWaitIdle(texture)");

        vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
        return true;
    }

    Bool VkTextureManager::CheckMipmapCompleteness(const MG_State::GLState::ITextureObject& texture,
                                                   TextureUploadTarget& outTarget,
                                                   IntVec3& outTexelSize,
                                                   SizeT& outByteSize,
                                                   Uint32& outMipLevelCount) {
        const auto* mipTexture = dynamic_cast<const MG_State::GLState::TextureObjectMipmap*>(&texture);
        if (!mipTexture) {
            MGLOG_D("%s: not TextureObjectMipmap", __func__);
            return false;
        }
        const auto& targets = texture.GetUploadTargets();
        if (targets.empty()) {
            MGLOG_D("%s: upload target empty", __func__);
            return false;
        }

        for (const auto target : targets) {
            const Uint32 mipLevelCount = GetUploadMipLevelCount(*mipTexture, target);
            if (mipLevelCount == 0) {
                MGLOG_D("%s: mipLevelCount == 0", __func__);
                continue;
            }

            const auto level0TexelSize = mipTexture->GetMipmapTexelSize(target, 0);
            const auto level0ByteSize = mipTexture->GetMipmapByteSize(target, 0);
            if (level0TexelSize.x() <= 0 || level0TexelSize.y() <= 0 /*|| level0ByteSize == 0*/) {
                continue;
            }

            outTarget = target;
            outTexelSize = level0TexelSize;
            outByteSize = level0ByteSize;
            outMipLevelCount = mipLevelCount;
            return true;
        }
        MGLOG_D("%s: no valid target or mipmap", __func__);
        return false;
    }

    Uint32 VkTextureManager::GetUploadMipLevelCount(const MG_State::GLState::TextureObjectMipmap& texture,
                                                    TextureUploadTarget target) {
        const Uint totalLevelCount = texture.GetMipmapLevelCount();
        if (totalLevelCount == 0) {
            return 0;
        }

        Uint32 validLevelCount = 0;
        for (Uint level = 0; level < totalLevelCount; ++level) {
            const auto size = texture.GetMipmapTexelSize(target, level);
            const auto byteSize = texture.GetMipmapByteSize(target, level);
            if (size.x() <= 0 || size.y() <= 0 /*|| byteSize == 0*/) {
                break;
            }
            ++validLevelCount;
        }
        return validLevelCount;
    }

    VkImageAspectFlags VkTextureManager::GetAspectMaskForFormat(VkFormat format) {
        switch (format) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
