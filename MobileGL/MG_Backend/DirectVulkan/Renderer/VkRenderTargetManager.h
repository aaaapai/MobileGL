// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkFramebufferManager.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "../VkIncludes.h"
#include <Includes.h>
#include <MG_State/GLState/FramebufferState/FramebufferObject.h>
#include <vk_mem_alloc.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    class VkRenderTargetManager {
    public:
        struct InitInfo {
            VkDevice device = VK_NULL_HANDLE;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VmaAllocator allocator = nullptr;
        };

        VkRenderTargetManager() = default;
        ~VkRenderTargetManager() = default;

        Bool Initialize(const InitInfo& initInfo);
        void Shutdown();

        Bool EnsureOffscreenColorTarget(Uint glFboExternalIndex, const MG_State::GLState::FramebufferObject& glFbo);
        Bool GetOffscreenRenderSurfaceState(Uint glFboExternalIndex, VkImage& outColorImage,
                                            VkImageLayout*& outColorLayout, VkImage& outDepthStencilImage,
                                            VkImageLayout*& outDepthStencilLayout,
                                            VkFormat& outDepthStencilFormat);
        Bool GetOffscreenColorTargetStateByTexture(Uint textureExternalIndex, VkImageView& outImageView,
                                                   VkImage& outImage, VkImageLayout*& outLayout);
        Bool GetOffscreenColorViewByTexture(Uint textureExternalIndex, VkImageView& outImageView) const;
        Bool GetOffscreenRenderSurface(Uint glFboExternalIndex, VkImageView& outColorView, VkFormat& outColorFormat,
                                       VkImageView& outDepthStencilView, VkFormat& outDepthStencilFormat,
                                       VkExtent2D& outExtent) const;

    private:
        struct OffscreenColorTarget {
            VkImage image = VK_NULL_HANDLE;
            VmaAllocation allocation = nullptr;
            VkImageView imageView = VK_NULL_HANDLE;
            VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkExtent2D extent = {0, 0};
            VkFormat format = VK_FORMAT_UNDEFINED;
            VkImage depthStencilImage = VK_NULL_HANDLE;
            VmaAllocation depthStencilAllocation = nullptr;
            VkImageView depthStencilImageView = VK_NULL_HANDLE;
            VkImageLayout depthStencilLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkFormat depthStencilFormat = VK_FORMAT_UNDEFINED;
            Uint16 glObjectVersion = 0;
            Uint colorTextureExternalIndex = 0;
        };

        Bool RecreateOffscreenColorTarget(OffscreenColorTarget& target,
                                          const MG_State::GLState::FramebufferObject& glFbo,
                                          const MG_State::GLState::FramebufferAttachmentObject& colorAttachment,
                                          Uint16 glObjectVersion);
        void DestroyOffscreenColorTarget(OffscreenColorTarget& target);
        static VkFormat ResolveColorFormat(const MG_State::GLState::FramebufferAttachmentObject& colorAttachment);
        static VkFormat ResolveDepthStencilFormat(
            const MG_State::GLState::FramebufferAttachmentObject& depthAttachment,
            const MG_State::GLState::FramebufferAttachmentObject& stencilAttachment);
        VkFormat FindSupportedDepthStencilFormat(const Vector<VkFormat>& candidates) const;

        VkDevice m_device = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VmaAllocator m_allocator = nullptr;
        UnorderedMap<Uint, OffscreenColorTarget> m_offscreenColorTargets;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
