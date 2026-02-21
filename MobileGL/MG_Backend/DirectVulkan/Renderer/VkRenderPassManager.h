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

        Bool Initialize(const InitInfo& initInfo);
        void Shutdown();

        VkRenderPass GetLoadRenderPass() const;
        VkRenderPass GetClearRenderPass() const;

    private:
        VkRenderPass CreateDefaultRenderPass(VkAttachmentLoadOp colorLoadOp) const;

        VkDevice m_device = VK_NULL_HANDLE;
        VkFormat m_colorFormat = VK_FORMAT_UNDEFINED;
        VkFormat m_depthStencilFormat = VK_FORMAT_UNDEFINED;
        VkRenderPass m_renderPassLoad = VK_NULL_HANDLE;
        VkRenderPass m_renderPassClear = VK_NULL_HANDLE;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
