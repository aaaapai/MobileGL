// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkTextureManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VkTextureManager.h"

#include "MG_State/GLState/Core.h"

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

        return true;
    }

    void VkTextureManager::Shutdown() {
        for (auto& [_, resource] : m_textureResources) {
            DestroyTextureResource(resource);
        }
        m_textureResources.clear();

        m_device = VK_NULL_HANDLE;
        m_physicalDevice = VK_NULL_HANDLE;
        m_allocator = nullptr;
        m_commandPool = VK_NULL_HANDLE;
        m_graphicsQueue = VK_NULL_HANDLE;
    }

    Bool VkTextureManager::SyncTextureAndGetDescriptor(const MG_State::GLState::ITextureObject& texture,
                                                       VkDescriptorImageInfo& outImageInfo) {
        MOBILEGL_ASSERT(m_device != VK_NULL_HANDLE, "SyncTextureAndGetDescriptor: m_device == VK_NULL_HANDLE");

        auto it = m_textureResources.find(texture.GetExternalIndex());
        if (it == m_textureResources.end()) {
            TextureResource initial{};
            initial.textureExternalIndex = texture.GetExternalIndex();
            auto [insertIt, _] = m_textureResources.emplace(texture.GetExternalIndex(), initial);
            it = insertIt;
        }

        if (!EnsureTextureSynced(it->second, texture)) {
            return false;
        }

        if (it->second.view == VK_NULL_HANDLE) {
            return false;
        }

        outImageInfo.sampler = VK_NULL_HANDLE;
        outImageInfo.imageView = it->second.view;
        outImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        return true;
    }

    Bool VkTextureManager::EnsureTextureSynced(TextureResource& resource, const MG_State::GLState::ITextureObject& texture) {
        TextureUploadTarget level0Target = TextureUploadTarget::Unknown;
        IntVec3 texelSize{0, 0, 0};
        SizeT byteSize = 0;
        if (!ResolveLevel0(texture, level0Target, texelSize, byteSize)) {
            return false;
        }

        if (!EnsureTextureResource(resource, texture, level0Target, texelSize, byteSize)) {
            return false;
        }

        const auto* mipTexture = dynamic_cast<const MG_State::GLState::TextureObjectMipmap*>(&texture);
        if (!mipTexture) {
            return false;
        }

        if (!mipTexture->IsStorageDirty(level0Target, 0)) {
            return true;
        }

        if (!UploadLevel0(resource, *mipTexture, level0Target, byteSize)) {
            return false;
        }

        auto& mutableTexture = const_cast<MG_State::GLState::TextureObjectMipmap&>(*mipTexture);
        mutableTexture.MarkStorageDirty(level0Target, 0, false);
        return true;
    }

    Bool VkTextureManager::EnsureTextureResource(TextureResource& resource, const MG_State::GLState::ITextureObject& texture,
                                                 TextureUploadTarget level0Target, const IntVec3& texelSize,
                                                 SizeT byteSize) {
        const VkFormat format = ResolveTextureFormat(texture.GetFormat());
        if (format == VK_FORMAT_UNDEFINED) {
            return false;
        }
        if (texelSize.x() <= 0 || texelSize.y() <= 0 || byteSize == 0) {
            return false;
        }
        if (level0Target != TextureUploadTarget::Texture2D) {
            return false;
        }

        const Bool compatible = resource.image != VK_NULL_HANDLE && resource.format == format &&
                                resource.extent.width == static_cast<Uint32>(texelSize.x()) &&
                                resource.extent.height == static_cast<Uint32>(texelSize.y());
        if (compatible) {
            return true;
        }

        DestroyTextureResource(resource);

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<Uint32>(texelSize.x());
        imageInfo.extent.height = static_cast<Uint32>(texelSize.y());
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
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
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        VK_VERIFY(vkCreateImageView(m_device, &viewInfo, nullptr, &resource.view), "vkCreateImageView(texture)");

        resource.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        resource.extent = {static_cast<Uint32>(texelSize.x()), static_cast<Uint32>(texelSize.y())};
        resource.format = format;
        resource.textureExternalIndex = texture.GetExternalIndex();
        return true;
    }

    Bool VkTextureManager::UploadLevel0(TextureResource& resource, const MG_State::GLState::TextureObjectMipmap& mipmapTexture,
                                        TextureUploadTarget level0Target, SizeT byteSize) {
        auto& mutableTexture = const_cast<MG_State::GLState::TextureObjectMipmap&>(mipmapTexture);
        const void* source = mutableTexture.MapMipmapData(level0Target, 0);
        if (source == nullptr || byteSize == 0) {
            return false;
        }

        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VmaAllocation stagingAllocation = nullptr;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = byteSize;
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
        std::memcpy(mapped, source, byteSize);
        vmaUnmapMemory(m_allocator, stagingAllocation);

        const Bool ok = ExecuteImmediate([&](VkCommandBuffer commandBuffer) {
            VkImageMemoryBarrier toTransferDst{};
            toTransferDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            toTransferDst.srcAccessMask = 0;
            toTransferDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            toTransferDst.oldLayout = resource.layout;
            toTransferDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            toTransferDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toTransferDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toTransferDst.image = resource.image;
            toTransferDst.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            toTransferDst.subresourceRange.baseMipLevel = 0;
            toTransferDst.subresourceRange.levelCount = 1;
            toTransferDst.subresourceRange.baseArrayLayer = 0;
            toTransferDst.subresourceRange.layerCount = 1;
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                                 nullptr, 0, nullptr, 1, &toTransferDst);

            VkBufferImageCopy copy{};
            copy.bufferOffset = 0;
            copy.bufferRowLength = 0;
            copy.bufferImageHeight = 0;
            copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copy.imageSubresource.mipLevel = 0;
            copy.imageSubresource.baseArrayLayer = 0;
            copy.imageSubresource.layerCount = 1;
            copy.imageOffset = {0, 0, 0};
            copy.imageExtent = {resource.extent.width, resource.extent.height, 1};
            vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, resource.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                   &copy);

            VkImageMemoryBarrier toSampled{};
            toSampled.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            toSampled.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            toSampled.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            toSampled.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            toSampled.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            toSampled.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toSampled.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toSampled.image = resource.image;
            toSampled.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            toSampled.subresourceRange.baseMipLevel = 0;
            toSampled.subresourceRange.levelCount = 1;
            toSampled.subresourceRange.baseArrayLayer = 0;
            toSampled.subresourceRange.layerCount = 1;
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                                 0, nullptr, 0, nullptr, 1, &toSampled);
        });

        vmaDestroyBuffer(m_allocator, stagingBuffer, stagingAllocation);

        if (!ok) {
            return false;
        }
        resource.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        return true;
    }

    Bool VkTextureManager::ExecuteImmediate(const std::function<void(VkCommandBuffer)>& recorder) const {
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

    void VkTextureManager::DestroyTextureResource(TextureResource& resource) const {
        if (m_device != VK_NULL_HANDLE && resource.view != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device, resource.view, nullptr);
        }
        if (m_allocator != nullptr && resource.image != VK_NULL_HANDLE && resource.allocation != nullptr) {
            vmaDestroyImage(m_allocator, resource.image, resource.allocation);
        }
        resource.view = VK_NULL_HANDLE;
        resource.image = VK_NULL_HANDLE;
        resource.allocation = nullptr;
        resource.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        resource.extent = {0, 0};
        resource.format = VK_FORMAT_UNDEFINED;
    }

    Bool VkTextureManager::ResolveLevel0(const MG_State::GLState::ITextureObject& texture, TextureUploadTarget& outTarget,
                                         IntVec3& outTexelSize, SizeT& outByteSize) {
        const auto* mipTexture = dynamic_cast<const MG_State::GLState::TextureObjectMipmap*>(&texture);
        if (!mipTexture) {
            return false;
        }
        const auto& targets = texture.GetUploadTargets();
        if (targets.empty()) {
            return false;
        }
        outTarget = targets.front();
        outTexelSize = mipTexture->GetMipmapTexelSize(outTarget, 0);
        outByteSize = mipTexture->GetMipmapByteSize(outTarget, 0);
        return outTexelSize.x() > 0 && outTexelSize.y() > 0 && outByteSize > 0;
    }

    VkFormat VkTextureManager::ResolveTextureFormat(TextureInternalFormat format) {
        switch (format) {
        case TextureInternalFormat::RGBA:
        case TextureInternalFormat::RGBA8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureInternalFormat::SRGB8Alpha8:
            return VK_FORMAT_R8G8B8A8_SRGB;
        default:
            return VK_FORMAT_UNDEFINED;
        }
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
