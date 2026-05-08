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
        Uint32 frameCount = 0;
    };

    struct TextureResource {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation allocation = nullptr;
        VkImageView fullView = VK_NULL_HANDLE;
        Vector<VkImageView> perMipViews;
        Vector<VkImageView> perMipSampledViews;
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkExtent2D extent = {0, 0};
        Uint32 depth = 1;
        Uint32 arrayLayers = 1;
        Uint32 mipLevels = 1;
        Uint32 sampledBaseMipLevel = 0;
        Uint32 sampledLevelCount = 1;
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkImageAspectFlags aspect = VK_IMAGE_ASPECT_NONE;
        VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
        Uint16 syncedTextureParamsVersion = 0;

        TextureResource() = default;
        TextureResource(const TextureResource&) = delete;
        TextureResource(TextureResource&& that) noexcept {
            std::swap(this->image, that.image);
            std::swap(this->allocation, that.allocation);
            std::swap(this->fullView, that.fullView);
            std::swap(this->perMipViews, that.perMipViews);
            std::swap(this->perMipSampledViews, that.perMipSampledViews);
            std::swap(this->layout, that.layout);
            std::swap(this->extent, that.extent);
            std::swap(this->depth, that.depth);
            std::swap(this->arrayLayers, that.arrayLayers);
            std::swap(this->mipLevels, that.mipLevels);
            std::swap(this->sampledBaseMipLevel, that.sampledBaseMipLevel);
            std::swap(this->sampledLevelCount, that.sampledLevelCount);
            std::swap(this->format, that.format);
            std::swap(this->aspect, that.aspect);
            std::swap(this->viewType, that.viewType);
            std::swap(this->syncedTextureParamsVersion, that.syncedTextureParamsVersion);
        }

        void Reset() {
            if (fullView != VK_NULL_HANDLE) {
                vkDestroyImageView(s_device, fullView, nullptr);
            }
            for (const auto attachmentView : perMipViews) {
                if (attachmentView != VK_NULL_HANDLE) {
                    vkDestroyImageView(s_device, attachmentView, nullptr);
                }
            }
            for (const auto sampledView : perMipSampledViews) {
                if (sampledView != VK_NULL_HANDLE) {
                    vkDestroyImageView(s_device, sampledView, nullptr);
                }
            }
            if (image != VK_NULL_HANDLE && allocation != nullptr) {
                vmaDestroyImage(s_allocator, image, allocation);
            }
            fullView = VK_NULL_HANDLE;
            perMipViews.clear();
            perMipSampledViews.clear();
            image = VK_NULL_HANDLE;
            allocation = nullptr;
            layout = VK_IMAGE_LAYOUT_UNDEFINED;
            extent = {0, 0};
            depth = 1;
            arrayLayers = 1;
            mipLevels = 1;
            sampledBaseMipLevel = 0;
            sampledLevelCount = 1;
            format = VK_FORMAT_UNDEFINED;
            aspect = VK_IMAGE_ASPECT_NONE;
            viewType = VK_IMAGE_VIEW_TYPE_2D;
            syncedTextureParamsVersion = 0;
        }

        ~TextureResource() {
            Reset();
        }

        static inline VkDevice s_device = VK_NULL_HANDLE;
        static inline VmaAllocator s_allocator = VK_NULL_HANDLE;
    };

    Bool Initialize(const InitInfo& initInfo);
    void Shutdown();
    void BeginFrame(Uint32 frameIndex);

    TextureResource* SyncTextureAndGetDescriptor(
        MG_State::GLState::ITextureObject& texture);
    VkImageView GetOrCreateViewAtMipLevel(MG_State::GLState::ITextureObject& texture, Uint32 mipLevel);
    VkImageView GetOrCreateSampledViewAtMipLevel(MG_State::GLState::ITextureObject& texture, Uint32 mipLevel);
    void UpdateTrackedImageLayout(MG_State::GLState::ITextureObject* texture, VkImageLayout newLayout);
    void UpdateTrackedImageLayoutAfterAttachmentWrite(VkCommandBuffer commandBuffer,
                                                      MG_State::GLState::ITextureObject* texture,
                                                      Uint32 writtenMipLevel,
                                                      VkImageLayout newLayout);
    Bool TransitionTextureForSampling(VkCommandBuffer commandBuffer, MG_State::GLState::ITextureObject& texture);

    static Bool TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout& trackedLayout,
                               VkImageLayout newLayout, VkPipelineStageFlags srcStageMask,
                               VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask,
                               VkAccessFlags dstAccessMask, VkImageAspectFlags aspectMask,
                               Uint32 baseMipLevel = 0, Uint32 levelCount = 1);

    SizeT CollectGarbage();
private:

    Bool SyncTexture(MG_State::GLState::ITextureObject &texture,
                     TextureResource &outResource);
    Bool SyncTextureResource(const MG_State::GLState::ITextureObject &texture,
                             TextureUploadTarget uploadTarget,
                             const IntVec3 &texelSize, SizeT byteSize, Uint32 mipLevels,
                             TextureResource &resource);
    Bool SyncTextureViews(const MG_State::GLState::ITextureObject& texture, TextureResource& resource);
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect,
                                VkImageViewType viewType, Uint32 baseMipLevel, Uint32 levelCount,
                                Uint32 layerCount,
                                const VkComponentMapping* components = nullptr) const;
    Bool UploadDirtyMipLevels(MG_State::GLState::TextureObjectMipmap &mipmapTexture,
                      TextureUploadTarget uploadTarget,
                      TextureResource &outResource);
    static Bool CheckMipmapCompleteness(const MG_State::GLState::ITextureObject& texture,
                                        TextureUploadTarget& outTarget,
                                        IntVec3& outTexelSize,
                                        SizeT& outByteSize,
                                        Uint32& outMipLevelCount);
    static Uint32 GetUploadMipLevelCount(const MG_State::GLState::TextureObjectMipmap& texture, TextureUploadTarget target);
    static void ResolveViewMipRange(const MG_State::GLState::ITextureObject& texture, Uint32 mipLevels,
                                    Uint32& outBaseMipLevel, Uint32& outLevelCount);
    static VkImageAspectFlags GetAspectMaskForFormat(VkFormat format);
    void DeferResourceRelease(TextureResource&& resource);
    void DeferViewRelease(VkImageView view);
    void CollectDeferredReleases(Uint32 frameIndex);
    void DestroyDeferredReleases();

    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VmaAllocator m_allocator = nullptr;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    Uint32 m_currentFrameIndex = 0;

    Uint8 m_gcCounter = 0;
    UnorderedMap<MG_State::GLState::ITextureObject*, WeakPtr<MG_State::GLState::ITextureObject>> m_aliveObjects;
    UnorderedMap<MG_State::GLState::ITextureObject*, TextureResource> m_textureResources;
    Vector<Vector<TextureResource>> m_deferredReleases;
    Vector<Vector<VkImageView>> m_deferredViewReleases;
};
} // namespace MobileGL::MG_Backend::DirectVulkan
