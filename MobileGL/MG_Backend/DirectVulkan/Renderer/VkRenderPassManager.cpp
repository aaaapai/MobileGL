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
        const VulkanRendererConfig& config, VkClearManager& clearManager, VkTextureManager& textureManager,
        SwapchainObject& swapchainObject):
        m_device(device), m_config(config), m_clearManager(clearManager), m_textureManager(textureManager),
        m_swapchainObject(swapchainObject) {
        RenderPassEntry::s_device = m_device;
        s_clearManager = &m_clearManager;
        s_textureManager = &m_textureManager;
        s_swapchainObject = &m_swapchainObject;
    }

    VkRenderPassManager::~VkRenderPassManager() {}

    Bool VkRenderPassManager::Initialize() {
        return true;
    }

    void VkRenderPassManager::Shutdown() {
    }

    VkRenderPassManager::HashType VkRenderPassManager::ComputeHash(
        const MG_State::GLState::FramebufferObject& fbo, Uint32 swapchainImageIndex, Bool includePendingClear) const {
        XXHASH_VERIFY(XXH64_reset(m_hashState, m_config.CacheVersion));
        const Bool isDefaultFbo = (&fbo == MG_Impl::GLImpl::FramebufferImpl::pDefaultFramebufferInfo->defaultFBO.get());
        if (isDefaultFbo) {
            XXHASH_VERIFY(XXH64_update(m_hashState, &swapchainImageIndex, sizeof(swapchainImageIndex)));
        }
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

            if (includePendingClear && att.IsTexture()) {
                auto* texture = att.GetTexture().get();
                auto hasClear = m_clearManager.HasPendingClear(texture);
                XXHASH_VERIFY(XXH64_update(m_hashState, &hasClear, sizeof(hasClear)));
                if (hasClear) {
                    ClearAttachmentPayload clearPayload{};
                    Bool hasPayload = m_clearManager.GetPendingClear(texture, clearPayload);
                    XXHASH_VERIFY(XXH64_update(m_hashState, &hasPayload, sizeof(hasPayload)));
                    if (hasPayload) {
                        XXHASH_VERIFY(XXH64_update(
                            m_hashState, &clearPayload.attachmentType, sizeof(clearPayload.attachmentType)));
                    }
                }

                VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                if (isDefaultFbo) {
                    if (attachment >= FramebufferAttachmentType::Color0 &&
                        attachment <= FramebufferAttachmentType::Color31) {
                        currentLayout = m_swapchainObject.GetImageLayout(swapchainImageIndex);
                    } else if (attachment == FramebufferAttachmentType::Depth ||
                               attachment == FramebufferAttachmentType::Stencil) {
                        currentLayout = m_swapchainObject.GetDepthStencilImageLayout(swapchainImageIndex);
                    }
                } else {
                    auto* resource = m_textureManager.SyncTextureAndGetDescriptor(*texture);
                    if (resource != nullptr) {
                        currentLayout = resource->layout;
                    }
                }
                XXHASH_VERIFY(XXH64_update(m_hashState, &currentLayout, sizeof(currentLayout)));
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

    RenderPassEntry& VkRenderPassManager::GetOrCreateRenderPass(const MG_State::GLState::FramebufferObject& fbo,
                                                                Uint32 swapchainImageIndex) {
        // retrieve from cache first
        auto* activeRenderPass = GetActiveRenderPass();
        auto compatibilityHash = ComputeHash(fbo, swapchainImageIndex, false);
        if (activeRenderPass != nullptr && activeRenderPass->CompatibleWith(compatibilityHash)) {
            auto activeIt = m_renderPasses.find(activeRenderPass->hash);
            MOBILEGL_ASSERT(activeIt != m_renderPasses.end(),
                            "GetOrCreateRenderPass: active render pass hash=0x%llx is missing from cache",
                            static_cast<unsigned long long>(activeRenderPass->hash));
            return activeIt->second;
        }
        auto hash = ComputeHash(fbo, swapchainImageIndex, true);
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
        Vector<VkAttachmentDescription> attachmentDescriptions(validDrawBufCount);
        Vector<VkAttachmentReference> colorAttachmentRefs(validDrawBufCount);
        Vector<PendingClearAttachmentInfo> pendingClearAttachments;
        Vector<TrackedAttachmentLayoutInfo> trackedAttachmentLayouts;
        auto& textureResources = RenderPassEntry::s_textureResourcesScratch;
        textureResources.clear();
        textureResources.resize(validDrawBufCount, nullptr);
        Vector<VkImageView> attachmentViews(validDrawBufCount, VK_NULL_HANDLE);
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
            VkAttachmentDescription& desc = attachmentDescriptions[i];
            switch (textureTarget) {
                case TextureTarget::Texture2D: {
                    auto* texture2d =
                        static_cast<MG_State::GLState::TextureObject2D*>(texture);
                    desc.flags = 0;
                    desc.format =
                        MG_Util::ConvertTextureInternalFormatToVkEnum(
                            texture2d->GetFormat());
                    desc.samples = VK_SAMPLE_COUNT_1_BIT;
                    ClearAttachmentPayload clearPayload{};
                    Bool hasClear = m_clearManager.GetPendingClear(texture, clearPayload);
                    desc.loadOp = hasClear ?
                        VK_ATTACHMENT_LOAD_OP_CLEAR :
                        VK_ATTACHMENT_LOAD_OP_LOAD;
                    desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    desc.finalLayout = isDefaultFbo ?
                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR :
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    if (hasClear) {
                        pendingClearAttachments.emplace_back(PendingClearAttachmentInfo {
                            .attachmentIndex = static_cast<Uint32>(i),
                            .texture = texture
                        });
                    }
                    if (width == 0)
                        width = texture2d->GetBaseSize().x();
                    if (height == 0)
                        height = texture2d->GetBaseSize().y();

                    if (isDefaultFbo) {
                        const auto& swapchainViews = m_swapchainObject.GetImageViews();
                        MOBILEGL_ASSERT(swapchainImageIndex < swapchainViews.size(),
                                        "GetOrCreateRenderPass: swapchain image index out of range");
                        desc.initialLayout = hasClear ?
                            VK_IMAGE_LAYOUT_UNDEFINED :
                            m_swapchainObject.GetImageLayout(swapchainImageIndex);
                        trackedAttachmentLayouts.emplace_back(TrackedAttachmentLayoutInfo {
                            .target = TrackedAttachmentTarget::SwapchainColor,
                            .swapchainImageIndex = swapchainImageIndex,
                            .finalLayout = desc.finalLayout,
                        });
                        attachmentViews[i] = swapchainViews[swapchainImageIndex];
                    } else {
                        textureResources[i] = m_textureManager.SyncTextureAndGetDescriptor(*texture);
                        MOBILEGL_ASSERT(textureResources[i],
                                        "GetOrCreateRenderPass: SyncTextureAndGetDescriptor failed at color attachment %d", i);
                        MOBILEGL_ASSERT(hasClear || textureResources[i]->layout != VK_IMAGE_LAYOUT_UNDEFINED,
                                        "GetOrCreateRenderPass: color attachment textureId=%d has undefined tracked layout with LOAD_OP_LOAD (attachment=%d)",
                                        texture->GetExternalIndex(), i);
                        desc.initialLayout = hasClear ?
                            VK_IMAGE_LAYOUT_UNDEFINED :
                            textureResources[i]->layout;
                        trackedAttachmentLayouts.emplace_back(TrackedAttachmentLayoutInfo {
                            .target = TrackedAttachmentTarget::Texture,
                            .texture = texture,
                            .finalLayout = desc.finalLayout,
                        });
                        attachmentViews[i] = textureResources[i]->view;
                    }

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
        auto& depthAtt = fbo.GetAttachment(FramebufferAttachmentType::Depth);
        VkAttachmentDescription depthAttachmentDescription;
        VkAttachmentReference depthAttachmentRef;
        depthAttachmentRef.attachment = VK_ATTACHMENT_UNUSED;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkTextureManager::TextureResource* depthTextureResource = nullptr;
        if (depthAtt.IsComplete() && depthAtt.IsTexture()) {
            auto& texture = *depthAtt.GetTexture();
            const Uint32 depthAttachmentIndex = static_cast<Uint32>(attachmentDescriptions.size());
            ClearAttachmentPayload clearPayload{};
            Bool hasClear = m_clearManager.GetPendingClear(&texture, clearPayload);
            Bool clearDepth = hasClear && clearPayload.attachmentType == FramebufferAttachmentType::Depth;
            Bool clearStencil = hasClear && clearPayload.attachmentType == FramebufferAttachmentType::Stencil;
            VkImageLayout trackedDepthLayout = isDefaultFbo ?
                m_swapchainObject.GetDepthStencilImageLayout(swapchainImageIndex) :
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            if (!isDefaultFbo) {
                depthTextureResource = m_textureManager.SyncTextureAndGetDescriptor(texture);
                MOBILEGL_ASSERT(depthTextureResource,
                                "GetOrCreateRenderPass: SyncTextureAndGetDescriptor failed at depth attachment");
                trackedDepthLayout = depthTextureResource->layout;
            }
            depthAttachmentDescription.flags = 0;
            depthAttachmentDescription.format =
                MG_Util::ConvertTextureInternalFormatToVkEnum(texture.GetFormat());
            depthAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachmentDescription.loadOp = clearDepth ?
                VK_ATTACHMENT_LOAD_OP_CLEAR :
                VK_ATTACHMENT_LOAD_OP_LOAD;
            depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            depthAttachmentDescription.stencilLoadOp = clearStencil ?
                VK_ATTACHMENT_LOAD_OP_CLEAR :
                VK_ATTACHMENT_LOAD_OP_LOAD;
            depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
            depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            if (trackedDepthLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                if (!clearDepth) {
                    depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                }
                if (!clearStencil) {
                    depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                }
            }
            depthAttachmentDescription.initialLayout =
                (clearDepth && clearStencil) || trackedDepthLayout == VK_IMAGE_LAYOUT_UNDEFINED ?
                    VK_IMAGE_LAYOUT_UNDEFINED :
                    trackedDepthLayout;
            if (hasClear) {
                pendingClearAttachments.emplace_back(PendingClearAttachmentInfo {
                    .attachmentIndex = depthAttachmentIndex,
                    .texture = &texture
                });
            }
            if (isDefaultFbo) {
                trackedAttachmentLayouts.emplace_back(TrackedAttachmentLayoutInfo {
                    .target = TrackedAttachmentTarget::SwapchainDepthStencil,
                    .swapchainImageIndex = swapchainImageIndex,
                    .finalLayout = depthAttachmentDescription.finalLayout,
                });
                attachmentViews.emplace_back(m_swapchainObject.GetDepthStencilImageView(swapchainImageIndex));
            } else {
                MOBILEGL_ASSERT(clearDepth || clearStencil || depthTextureResource->layout != VK_IMAGE_LAYOUT_UNDEFINED,
                                "GetOrCreateRenderPass: depth attachment textureId=%d has undefined tracked layout with LOAD_OP_LOAD",
                                texture.GetExternalIndex());
                trackedAttachmentLayouts.emplace_back(TrackedAttachmentLayoutInfo {
                    .target = TrackedAttachmentTarget::Texture,
                    .texture = &texture,
                    .finalLayout = depthAttachmentDescription.finalLayout,
                });
                textureResources.emplace_back(depthTextureResource);
                attachmentViews.emplace_back(depthTextureResource->view);
                if (width == 0 || height == 0) {
                    auto texture2d = static_cast<MG_State::GLState::TextureObject2D*>(&texture);
                    width = texture2d->GetBaseSize().x();
                    height = texture2d->GetBaseSize().y();
                }
            }
            attachmentDescriptions.emplace_back(depthAttachmentDescription);
            depthAttachmentRef.attachment = depthAttachmentIndex;
        }

        // Subpass
        VkSubpassDescription subpassDesc;
        subpassDesc.flags = 0;
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.inputAttachmentCount = 0;
        subpassDesc.pInputAttachments = nullptr;
        subpassDesc.colorAttachmentCount = colorAttachmentRefs.size();
        subpassDesc.pColorAttachments = colorAttachmentRefs.data();
        subpassDesc.pResolveAttachments = nullptr;
        subpassDesc.pDepthStencilAttachment = depthAtt.IsComplete() ? &depthAttachmentRef : VK_NULL_HANDLE;
        subpassDesc.preserveAttachmentCount = 0;
        subpassDesc.pPreserveAttachments = nullptr;

        // Render Pass
        VkRenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.pNext = VK_NULL_HANDLE;
        renderPassCreateInfo.flags = 0;
        renderPassCreateInfo.attachmentCount = attachmentDescriptions.size();
        renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDesc;
        renderPassCreateInfo.dependencyCount = 0;
        renderPassCreateInfo.pDependencies = nullptr;

        VkRenderPass renderPass = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr, &renderPass));

        // Framebuffer
        VkFramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.pNext = nullptr;
        framebufferCreateInfo.flags = 0;
        framebufferCreateInfo.renderPass = renderPass;
        framebufferCreateInfo.attachmentCount = attachmentViews.size();
        framebufferCreateInfo.pAttachments = attachmentViews.data();
        framebufferCreateInfo.width = width;
        framebufferCreateInfo.height = height;
        framebufferCreateInfo.layers = 1;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateFramebuffer(m_device, &framebufferCreateInfo, nullptr, &framebuffer));
        IntVec2 extent = {width, height};
        RenderPassEntry renderPassEntry {
            hash,
            renderPass,
            framebuffer,
            compatibilityHash,
            Move(pendingClearAttachments),
            Move(trackedAttachmentLayouts),
            static_cast<Uint32>(attachmentViews.size()),
            extent,
            1 };
        auto [insertedIt, _] = m_renderPasses.emplace(hash, Move(renderPassEntry));
        return insertedIt->second;
    }

    Bool VkRenderPassManager::BeginRenderPass(VkCommandBuffer commandBuffer, RenderPassEntry& renderPassEntry) {
        // TODO: Transition all the attachments into proper layout before starting the render pass
        VkRenderPassBeginInfo renderPassBeginInfo;
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.pNext = nullptr;
        renderPassBeginInfo.renderPass = renderPassEntry.renderPass;
        renderPassBeginInfo.framebuffer = renderPassEntry.framebuffer;
        renderPassBeginInfo.renderArea.offset = { 0, 0 };
        renderPassBeginInfo.renderArea.extent = {
            (Uint32)renderPassEntry.extent.x(), (Uint32)renderPassEntry.extent.y() };

        Vector<VkClearValue> clearValues(renderPassEntry.attachmentCount);
        for (auto& clearValue: clearValues) {
            clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
            clearValue.depthStencil = {1.0f, 0};
        }
        for (const auto& pending: renderPassEntry.pendingClearAttachments) {
            if (!pending.texture || pending.attachmentIndex >= clearValues.size()) {
                continue;
            }
            ClearAttachmentPayload clearPayload{};
            if (!s_clearManager->GetPendingClear(pending.texture, clearPayload)) {
                continue;
            }
            if (clearPayload.attachmentType >= FramebufferAttachmentType::Color0 &&
                clearPayload.attachmentType <= FramebufferAttachmentType::Color31) {
                clearValues[pending.attachmentIndex].color = {
                    clearPayload.color.x(),
                    clearPayload.color.y(),
                    clearPayload.color.z(),
                    clearPayload.color.w()
                };
            } else if (clearPayload.attachmentType == FramebufferAttachmentType::Depth) {
                clearValues[pending.attachmentIndex].depthStencil.depth = clearPayload.depth;
            } else if (clearPayload.attachmentType == FramebufferAttachmentType::Stencil) {
                clearValues[pending.attachmentIndex].depthStencil.stencil = clearPayload.stencil;
            }
        }

        renderPassBeginInfo.clearValueCount = static_cast<Uint32>(clearValues.size());
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        for (const auto& pending: renderPassEntry.pendingClearAttachments) {
            s_clearManager->PopPendingClear(pending.texture);
        }
        s_activeRenderPass.hash = renderPassEntry.hash;
        s_activeRenderPass.compatibilityHash = renderPassEntry.compatibilityHash;
        s_activeRenderPass.trackedAttachmentLayouts = renderPassEntry.trackedAttachmentLayouts;
        s_activeRenderPass.extent = renderPassEntry.extent;
        s_hasActiveRenderPass = true;

        return true;
    }

    Bool VkRenderPassManager::EndRenderPass(VkCommandBuffer commandBuffer) {
        auto* activeRenderPass = GetActiveRenderPass();
        vkCmdEndRenderPass(commandBuffer);
        if (activeRenderPass != nullptr && !activeRenderPass->trackedAttachmentLayouts.empty()) {
            for (const auto& trackedAttachment : activeRenderPass->trackedAttachmentLayouts) {
                switch (trackedAttachment.target) {
                    case TrackedAttachmentTarget::Texture:
                        MOBILEGL_ASSERT(s_textureManager != nullptr, "EndRenderPass: texture manager is null");
                        s_textureManager->UpdateTrackedImageLayout(trackedAttachment.texture, trackedAttachment.finalLayout);
                        break;
                    case TrackedAttachmentTarget::SwapchainColor:
                        MOBILEGL_ASSERT(s_swapchainObject != nullptr, "EndRenderPass: swapchain object is null");
                        s_swapchainObject->SetImageLayout(trackedAttachment.swapchainImageIndex, trackedAttachment.finalLayout);
                        break;
                    case TrackedAttachmentTarget::SwapchainDepthStencil:
                        MOBILEGL_ASSERT(s_swapchainObject != nullptr, "EndRenderPass: swapchain object is null");
                        s_swapchainObject->SetDepthStencilImageLayout(trackedAttachment.swapchainImageIndex,
                                                                      trackedAttachment.finalLayout);
                        break;
                    default:
                        MOBILEGL_ASSERT(false, "EndRenderPass: unsupported tracked attachment target=%d",
                                        static_cast<Int>(trackedAttachment.target));
                        break;
                }
            }
        }
        s_activeRenderPass = {};
        s_hasActiveRenderPass = false;
        return true;
    }

    ActiveRenderPassInfo* VkRenderPassManager::GetActiveRenderPass() {
        return s_hasActiveRenderPass ? &s_activeRenderPass : nullptr;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
