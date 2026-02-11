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
        void Present();

        void RegisterRenderCallback(const String& name, RenderCallback cb);
        void UnregisterRenderCallback(const String& name);

        VkPipeline CreateGraphicsPipelineFromSpv(const String& key, const Vector<uint32_t>& vsSpv,
                                                 const Vector<uint32_t>& fsSpv);

        VkExtent2D GetExtent() const;

        void WaitIdle();

    private:
        NativeWindowType m_window = 0;
        RendererConfig m_config;

        UniquePtr<VulkanContext> m_context;
        UniquePtr<SwapchainManager> m_swapchain;
        UniquePtr<PipelineManager> m_pipelineMgr;

        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        VkCommandPool m_commandPool = VK_NULL_HANDLE;

        Vector<UniquePtr<FrameContext>> m_frames;
        Uint32 m_currentFrame = 0;

        // Render callbacks map
        Vector<std::pair<String, RenderCallback>> m_renderCallbacks;

        // Internals
        void CreateRenderPass();
        void DestroyRenderPass();
        void CreateCommandPool();
        void DestroyCommandPool();
        void CreateFrameResources();
        void DestroyFrameResources();
        void RecordFrameCommandBuffer(FrameContext& frame, uint32_t imageIndex);
        void RecreateSwapchainIfNeeded();
        bool FrameBegin();
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
