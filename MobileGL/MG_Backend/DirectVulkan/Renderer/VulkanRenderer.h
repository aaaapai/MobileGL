// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    class VulkanContext;
    class SwapchainManager;
    class PipelineManager;
    struct FrameContext;

    using RenderCallback = std::function<void(VkCommandBuffer cmdBuf, uint32_t imageIndex, VkExtent2D extent)>;

    struct RendererConfig {
        Uint32 MaxFramesInFlight = 2;
        String AppName = "MobileGL-VulkanRenderer";
    };

    class VulkanRenderer {
    public:
        VulkanRenderer(NativeWindowType window, const RendererConfig& cfg = {});
        ~VulkanRenderer();

        void Initialize();
        void Shutdown();

        void RenderFrame();

        void RegisterRenderCallback(const std::string& name, RenderCallback cb);
        void UnregisterRenderCallback(const std::string& name);

        VkPipeline CreateGraphicsPipelineFromSpv(const std::string& key, const std::vector<uint32_t>& vsSpv,
                                                 const std::vector<uint32_t>& fsSpv);

        VkExtent2D GetExtent() const;

        void WaitIdle();

    private:
        NativeWindowType Window = 0;
        RendererConfig Config;

        std::unique_ptr<VulkanContext> Ctx;
        std::unique_ptr<SwapchainManager> Swapchain;
        std::unique_ptr<PipelineManager> PipelineMgr;

        VkRenderPass RenderPass = VK_NULL_HANDLE;
        VkCommandPool CommandPool = VK_NULL_HANDLE;

        std::vector<std::unique_ptr<FrameContext>> Frames;
        uint32_t CurrentFrame = 0;

        // Render callbacks map
        std::vector<std::pair<std::string, RenderCallback>> RenderCallbacks;

        // Internals
        void CreateRenderPass();
        void DestroyRenderPass();
        void CreateCommandPool();
        void DestroyCommandPool();
        void CreateFrameResources();
        void DestroyFrameResources();
        void RecordFrameCommandBuffer(FrameContext& frame, uint32_t imageIndex);
        void RecreateSwapchainIfNeeded();
    };
} // namespace MobileGL::MG_Backend::DirectVulkan