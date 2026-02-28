// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VkRenderPassManager.h"

#include "MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.h"
#include "MG_State/GLState/TextureState/TextureObject2D.h"
#include "MG_Util/Converters/MGToVk/TextureEnumConverter.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    VkRenderPassManager::VkRenderPassManager(VkDevice device,
        const VulkanRendererConfig& config, VkClearManager& clearManager, VkTextureManager& textureManager):
        m_device(device), m_config(config), m_clearManager(clearManager), m_textureManager(textureManager) {
        RenderPassEntry::s_device = m_device;
    }

    VkRenderPassManager::~VkRenderPassManager() {}

    Bool VkRenderPassManager::Initialize() {
        return true;
    }

    void VkRenderPassManager::Shutdown() {
    }

    VkRenderPassManager::HashType VkRenderPassManager::ComputeHash(
        const MG_State::GLState::FramebufferObject& fbo) const {
        XXHASH_VERIFY(XXH64_reset(m_hashState, m_config.CacheVersion));
        auto& drawBuffers = fbo.GetDrawBuffers();
        XXHASH_VERIFY(XXH64_update(m_hashState, drawBuffers.data(), drawBuffers.size() * sizeof(drawBuffers[0])));
        auto readBuffer = fbo.GetReadBuffer();
        XXHASH_VERIFY(XXH64_update(m_hashState, &readBuffer, sizeof(FramebufferAttachmentType)));
        Int validDrawBufCount = 0;
        for (Int i = 0; i < drawBuffers.size(); ++i) {
            auto drawbuf = drawBuffers[i];
            if (drawbuf != FramebufferAttachmentType::None)
                validDrawBufCount = std::max(validDrawBufCount, i + 1);
        }
        XXHASH_VERIFY(XXH64_update(m_hashState, &validDrawBufCount, sizeof(validDrawBufCount)));

        auto combineFramebufferAttachmentObjHash = [&](FramebufferAttachmentType attachment) {
            auto& att = fbo.GetAttachment(attachment);

            Int type = 0;
            if (att.IsEmpty()) type = 0;
            else if (att.IsTexture()) type = 1;
            else if (att.IsRenderbuffer()) type = 2;
            XXHASH_VERIFY(XXH64_update(m_hashState, &type, sizeof(type)));
            void* contentPtr = nullptr;
            if (att.IsTexture())
                contentPtr = att.GetTexture().get();
            else if (att.IsRenderbuffer())
                contentPtr = att.GetRenderbuffer().get();
            XXHASH_VERIFY(XXH64_update(m_hashState, &contentPtr, sizeof(contentPtr)));

            if (att.IsTexture()) {
                auto hasClear = m_clearManager.HasPendingClear(att.GetTexture().get());
                XXHASH_VERIFY(XXH64_update(m_hashState, &hasClear, sizeof(hasClear)));
            }
        };

        for (Int i = 0; i < validDrawBufCount; ++i) {
            auto drawbuf = drawBuffers[i];
            combineFramebufferAttachmentObjHash(drawbuf);
        }

        combineFramebufferAttachmentObjHash(FramebufferAttachmentType::Depth);
        combineFramebufferAttachmentObjHash(FramebufferAttachmentType::Stencil);

        return XXH64_digest(m_hashState);
    }

    RenderPassEntry& VkRenderPassManager::GetOrCreateRenderPass(const MG_State::GLState::FramebufferObject& fbo) {
        // retrieve from cache first
        auto hash = ComputeHash(fbo);
        auto it = m_renderPasses.find(hash);
        if (it != m_renderPasses.end())
            return it->second;

        Bool isDefaultFbo = (&fbo == MG_Impl::GLImpl::FramebufferImpl::pDefaultFramebufferInfo->defaultFBO.get());
        // Color attachment
        auto& drawbufs = fbo.GetDrawBuffers();
        Int validDrawBufCount = 0;
        for (Int i = 0; i < drawbufs.size(); ++i) {
            auto drawbuf = drawbufs[i];
            if (drawbuf != FramebufferAttachmentType::None)
                validDrawBufCount = std::max(validDrawBufCount, i + 1);
        }

        Int width = 0;
        Int height = 0;
        Vector<VkAttachmentDescription> colorAttachmentDescriptions(validDrawBufCount);
        Vector<VkAttachmentReference> colorAttachmentRefs(validDrawBufCount);
        Vector<VkDescriptorImageInfo> descriptorImageInfo(validDrawBufCount);
        // This should automatically work on default & offscreen FBO
        // assuming default FBO has the right param
        for (Int i = 0; i < validDrawBufCount; ++i) {
            auto drawbuf = drawbufs[i];
            if (drawbuf == FramebufferAttachmentType::None ||
                fbo.GetAttachment(drawbuf).IsRenderbuffer())
                continue;

            auto& att = fbo.GetAttachment(drawbuf);
            auto* texture = att.GetTexture().get();
            const auto textureTarget = texture->GetTarget();

            // Color attachment description
            VkAttachmentDescription& desc = colorAttachmentDescriptions[i];
            switch (textureTarget) {
                case TextureTarget::Texture2D: {
                    auto* texture2d =
                        static_cast<MG_State::GLState::TextureObject2D*>(texture);
                    desc.format =
                        MG_Util::ConvertTextureInternalFormatToVkEnum(
                            texture2d->GetFormat());
                    desc.samples = VK_SAMPLE_COUNT_1_BIT;
                    Bool hasClear = m_clearManager.HasPendingClear(texture);
                    desc.loadOp = hasClear ?
                        VK_ATTACHMENT_LOAD_OP_CLEAR :
                        VK_ATTACHMENT_LOAD_OP_LOAD;
                    desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    desc.finalLayout = isDefaultFbo ?
                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR :
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    if (width == 0)
                        width = texture2d->GetBaseSize().x();
                    if (height == 0)
                        height = texture2d->GetBaseSize().y();

                    Bool ok = m_textureManager.SyncTextureAndGetDescriptor(*texture, descriptorImageInfo[i]);
                    MOBILEGL_ASSERT(ok, "GetOrCreateRenderPass: SyncTextureAndGetDescriptor failed");

                    break;
                }
                default:
                    MOBILEGL_ASSERT(false, "Unsupported texture target");
            }

            // Attachment reference
            VkAttachmentReference& attachmentRef = colorAttachmentRefs[i];
            attachmentRef.attachment = i;
            attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }



        // Depth attachment description
        VkAttachmentDescription depthAttachmentDescription;
        auto& depthAtt = fbo.GetAttachment(FramebufferAttachmentType::Depth);
        depthAttachmentDescription.format =
            MG_Util::ConvertTextureInternalFormatToVkEnum(depthAtt.GetTexture()->GetFormat());
        depthAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachmentDescription.finalLayout = isDefaultFbo ?
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR :
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Depth attachment ref
        VkAttachmentReference depthAttachmentRef;
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Subpass
        VkSubpassDescription subpassDesc;
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.colorAttachmentCount = colorAttachmentRefs.size();
        subpassDesc.pColorAttachments = colorAttachmentRefs.data();
        subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;

        // Render Pass
        VkRenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = colorAttachmentDescriptions.size();
        renderPassCreateInfo.pAttachments = colorAttachmentDescriptions.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDesc;

        VkRenderPass renderPass = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr, &renderPass));

        Vector<VkImageView> attachmentViews(descriptorImageInfo.size(), VK_NULL_HANDLE);
        for (Int i = 0; i < descriptorImageInfo.size(); i++) {
            attachmentViews[i] = descriptorImageInfo[i].imageView;
        }

        // Framebuffer
        VkFramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = renderPass;
        framebufferCreateInfo.attachmentCount = attachmentViews.size();
        framebufferCreateInfo.pAttachments = attachmentViews.data();
        framebufferCreateInfo.width = width;
        framebufferCreateInfo.height = height;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateFramebuffer(m_device, &framebufferCreateInfo, nullptr, &framebuffer));
        IntVec2 extent = {width, height};
        m_renderPasses[hash] = { renderPass, framebuffer, Move(descriptorImageInfo), extent };
        return m_renderPasses[hash];
    }

    Bool VkRenderPassManager::StartRenderPass(VkCommandBuffer commandBuffer, RenderPassEntry& renderPassEntry) {
        if (m_activeRenderPass != nullptr)
            return false;

        m_activeRenderPass = &renderPassEntry;
        VkRenderPassBeginInfo renderPassBeginInfo;
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = m_activeRenderPass->renderPass;
        renderPassBeginInfo.framebuffer = m_activeRenderPass->framebuffer;
        renderPassBeginInfo.renderArea.offset = { 0, 0 };
        renderPassBeginInfo.renderArea.extent = {
            (Uint32)m_activeRenderPass->extent.x(), (Uint32)m_activeRenderPass->extent.y() };

        // TODO: should query proper clear color
        VkClearValue clearValue;
        clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValue.depthStencil = { 1.0f, 0 };
        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        return true;
    }

    Bool VkRenderPassManager::EndRenderPass(VkCommandBuffer commandBuffer) {
        if (m_activeRenderPass == nullptr)
            return false;
        vkCmdEndRenderPass(commandBuffer);
        return true;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
