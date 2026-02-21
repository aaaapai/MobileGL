// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VkRenderPassManager.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    Bool VkRenderPassManager::Initialize(const InitInfo& initInfo) {
        Shutdown();

        if (initInfo.device == VK_NULL_HANDLE || initInfo.colorFormat == VK_FORMAT_UNDEFINED ||
            initInfo.depthStencilFormat == VK_FORMAT_UNDEFINED) {
            return false;
        }

        m_device = initInfo.device;
        m_colorFormat = initInfo.colorFormat;
        m_depthStencilFormat = initInfo.depthStencilFormat;
        m_renderPassLoad = CreateDefaultRenderPass(VK_ATTACHMENT_LOAD_OP_LOAD);
        m_renderPassClear = CreateDefaultRenderPass(VK_ATTACHMENT_LOAD_OP_CLEAR);
        MGLOG_D("VkRenderPassManager: RenderPasses created (LOAD/CLEAR).");
        return m_renderPassLoad != VK_NULL_HANDLE && m_renderPassClear != VK_NULL_HANDLE;
    }

    void VkRenderPassManager::Shutdown() {
        if (m_device != VK_NULL_HANDLE) {
            if (m_renderPassClear != VK_NULL_HANDLE) {
                vkDestroyRenderPass(m_device, m_renderPassClear, nullptr);
            }
            if (m_renderPassLoad != VK_NULL_HANDLE) {
                vkDestroyRenderPass(m_device, m_renderPassLoad, nullptr);
            }
        }

        m_renderPassClear = VK_NULL_HANDLE;
        m_renderPassLoad = VK_NULL_HANDLE;
        m_colorFormat = VK_FORMAT_UNDEFINED;
        m_depthStencilFormat = VK_FORMAT_UNDEFINED;
        m_device = VK_NULL_HANDLE;
    }

    VkRenderPass VkRenderPassManager::GetLoadRenderPass() const {
        return m_renderPassLoad;
    }

    VkRenderPass VkRenderPassManager::GetClearRenderPass() const {
        return m_renderPassClear;
    }

    VkRenderPass VkRenderPassManager::CreateDefaultRenderPass(VkAttachmentLoadOp colorLoadOp) const {
        MOBILEGL_ASSERT(m_device != VK_NULL_HANDLE, "VkRenderPassManager: device is null");
        MOBILEGL_ASSERT(m_colorFormat != VK_FORMAT_UNDEFINED, "VkRenderPassManager: color format is undefined");
        MOBILEGL_ASSERT(m_depthStencilFormat != VK_FORMAT_UNDEFINED,
                        "VkRenderPassManager: depth/stencil format is undefined");

        VkAttachmentDescription color{};
        color.format = m_colorFormat;
        color.samples = VK_SAMPLE_COUNT_1_BIT;
        color.loadOp = colorLoadOp;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthStencil{};
        depthStencil.format = m_depthStencilFormat;
        depthStencil.samples = VK_SAMPLE_COUNT_1_BIT;
        depthStencil.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        depthStencil.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthStencil.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        depthStencil.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthStencil.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencil.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference depthStencilRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;
        subpass.pDepthStencilAttachment = &depthStencilRef;

        VkAttachmentDescription attachments[2] = {color, depthStencil};

        VkRenderPassCreateInfo createInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        createInfo.attachmentCount = 2;
        createInfo.pAttachments = attachments;
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpass;

        VkRenderPass renderPass = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateRenderPass(m_device, &createInfo, nullptr, &renderPass), "vkCreateRenderPass(default)");
        return renderPass;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
