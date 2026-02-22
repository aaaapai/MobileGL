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
        DestroyDefaultFramebuffers();
        for (auto& [_, target] : m_offscreenRenderTargets) {
            DestroyOffscreenRenderTarget(target);
        }
        m_offscreenRenderTargets.clear();

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
        m_defaultExtent = {0, 0};
        m_device = VK_NULL_HANDLE;
    }

    Bool VkRenderPassManager::RecreateDefaultFramebuffers(const Vector<VkImageView>& colorViews,
                                                          const Vector<VkImageView>& depthStencilViews,
                                                          VkExtent2D extent) {
        DestroyDefaultFramebuffers();

        if (m_device == VK_NULL_HANDLE || m_renderPassLoad == VK_NULL_HANDLE || extent.width == 0 ||
            extent.height == 0 || colorViews.empty() || colorViews.size() != depthStencilViews.size()) {
            return false;
        }

        m_defaultFramebuffers.reserve(colorViews.size());
        for (SizeT i = 0; i < colorViews.size(); ++i) {
            if (colorViews[i] == VK_NULL_HANDLE || depthStencilViews[i] == VK_NULL_HANDLE) {
                DestroyDefaultFramebuffers();
                return false;
            }
            VkImageView attachments[] = {colorViews[i], depthStencilViews[i]};
            VkFramebufferCreateInfo createInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            createInfo.renderPass = m_renderPassLoad;
            createInfo.attachmentCount = static_cast<Uint32>(std::size(attachments));
            createInfo.pAttachments = attachments;
            createInfo.width = extent.width;
            createInfo.height = extent.height;
            createInfo.layers = 1;
            VkFramebuffer framebuffer = VK_NULL_HANDLE;
            if (vkCreateFramebuffer(m_device, &createInfo, nullptr, &framebuffer) != VK_SUCCESS) {
                DestroyDefaultFramebuffers();
                return false;
            }
            m_defaultFramebuffers.push_back(framebuffer);
        }
        m_defaultExtent = extent;
        return true;
    }

    Bool VkRenderPassManager::GetDefaultRenderTarget(Uint32 imageIndex, VkRenderPass& outRenderPass,
                                                     VkFramebuffer& outFramebuffer, VkExtent2D& outExtent,
                                                     VkFormat& outDepthStencilFormat) const {
        if (imageIndex >= m_defaultFramebuffers.size() || m_renderPassLoad == VK_NULL_HANDLE) {
            return false;
        }
        outRenderPass = m_renderPassLoad;
        outFramebuffer = m_defaultFramebuffers[imageIndex];
        outExtent = m_defaultExtent;
        outDepthStencilFormat = m_depthStencilFormat;
        return outFramebuffer != VK_NULL_HANDLE;
    }

    Bool VkRenderPassManager::EnsureOffscreenRenderTarget(const OffscreenRenderTargetInfo& targetInfo) {
        if (m_device == VK_NULL_HANDLE || targetInfo.colorView == VK_NULL_HANDLE ||
            targetInfo.colorFormat == VK_FORMAT_UNDEFINED || targetInfo.extent.width == 0 ||
            targetInfo.extent.height == 0) {
            return false;
        }

        const Bool hasDepthStencil =
            targetInfo.depthStencilView != VK_NULL_HANDLE && targetInfo.depthStencilFormat != VK_FORMAT_UNDEFINED;

        auto& target = m_offscreenRenderTargets[targetInfo.targetExternalIndex];
        if (target.framebuffer != VK_NULL_HANDLE && target.renderPassLoad != VK_NULL_HANDLE &&
            target.targetVersion == targetInfo.targetVersion && target.colorView == targetInfo.colorView &&
            target.colorFormat == targetInfo.colorFormat && target.depthStencilView == targetInfo.depthStencilView &&
            target.depthStencilFormat == targetInfo.depthStencilFormat &&
            target.extent.width == targetInfo.extent.width && target.extent.height == targetInfo.extent.height) {
            return true;
        }

        DestroyOffscreenRenderTarget(target);

        const VkFormat depthStencilFormat = hasDepthStencil ? targetInfo.depthStencilFormat : VK_FORMAT_UNDEFINED;
        target.renderPassLoad = CreateRenderPass(targetInfo.colorFormat, depthStencilFormat, VK_ATTACHMENT_LOAD_OP_LOAD,
                                                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        if (target.renderPassLoad == VK_NULL_HANDLE) {
            return false;
        }

        Array<VkImageView, 2> attachments{};
        attachments[0] = targetInfo.colorView;
        Uint32 attachmentCount = 1;
        if (hasDepthStencil) {
            attachments[1] = targetInfo.depthStencilView;
            attachmentCount = 2;
        }

        VkFramebufferCreateInfo framebufferInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        framebufferInfo.renderPass = target.renderPassLoad;
        framebufferInfo.attachmentCount = attachmentCount;
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = targetInfo.extent.width;
        framebufferInfo.height = targetInfo.extent.height;
        framebufferInfo.layers = 1;
        VK_VERIFY(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &target.framebuffer),
                  "vkCreateFramebuffer(offscreen)");

        target.targetVersion = targetInfo.targetVersion;
        target.colorView = targetInfo.colorView;
        target.colorFormat = targetInfo.colorFormat;
        target.depthStencilView = targetInfo.depthStencilView;
        target.depthStencilFormat = targetInfo.depthStencilFormat;
        target.extent = targetInfo.extent;
        return true;
    }

    Bool VkRenderPassManager::GetOffscreenRenderTarget(Uint targetExternalIndex, VkRenderPass& outRenderPass,
                                                       VkFramebuffer& outFramebuffer, VkExtent2D& outExtent,
                                                       VkFormat& outDepthStencilFormat) const {
        auto it = m_offscreenRenderTargets.find(targetExternalIndex);
        if (it == m_offscreenRenderTargets.end() || it->second.renderPassLoad == VK_NULL_HANDLE ||
            it->second.framebuffer == VK_NULL_HANDLE) {
            return false;
        }

        outRenderPass = it->second.renderPassLoad;
        outFramebuffer = it->second.framebuffer;
        outExtent = it->second.extent;
        outDepthStencilFormat = it->second.depthStencilFormat;
        return true;
    }

    void VkRenderPassManager::RemoveOffscreenRenderTarget(Uint targetExternalIndex) {
        auto it = m_offscreenRenderTargets.find(targetExternalIndex);
        if (it == m_offscreenRenderTargets.end()) {
            return;
        }
        DestroyOffscreenRenderTarget(it->second);
        m_offscreenRenderTargets.erase(it);
    }

    void VkRenderPassManager::BeginRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass,
                                              VkFramebuffer framebuffer, VkExtent2D extent) const {
        VkRenderPassBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.renderPass = renderPass;
        beginInfo.framebuffer = framebuffer;
        beginInfo.renderArea.offset = {0, 0};
        beginInfo.renderArea.extent = extent;
        beginInfo.clearValueCount = 0;
        beginInfo.pClearValues = nullptr;
        vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VkRenderPassManager::EndRenderPass(VkCommandBuffer commandBuffer) const {
        vkCmdEndRenderPass(commandBuffer);
    }

    void VkRenderPassManager::RecordColorClear(VkCommandBuffer commandBuffer, VkExtent2D extent,
                                               const VkClearColorValue& clearColor) const {
        VkClearAttachment clearAttachment{};
        clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clearAttachment.colorAttachment = 0;
        clearAttachment.clearValue.color = clearColor;

        VkClearRect clearRect{};
        clearRect.rect.offset = {0, 0};
        clearRect.rect.extent = extent;
        clearRect.baseArrayLayer = 0;
        clearRect.layerCount = 1;

        vkCmdClearAttachments(commandBuffer, 1, &clearAttachment, 1, &clearRect);
    }

    void VkRenderPassManager::RecordDepthStencilClear(VkCommandBuffer commandBuffer, VkExtent2D extent, GLbitfield mask,
                                                      Float depth, Uint32 stencil, VkFormat depthStencilFormat) const {
        if (depthStencilFormat == VK_FORMAT_UNDEFINED) {
            return;
        }

        VkImageAspectFlags aspectMask = 0;
        if ((mask & GL_DEPTH_BUFFER_BIT) != 0) {
            aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        if ((mask & GL_STENCIL_BUFFER_BIT) != 0 && HasStencilComponent(depthStencilFormat)) {
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        if (aspectMask == 0) {
            return;
        }

        VkClearAttachment clearAttachment{};
        clearAttachment.aspectMask = aspectMask;
        clearAttachment.clearValue.depthStencil.depth = depth;
        clearAttachment.clearValue.depthStencil.stencil = stencil;

        VkClearRect clearRect{};
        clearRect.rect.offset = {0, 0};
        clearRect.rect.extent = extent;
        clearRect.baseArrayLayer = 0;
        clearRect.layerCount = 1;

        vkCmdClearAttachments(commandBuffer, 1, &clearAttachment, 1, &clearRect);
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
        return CreateRenderPass(m_colorFormat, m_depthStencilFormat, colorLoadOp, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }

    VkRenderPass VkRenderPassManager::CreateRenderPass(VkFormat colorFormat, VkFormat depthStencilFormat,
                                                       VkAttachmentLoadOp colorLoadOp,
                                                       VkImageLayout colorFinalLayout) const {
        VkAttachmentDescription color{};
        color.format = colorFormat;
        color.samples = VK_SAMPLE_COUNT_1_BIT;
        color.loadOp = colorLoadOp;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.finalLayout = colorFinalLayout;
        color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        const Bool hasDepthStencil = depthStencilFormat != VK_FORMAT_UNDEFINED;
        VkAttachmentDescription depthStencil{};
        if (hasDepthStencil) {
            depthStencil.format = depthStencilFormat;
            depthStencil.samples = VK_SAMPLE_COUNT_1_BIT;
            depthStencil.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            depthStencil.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthStencil.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            depthStencil.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthStencil.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthStencil.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkAttachmentReference colorRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference depthStencilRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;
        if (hasDepthStencil) {
            subpass.pDepthStencilAttachment = &depthStencilRef;
        }

        Array<VkAttachmentDescription, 2> attachments{};
        attachments[0] = color;
        Uint32 attachmentCount = 1;
        if (hasDepthStencil) {
            attachments[1] = depthStencil;
            attachmentCount = 2;
        }

        VkRenderPassCreateInfo createInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        createInfo.attachmentCount = attachmentCount;
        createInfo.pAttachments = attachments.data();
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpass;

        VkRenderPass renderPass = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateRenderPass(m_device, &createInfo, nullptr, &renderPass), "vkCreateRenderPass");
        return renderPass;
    }

    void VkRenderPassManager::DestroyDefaultFramebuffers() {
        for (auto framebuffer : m_defaultFramebuffers) {
            if (framebuffer != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(m_device, framebuffer, nullptr);
            }
        }
        m_defaultFramebuffers.clear();
        m_defaultExtent = {0, 0};
    }

    void VkRenderPassManager::DestroyOffscreenRenderTarget(OffscreenRenderTarget& target) {
        if (target.framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(m_device, target.framebuffer, nullptr);
            target.framebuffer = VK_NULL_HANDLE;
        }
        if (target.renderPassLoad != VK_NULL_HANDLE) {
            vkDestroyRenderPass(m_device, target.renderPassLoad, nullptr);
            target.renderPassLoad = VK_NULL_HANDLE;
        }
        target.targetVersion = 0;
        target.colorView = VK_NULL_HANDLE;
        target.colorFormat = VK_FORMAT_UNDEFINED;
        target.depthStencilView = VK_NULL_HANDLE;
        target.depthStencilFormat = VK_FORMAT_UNDEFINED;
        target.extent = {0, 0};
    }

    Bool VkRenderPassManager::HasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
