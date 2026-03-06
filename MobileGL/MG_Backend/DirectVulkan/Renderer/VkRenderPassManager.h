// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "SwapchainObject.h"
#include "VkClearManager.h"
#include "VkTextureManager.h"
#include "../VkIncludes.h"
#include "../VulkanRendererConfig.h"
#include "MG_State/GLState/FramebufferState/FramebufferObject.h"

#include <Includes.h>

namespace MobileGL::MG_Backend::DirectVulkan {

    struct PendingClearAttachmentInfo {
        Uint32 attachmentIndex = 0;
        MG_State::GLState::ITextureObject* texture = nullptr;
    };

    struct TrackedAttachmentLayoutInfo {
        MG_State::GLState::ITextureObject* texture = nullptr;
        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    };

    struct RenderPassEntry {
        static inline VkDevice s_device;
        static inline Vector<VkTextureManager::TextureResource*> s_textureResourcesScratch;
        Uint64 hash = 0;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        Uint64 compatibilityHash = 0;
        Vector<PendingClearAttachmentInfo> pendingClearAttachments;
        Vector<TrackedAttachmentLayoutInfo> trackedAttachmentLayouts;
        Uint32 attachmentCount = 0;
        IntVec2 extent = {0, 0};
        Uint32 subpass = 0;

        RenderPassEntry() = default;
        RenderPassEntry(const RenderPassEntry&) = delete;
        RenderPassEntry(RenderPassEntry&& that) noexcept {
            std::swap(hash, that.hash);
            std::swap(renderPass, that.renderPass);
            std::swap(framebuffer, that.framebuffer);
            std::swap(compatibilityHash, that.compatibilityHash);
            std::swap(pendingClearAttachments, that.pendingClearAttachments);
            std::swap(trackedAttachmentLayouts, that.trackedAttachmentLayouts);
            std::swap(attachmentCount, that.attachmentCount);
            std::swap(extent, that.extent);
            std::swap(subpass, that.subpass);
        }
        RenderPassEntry(
            Uint64 hash,
            VkRenderPass renderpass,
            VkFramebuffer framebuffer,
            Uint64 compatibilityHash,
            const Vector<PendingClearAttachmentInfo>& pendingClearAttachments,
            const Vector<TrackedAttachmentLayoutInfo>& trackedAttachmentLayouts,
            Uint32 attachmentCount,
            IntVec2 extent, int subpass):
            hash(hash),
            renderPass(renderpass),
            framebuffer(framebuffer),
            compatibilityHash(compatibilityHash),
            pendingClearAttachments(Move(pendingClearAttachments)),
            trackedAttachmentLayouts(Move(trackedAttachmentLayouts)),
            attachmentCount(attachmentCount),
            extent(extent),
            subpass(subpass)
        {}

        ~RenderPassEntry() {
            if (renderPass != VK_NULL_HANDLE) {
                vkDestroyRenderPass(s_device, renderPass, nullptr);
            }
            if (framebuffer != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(s_device, framebuffer, nullptr);
            }
        }

        Bool CompatibleWith(const RenderPassEntry& that) const {
            return this->compatibilityHash == that.compatibilityHash;
        }

        Bool CompatibleWith(Uint64 compatibilityHash) const {
            return this->compatibilityHash == compatibilityHash;
        }
    };

    struct ActiveRenderPassInfo {
        Uint64 hash = 0;
        Uint64 compatibilityHash = 0;
        Vector<TrackedAttachmentLayoutInfo> trackedAttachmentLayouts;
        IntVec2 extent = {0, 0};

        Bool CompatibleWith(const RenderPassEntry& that) const {
            return compatibilityHash == that.compatibilityHash;
        }

        Bool CompatibleWith(Uint64 thatCompatibilityHash) const {
            return compatibilityHash == thatCompatibilityHash;
        }
    };

    class VkRenderPassManager {
    public:
        using HashType = Uint64;
        VkRenderPassManager(VkDevice device,
            const VulkanRendererConfig& config, VkClearManager& clearManager, VkTextureManager& textureManager,
            const SwapchainObject& swapchainObject);
        ~VkRenderPassManager();

        Bool Initialize();
        void Shutdown();

        HashType ComputeHash(
            const MG_State::GLState::FramebufferObject& fbo,
            Uint32 swapchainImageIndex,
            Bool includePendingClear = true) const;
        RenderPassEntry& GetOrCreateRenderPass(const MG_State::GLState::FramebufferObject& fbo, Uint32 swapchainImageIndex);
        static Bool BeginRenderPass(VkCommandBuffer commandBuffer, RenderPassEntry& renderPassEntry);
        static Bool EndRenderPass(VkCommandBuffer commandBuffer);
        static ActiveRenderPassInfo* GetActiveRenderPass();
    private:
        VkDevice m_device = VK_NULL_HANDLE;
        const VulkanRendererConfig& m_config;
        VkClearManager& m_clearManager;
        VkTextureManager& m_textureManager;
        const SwapchainObject& m_swapchainObject;
        UnorderedMap<Uint64, RenderPassEntry> m_renderPasses;
        static inline XXH64_state_t* m_hashState = XXH64_createState();
        static inline ActiveRenderPassInfo s_activeRenderPass{};
        static inline Bool s_hasActiveRenderPass = false;
        static inline VkClearManager* s_clearManager = nullptr;
        static inline VkTextureManager* s_textureManager = nullptr;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
