// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "VkClearManager.h"
#include "VkTextureManager.h"
#include "../VkIncludes.h"
#include "../VulkanRendererConfig.h"
#include "MG_State/GLState/FramebufferState/FramebufferObject.h"

#include <Includes.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    struct RenderPassEntry {
        static inline VkDevice s_device;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        // Should we hold pointer-to-resource here?
        Vector<VkTextureManager::TextureResource*> textureResources;
        IntVec2 extent = {0, 0};
        Uint32 subpass = 0;

        RenderPassEntry() = default;
        RenderPassEntry(const RenderPassEntry&) = delete;
        RenderPassEntry(RenderPassEntry&& that) noexcept {
            std::swap(renderPass, that.renderPass);
            std::swap(framebuffer, that.framebuffer);
            std::swap(textureResources, that.textureResources);
            std::swap(extent, that.extent);
            std::swap(subpass, that.subpass);
        }
        RenderPassEntry(
            VkRenderPass renderpass,
            VkFramebuffer framebuffer,
            const std::vector<VkTextureManager::TextureResource*>& textureResources,
            IntVec2 extent, int subpass):
            renderPass(renderpass),
            framebuffer(framebuffer),
            textureResources(Move(textureResources)),
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
    };

    class VkRenderPassManager {
    public:
        using HashType = Uint64;
        VkRenderPassManager(VkDevice device,
            const VulkanRendererConfig& config, VkClearManager& clearManager, VkTextureManager& textureManager);
        ~VkRenderPassManager();

        Bool Initialize();
        void Shutdown();

        HashType ComputeHash(const MG_State::GLState::FramebufferObject& fbo) const;
        RenderPassEntry& GetOrCreateRenderPass(const MG_State::GLState::FramebufferObject& fbo);
        static Bool BeginRenderPass(VkCommandBuffer commandBuffer, RenderPassEntry& renderPassEntry);
        static Bool EndRenderPass(VkCommandBuffer commandBuffer);
    private:
        VkDevice m_device = VK_NULL_HANDLE;
        const VulkanRendererConfig& m_config;
        VkClearManager& m_clearManager;
        VkTextureManager& m_textureManager;
        UnorderedMap<Uint64, RenderPassEntry> m_renderPasses;
        static inline XXH64_state_t* m_hashState = XXH64_createState();
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
