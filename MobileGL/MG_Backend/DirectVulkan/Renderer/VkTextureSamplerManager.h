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
#include <MG_State/GLState/SamplerState/SamplerObject.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

namespace MobileGL::MG_State::GLState {
class ITextureObject;
class SamplerObject;
}

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

    Bool SyncTextureAndGetDescriptor(const MG_State::GLState::ITextureObject& texture,
                                     const MG_State::GLState::SamplerObject* samplerOverride,
                                     VkDescriptorImageInfo& outImageInfo);

private:
    struct TextureResource {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkExtent2D extent = {0, 0};
        VkFormat format = VK_FORMAT_UNDEFINED;
        Uint textureExternalIndex = 0;
    };

    struct SamplerCacheEntry {
        VkSampler handle = VK_NULL_HANDLE;
        Uint externalIndex = 0;
        Uint16 version = 0;
    };

    Bool EnsureTextureSynced(TextureResource& resource, const MG_State::GLState::ITextureObject& texture);
    Bool EnsureTextureResource(TextureResource& resource, const MG_State::GLState::ITextureObject& texture,
                               TextureUploadTarget level0Target, const IntVec3& texelSize, SizeT byteSize);
    Bool UploadLevel0(TextureResource& resource, const MG_State::GLState::TextureObjectMipmap& mipmapTexture,
                      TextureUploadTarget level0Target, SizeT byteSize);
    Bool ExecuteImmediate(const std::function<void(VkCommandBuffer)>& recorder) const;
    void DestroyTextureResource(TextureResource& resource) const;
    static Bool ResolveLevel0(const MG_State::GLState::ITextureObject& texture, TextureUploadTarget& outTarget,
                              IntVec3& outTexelSize, SizeT& outByteSize);
    static VkFormat ResolveTextureFormat(TextureInternalFormat format);
    Uint32 FindMemoryType(Uint32 typeFilter, VkMemoryPropertyFlags properties) const;

    VkSampler GetOrCreateSampler(const MG_State::GLState::SamplerObject& sampler);
    static VkFilter ToVkFilter(SamplerFilterMode mode);
    static VkSamplerMipmapMode ToVkMipmapMode(SamplerMipmapMode mode);
    static VkSamplerAddressMode ToVkAddressMode(SamplerWrapMode mode);
    static VkCompareOp ToVkCompareOp(SamplerCompareFunc func);

    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;

    UnorderedMap<Uint, TextureResource> m_textureResources;
    UnorderedMap<Uint64, SamplerCacheEntry> m_samplers;
};
} // namespace MobileGL::MG_Backend::DirectVulkan
