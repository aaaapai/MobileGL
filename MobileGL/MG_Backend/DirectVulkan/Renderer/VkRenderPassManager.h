// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "../VkIncludes.h"
#include <Includes.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    class VkRenderPassManager {
    public:
        struct InitInfo {
            VkDevice device = VK_NULL_HANDLE;
            VkFormat colorFormat = VK_FORMAT_UNDEFINED;
            VkFormat depthStencilFormat = VK_FORMAT_UNDEFINED;
        };

        struct OffscreenRenderTargetInfo {
            Uint targetExternalIndex = 0;
            Uint16 targetVersion = 0;
            VkImageView colorView = VK_NULL_HANDLE;
            VkFormat colorFormat = VK_FORMAT_UNDEFINED;
            VkImageView depthStencilView = VK_NULL_HANDLE;
            VkFormat depthStencilFormat = VK_FORMAT_UNDEFINED;
            VkExtent2D extent = {0, 0};
        };

        Bool Initialize(const InitInfo& initInfo);
        void Shutdown();

        Bool RecreateDefaultFramebuffers(const Vector<VkImageView>& colorViews,
                                         const Vector<VkImageView>& depthStencilViews, VkExtent2D extent);
        Bool GetDefaultRenderTarget(Uint32 imageIndex, VkRenderPass& outRenderPass, VkFramebuffer& outFramebuffer,
                                    VkExtent2D& outExtent, VkFormat& outDepthStencilFormat) const;

        Bool EnsureOffscreenRenderTarget(const OffscreenRenderTargetInfo& targetInfo);
        Bool GetOffscreenRenderTarget(Uint targetExternalIndex, VkRenderPass& outRenderPass,
                                      VkFramebuffer& outFramebuffer, VkExtent2D& outExtent,
                                      VkFormat& outDepthStencilFormat) const;
        void RemoveOffscreenRenderTarget(Uint targetExternalIndex);

        void BeginRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer framebuffer,
                             VkExtent2D extent) const;
        void EndRenderPass(VkCommandBuffer commandBuffer) const;
        void RecordColorClear(VkCommandBuffer commandBuffer, VkExtent2D extent,
                              const VkClearColorValue& clearColor) const;
        void RecordDepthStencilClear(VkCommandBuffer commandBuffer, VkExtent2D extent, GLbitfield mask, Float depth,
                                     Uint32 stencil, VkFormat depthStencilFormat) const;

    private:
        struct OffscreenRenderTarget {
            Uint16 targetVersion = 0;
            VkImageView colorView = VK_NULL_HANDLE;
            VkFormat colorFormat = VK_FORMAT_UNDEFINED;
            VkImageView depthStencilView = VK_NULL_HANDLE;
            VkFormat depthStencilFormat = VK_FORMAT_UNDEFINED;
            VkExtent2D extent = {0, 0};
            VkRenderPass renderPassLoad = VK_NULL_HANDLE;
            VkFramebuffer framebuffer = VK_NULL_HANDLE;
        };

        VkRenderPass CreateDefaultRenderPass(VkAttachmentLoadOp colorLoadOp) const;
        VkRenderPass CreateRenderPass(VkFormat colorFormat, VkFormat depthStencilFormat, VkAttachmentLoadOp colorLoadOp,
                                      VkImageLayout colorFinalLayout) const;
        void DestroyDefaultFramebuffers();
        void DestroyOffscreenRenderTarget(OffscreenRenderTarget& target);
        static Bool HasStencilComponent(VkFormat format);

        VkDevice m_device = VK_NULL_HANDLE;
        VkFormat m_colorFormat = VK_FORMAT_UNDEFINED;
        VkFormat m_depthStencilFormat = VK_FORMAT_UNDEFINED;
        VkRenderPass m_renderPassLoad = VK_NULL_HANDLE;
        Vector<VkFramebuffer> m_defaultFramebuffers;
        VkExtent2D m_defaultExtent = {0, 0};
        UnorderedMap<Uint, OffscreenRenderTarget> m_offscreenRenderTargets;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
