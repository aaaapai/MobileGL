// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkTextureManager.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "../VkIncludes.h"
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>
#include <vk_mem_alloc.h>

namespace MobileGL::MG_State::GLState {
class ITextureObject;
}

namespace MobileGL::MG_Backend::DirectVulkan {
class VkTextureManager {
public:
    struct InitInfo {
        VkDevice device = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VmaAllocator allocator = nullptr;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
    };

    struct TextureResource {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation allocation = nullptr;
        VkImageView view = VK_NULL_HANDLE;
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkExtent2D extent = {0, 0};
        Uint32 mipLevels = 1;
        VkFormat format = VK_FORMAT_UNDEFINED;

        TextureResource() {}
        TextureResource(const TextureResource&) = delete;
        TextureResource(TextureResource&& that) noexcept {
            std::swap(this->image, that.image);
            std::swap(this->allocation, that.allocation);
            std::swap(this->view, that.view);
            std::swap(this->layout, that.layout);
            std::swap(this->extent, that.extent);
            std::swap(this->mipLevels, that.mipLevels);
            std::swap(this->format, that.format);
        }

        ~TextureResource() {
            if (view != VK_NULL_HANDLE) {
                vkDestroyImageView(s_device, view, nullptr);
            }
            if (image != VK_NULL_HANDLE && allocation != nullptr) {
                vmaDestroyImage(s_allocator, image, allocation);
            }
            view = VK_NULL_HANDLE;
            image = VK_NULL_HANDLE;
            allocation = nullptr;
            layout = VK_IMAGE_LAYOUT_UNDEFINED;
            extent = {0, 0};
            mipLevels = 1;
            format = VK_FORMAT_UNDEFINED;
        }

        static inline VkDevice s_device = VK_NULL_HANDLE;
        static inline VmaAllocator s_allocator = VK_NULL_HANDLE;
    };

    Bool Initialize(const InitInfo& initInfo);
    void Shutdown();

    TextureResource* SyncTextureAndGetDescriptor(
        MG_State::GLState::ITextureObject& texture);

    static Bool TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout& trackedLayout,
                               VkImageLayout newLayout, VkPipelineStageFlags srcStageMask,
                               VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask,
                               VkAccessFlags dstAccessMask, VkImageAspectFlags aspectMask);

    SizeT CollectGarbage();
private:

    Bool SyncTexture(MG_State::GLState::ITextureObject &texture,
                     TextureResource &outResource);
    Bool SyncTextureResource(const MG_State::GLState::ITextureObject &texture,
                             TextureUploadTarget uploadTarget,
                             const IntVec3 &texelSize, SizeT byteSize, Uint32 mipLevels,
                             TextureResource &resource);
    Bool UploadDirtyMipLevels(MG_State::GLState::TextureObjectMipmap &mipmapTexture,
                      TextureUploadTarget uploadTarget,
                      TextureResource &outResource);
    Bool ExecuteCmdBufImmediate(const std::function<void(VkCommandBuffer)>& recorder) const;
    static Bool CheckMipmapCompleteness(const MG_State::GLState::ITextureObject& texture,
                                        TextureUploadTarget& outTarget,
                                        IntVec3& outTexelSize,
                                        SizeT& outByteSize,
                                        Uint32& outMipLevelCount);
    static Uint32 GetUploadMipLevelCount(const MG_State::GLState::TextureObjectMipmap& texture, TextureUploadTarget target);
    static VkFormat GetVkFormat(TextureInternalFormat format);

    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VmaAllocator m_allocator = nullptr;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;

    Uint8 m_gcCounter = 0;
    UnorderedMap<MG_State::GLState::ITextureObject*, WeakPtr<MG_State::GLState::ITextureObject>> m_aliveObjects;
    UnorderedMap<MG_State::GLState::ITextureObject*, TextureResource> m_textureResources;
};
} // namespace MobileGL::MG_Backend::DirectVulkan
