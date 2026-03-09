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
    static constexpr VkPipelineStageFlags kGraphicsSampledReadStages = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;

    static Bool IsValidSampledImageLayout(VkImageLayout layout) {
        switch (layout) {
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_GENERAL:
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            return true;
        default:
            return false;
        }
    }

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

    VkImageView VkTextureManager::GetOrCreateViewAtMipLevel(MG_State::GLState::ITextureObject& texture, Uint32 mipLevel) {
        TextureResource* resource = SyncTextureAndGetDescriptor(texture);
        if (resource == nullptr || resource->image == VK_NULL_HANDLE || mipLevel >= resource->mipLevels) {
            return VK_NULL_HANDLE;
        }

        if (resource->perMipViews.size() != resource->mipLevels) {
            resource->perMipViews.resize(resource->mipLevels, VK_NULL_HANDLE);
        }

        VkImageView& perMipView = resource->perMipViews[mipLevel];
        if (perMipView != VK_NULL_HANDLE) {
            return perMipView;
        }

        perMipView = CreateImageView(resource->image, resource->format, resource->aspect, mipLevel, 1);
        if (perMipView == VK_NULL_HANDLE) {
            MGLOG_D("%s: CreateImageView failed for textureId=%d mipLevel=%u", __func__, texture.GetExternalIndex(), mipLevel);
            return VK_NULL_HANDLE;
        }

        return perMipView;
    }

    void VkTextureManager::UpdateTrackedImageLayout(MG_State::GLState::ITextureObject* texture, VkImageLayout newLayout) {
        MOBILEGL_ASSERT(texture != nullptr, "UpdateTrackedImageLayout: texture is null");
        auto it = m_textureResources.find(texture);
        MOBILEGL_ASSERT(it != m_textureResources.end(),
                        "UpdateTrackedImageLayout: textureId=%d has no tracked resource", texture->GetExternalIndex());
        MOBILEGL_ASSERT(it->second.image != VK_NULL_HANDLE,
                        "UpdateTrackedImageLayout: textureId=%d has null image", texture->GetExternalIndex());
        it->second.layout = newLayout;
    }

    Bool VkTextureManager::TransitionTextureForSampling(VkCommandBuffer commandBuffer, MG_State::GLState::ITextureObject& texture) {
        TextureResource* resource = SyncTextureAndGetDescriptor(texture);
        if (resource == nullptr) {
            return false;
        }
        if (IsValidSampledImageLayout(resource->layout)) {
            return true;
        }
        if (resource->layout == VK_IMAGE_LAYOUT_UNDEFINED) {
            MGLOG_W("TransitionTextureForSampling: textureId=%d is still in VK_IMAGE_LAYOUT_UNDEFINED before sampling",
                    texture.GetExternalIndex());
        }

        VkImageLayout targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkAccessFlags srcAccessMask = 0;
        if ((resource->aspect & VK_IMAGE_ASPECT_COLOR_BIT) != 0) {
            MOBILEGL_ASSERT(resource->layout == VK_IMAGE_LAYOUT_UNDEFINED ||
                            resource->layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                            "TransitionTextureForSampling: unsupported color layout=%d for textureId=%d",
                            static_cast<Int>(resource->layout), texture.GetExternalIndex());
            if (resource->layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            }
            targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        } else if ((resource->aspect & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0) {
            MOBILEGL_ASSERT(resource->layout == VK_IMAGE_LAYOUT_UNDEFINED ||
                            resource->layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                            "TransitionTextureForSampling: unsupported depth/stencil layout=%d for textureId=%d",
                            static_cast<Int>(resource->layout), texture.GetExternalIndex());
            if (resource->layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            }
            targetLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        } else {
            MOBILEGL_ASSERT(false, "TransitionTextureForSampling: unsupported aspect mask=0x%x for textureId=%d",
                            static_cast<Uint32>(resource->aspect), texture.GetExternalIndex());
        }

        const Bool ok = TransitionImageLayout(commandBuffer, resource->image, resource->layout, targetLayout, srcStageMask,
                                              kGraphicsSampledReadStages, srcAccessMask,
                                              VK_ACCESS_SHADER_READ_BIT, resource->aspect, 0, resource->mipLevels);
        MOBILEGL_ASSERT(ok, "TransitionTextureForSampling: transition failed for textureId=%d", texture.GetExternalIndex());
        return ok;
    }

    Bool VkTextureManager::TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
                                                 VkImageLayout& trackedLayout, VkImageLayout newLayout,
                                                 VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                 VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                                 VkImageAspectFlags aspectMask, Uint32 baseMipLevel, Uint32 levelCount) {
        MOBILEGL_ASSERT(image != VK_NULL_HANDLE, "TransitionImageLayout: m_image == VK_NULL_HANDLE");
        MOBILEGL_ASSERT(!((dstAccessMask & VK_ACCESS_TRANSFER_READ_BIT) != 0 &&
                          (dstStageMask & VK_PIPELINE_STAGE_TRANSFER_BIT) == 0),
                        "TransitionImageLayout: invalid dstAccess/dstStage pair (dstAccess=0x%x, dstStage=0x%x, oldLayout=%d, newLayout=%d)",
                        static_cast<Uint32>(dstAccessMask), static_cast<Uint32>(dstStageMask), static_cast<Int>(trackedLayout),
                        static_cast<Int>(newLayout));

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
        barrier.subresourceRange.baseMipLevel = baseMipLevel;
        barrier.subresourceRange.levelCount = levelCount;
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
        if (!SyncTextureViews(texture, outResource)) {
            MGLOG_D("%s: SyncTextureViews failed", __func__);
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
            if (resource.perMipViews.size() != mipLevels) {
                resource.perMipViews.resize(mipLevels, VK_NULL_HANDLE);
            }
            return true;
        }

         resource.Reset();

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

        resource.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        resource.extent = {static_cast<Uint32>(texelSize.x()), static_cast<Uint32>(texelSize.y())};
        resource.mipLevels = mipLevels;
        resource.perMipViews.assign(mipLevels, VK_NULL_HANDLE);
        resource.sampledBaseMipLevel = 0;
        resource.sampledLevelCount = mipLevels;
        resource.format = format;
        resource.aspect = aspect;
        resource.syncedTextureParamsVersion = 0;
        return true;
    }

    Bool VkTextureManager::SyncTextureViews(const MG_State::GLState::ITextureObject& texture, TextureResource& resource) {
        MOBILEGL_ASSERT(resource.image != VK_NULL_HANDLE, "SyncTextureViews: image == VK_NULL_HANDLE");

        Uint32 baseMipLevel = 0;
        Uint32 levelCount = 1;
        ResolveViewMipRange(texture, resource.mipLevels, baseMipLevel, levelCount);

        const Bool needsRecreate =
            resource.fullView == VK_NULL_HANDLE ||
            resource.sampledBaseMipLevel != baseMipLevel ||
            resource.sampledLevelCount != levelCount ||
            resource.syncedTextureParamsVersion != texture.GetTextureParamsVersion();
        if (!needsRecreate) {
            return true;
        }

        if (resource.fullView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device, resource.fullView, nullptr);
            resource.fullView = VK_NULL_HANDLE;
        }

        resource.fullView = CreateImageView(resource.image, resource.format, resource.aspect, baseMipLevel, levelCount);
        if (resource.fullView == VK_NULL_HANDLE) {
            return false;
        }

        resource.sampledBaseMipLevel = baseMipLevel;
        resource.sampledLevelCount = levelCount;
        resource.syncedTextureParamsVersion = texture.GetTextureParamsVersion();
        return true;
    }

    VkImageView VkTextureManager::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect,
                                                  Uint32 baseMipLevel, Uint32 levelCount) const {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspect;
        viewInfo.subresourceRange.baseMipLevel = baseMipLevel;
        viewInfo.subresourceRange.levelCount = levelCount;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView view = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateImageView(m_device, &viewInfo, nullptr, &view), "vkCreateImageView(texture)");
        return view;
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

        const VkImageAspectFlags aspectMask = GetAspectMaskForFormat(outResource.format);
        Bool ok = TransitionImageLayout(commandBuffer, outResource.image,
                              outResource.layout,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              outResource.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ?
                                  kGraphicsSampledReadStages :
                                  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                              VK_PIPELINE_STAGE_TRANSFER_BIT,
                              outResource.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ? VK_ACCESS_SHADER_READ_BIT : 0,
                              VK_ACCESS_TRANSFER_WRITE_BIT,
                              aspectMask, 0, outResource.mipLevels);
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
                                        kGraphicsSampledReadStages,
                                        VK_ACCESS_TRANSFER_WRITE_BIT,
                                        VK_ACCESS_SHADER_READ_BIT,
                                        aspectMask, 0, outResource.mipLevels);
        MOBILEGL_ASSERT(ok, "TransitionImageLayout to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL failed");
        outResource.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VK_VERIFY(vkEndCommandBuffer(commandBuffer), "vkEndCommandBuffer(texture)");

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence uploadFence = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateFence(m_device, &fenceInfo, nullptr, &uploadFence), "vkCreateFence(texture upload)");

        VK_VERIFY(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, uploadFence), "vkQueueSubmit(texture)");
        VK_VERIFY(vkWaitForFences(m_device, 1, &uploadFence, VK_TRUE, UINT64_MAX), "vkWaitForFences(texture upload)");
        vkDestroyFence(m_device, uploadFence, nullptr);
        vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);

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

            // Backing VkImage allocation still uses storage mip 0 as the physical image extent.
            // GL_TEXTURE_BASE_LEVEL / MAX_LEVEL are applied later when building the sampled view.
            const auto storageBaseTexelSize = mipTexture->GetMipmapTexelSize(target, 0);
            const auto storageBaseByteSize = mipTexture->GetMipmapByteSize(target, 0);
            if (storageBaseTexelSize.x() <= 0 || storageBaseTexelSize.y() <= 0 /*|| storageBaseByteSize == 0*/) {
                continue;
            }

            outTarget = target;
            outTexelSize = storageBaseTexelSize;
            outByteSize = storageBaseByteSize;
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

    void VkTextureManager::ResolveViewMipRange(const MG_State::GLState::ITextureObject& texture, Uint32 mipLevels,
                                               Uint32& outBaseMipLevel, Uint32& outLevelCount) {
        MOBILEGL_ASSERT(mipLevels > 0, "ResolveViewMipRange: mipLevels must be > 0");

        const auto& levelRange = texture.GetLevelRange();
        const Uint32 maxAvailableMipLevel = mipLevels - 1;
        const Uint32 requestedBaseMipLevel = std::min(static_cast<Uint32>(levelRange.x()), maxAvailableMipLevel);
        Uint32 requestedMaxMipLevel = std::min(static_cast<Uint32>(levelRange.y()), maxAvailableMipLevel);
        if (requestedMaxMipLevel < requestedBaseMipLevel) {
            requestedMaxMipLevel = requestedBaseMipLevel;
        }

        outBaseMipLevel = requestedBaseMipLevel;
        outLevelCount = requestedMaxMipLevel - requestedBaseMipLevel + 1;
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
