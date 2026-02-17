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

namespace MobileGL::MG_Backend::DirectVulkan {
    class VkFramebufferManager {
    public:
        struct InitInfo {
            VkDevice device = VK_NULL_HANDLE;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        };

        VkFramebufferManager() = default;
        ~VkFramebufferManager() = default;

        Bool Initialize(const InitInfo& initInfo);
        void Shutdown();

        Bool EnsureOffscreenColorTarget(Uint glFboExternalIndex, const MG_State::GLState::FramebufferObject& glFbo);
        Bool ClearColor(VkCommandBuffer commandBuffer, Uint glFboExternalIndex, const VkClearColorValue& clearColor);
        Bool TransitionOffscreenColorToTransferSrc(VkCommandBuffer commandBuffer, Uint glFboExternalIndex);
        Bool GetOffscreenColorImage(Uint glFboExternalIndex, VkImage& outImage, VkExtent2D& outExtent) const;

    private:
        struct OffscreenColorTarget {
            VkImage image = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;
            VkImageView imageView = VK_NULL_HANDLE;
            VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkExtent2D extent = {0, 0};
            VkFormat format = VK_FORMAT_UNDEFINED;
            Uint16 glObjectVersion = 0;
        };

        Bool RecreateOffscreenColorTarget(OffscreenColorTarget& target,
                                          const MG_State::GLState::FramebufferAttachmentObject& colorAttachment,
                                          Uint16 glObjectVersion);
        void DestroyOffscreenColorTarget(OffscreenColorTarget& target);
        Bool TransitionColorTargetForClear(VkCommandBuffer commandBuffer, OffscreenColorTarget& target);
        Bool TransitionColorTargetForBlitSrc(VkCommandBuffer commandBuffer, OffscreenColorTarget& target);
        Uint32 FindMemoryType(Uint32 typeFilter, VkMemoryPropertyFlags properties) const;
        static VkFormat ResolveColorFormat(const MG_State::GLState::FramebufferAttachmentObject& colorAttachment);

        VkDevice m_device = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        UnorderedMap<Uint, OffscreenColorTarget> m_offscreenColorTargets;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
