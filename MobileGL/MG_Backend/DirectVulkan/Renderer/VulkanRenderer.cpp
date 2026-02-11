// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VulkanRenderer.h"
#include "VulkanContext.h"
#include "SwapchainManager.h"
#include "PipelineManager.h"
#include "FrameContext.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    VulkanRenderer::VulkanRenderer(NativeWindowType window, const RendererConfig& cfg) : m_window(window), m_config(cfg) {
        m_context = std::make_unique<VulkanContext>();
    }

    VulkanRenderer::~VulkanRenderer() {
        Shutdown();
    }

    void VulkanRenderer::Initialize() {
        m_context->Initialize(m_window, m_config.AppName);

        m_swapchain = std::make_unique<SwapchainManager>(*m_context);
        m_swapchain->Initialize();

        CreateRenderPass();

        m_pipelineMgr = std::make_unique<PipelineManager>(*m_context);

        CreateCommandPool();
        CreateFrameResources();

        MGLOG_D("VulkanRenderer initialized");
    }

    void VulkanRenderer::Shutdown() {
        if (!m_context) return;

        if (m_context->GetDevice() == VK_NULL_HANDLE) return;

        vkDeviceWaitIdle(m_context->GetDevice());

        DestroyFrameResources();
        DestroyCommandPool();

        if (m_pipelineMgr) {
            m_pipelineMgr->Cleanup();
            m_pipelineMgr.reset();
        }
        DestroyRenderPass();
        if (m_swapchain) {
            m_swapchain->Cleanup();
            m_swapchain.reset();
        }
        if (m_context) {
            m_context->Shutdown();
            m_context.reset();
        }
        MGLOG_D("VulkanRenderer shutdown");
    }

    void VulkanRenderer::CreateRenderPass() {
        VkAttachmentDescription color{};
        color.format = m_swapchain->GetFormat();
        color.samples = VK_SAMPLE_COUNT_1_BIT;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkSubpassDescription sub{};
        sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sub.colorAttachmentCount = 1;
        sub.pColorAttachments = &colorRef;

        VkRenderPassCreateInfo rpci{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        rpci.attachmentCount = 1;
        rpci.pAttachments = &color;
        rpci.subpassCount = 1;
        rpci.pSubpasses = &sub;

        VK_VERIFY(vkCreateRenderPass(m_context->GetDevice(), &rpci, nullptr, &m_renderPass), "vkCreateRenderPass");

        // Create framebuffers now (use swapchain imageviews)
        const auto& imageViews = m_swapchain->GetImageViews();
        std::vector<VkFramebuffer> fbs;
        fbs.reserve(imageViews.size());
        for (auto iv : imageViews) {
            VkImageView attachments[] = {iv};
            VkFramebufferCreateInfo fbci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            fbci.renderPass = m_renderPass;
            fbci.attachmentCount = 1;
            fbci.pAttachments = attachments;
            fbci.width = m_swapchain->GetExtent().width;
            fbci.height = m_swapchain->GetExtent().height;
            fbci.layers = 1;
            VkFramebuffer fb;
            VK_VERIFY(vkCreateFramebuffer(m_context->GetDevice(), &fbci, nullptr, &fb), "vkCreateFramebuffer");
            fbs.push_back(fb);
        }
        m_swapchain->SetFramebuffers(std::move(fbs));
        MGLOG_D("RenderPass created and framebuffers set");
    }

    void VulkanRenderer::DestroyRenderPass() {
        if (m_renderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(m_context->GetDevice(), m_renderPass, nullptr);
            m_renderPass = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::CreateCommandPool() {
        VkCommandPoolCreateInfo cpci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        cpci.queueFamilyIndex = m_context->GetGraphicsQueueFamily();
        cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VK_VERIFY(vkCreateCommandPool(m_context->GetDevice(), &cpci, nullptr, &m_commandPool), "vkCreateCommandPool");
    }

    void VulkanRenderer::DestroyCommandPool() {
        if (m_commandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(m_context->GetDevice(), m_commandPool, nullptr);
            m_commandPool = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::CreateFrameResources() {
        uint32_t imageCount = static_cast<uint32_t>(m_swapchain->GetImageViews().size());
        if (imageCount == 0) throw RuntimeError("Swapchain has zero images");
        uint32_t frames = std::min<uint32_t>(m_config.MaxFramesInFlight, imageCount);
        m_frames.clear();
        for (uint32_t i = 0; i < frames; ++i) {
            auto fr = std::make_unique<FrameContext>();
            fr->Initialize(*m_context, m_commandPool);
            m_frames.push_back(std::move(fr));
        }
        m_currentFrame = 0;
        MGLOG_D("FrameResources created: %u", (uint32_t)Frames.size());
    }

    void VulkanRenderer::DestroyFrameResources() {
        for (auto& f : m_frames) {
            if (f) f->Cleanup(*m_context);
        }
        m_frames.clear();
    }

    void VulkanRenderer::RecordFrameCommandBuffer(FrameContext& frame, uint32_t imageIndex) {
        // Begin
        VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        VK_VERIFY(vkBeginCommandBuffer(frame.CommandBuffer, &bi), "vkBeginCommandBuffer");

        VkClearValue clear{};
        clear.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        VkRenderPassBeginInfo rpbi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        rpbi.renderPass = m_renderPass;
        rpbi.framebuffer = m_swapchain->GetFramebuffers()[imageIndex];
        rpbi.renderArea.offset = {0, 0};
        rpbi.renderArea.extent = m_swapchain->GetExtent();
        rpbi.clearValueCount = 1;
        rpbi.pClearValues = &clear;

        vkCmdBeginRenderPass(frame.CommandBuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);

        for (auto& kv : m_renderCallbacks) {
            if (kv.second) {
                kv.second(frame.CommandBuffer, imageIndex, m_swapchain->GetExtent());
            }
        }

        vkCmdEndRenderPass(frame.CommandBuffer);
        VK_VERIFY(vkEndCommandBuffer(frame.CommandBuffer), "vkEndCommandBuffer");
    }

    void VulkanRenderer::RecreateSwapchainIfNeeded() {
        vkDeviceWaitIdle(m_context->GetDevice());
        DestroyFrameResources();
        DestroyRenderPass();
        m_swapchain->Recreate();
        CreateRenderPass();
        CreateFrameResources();
    }

    // Wait fence & Acquire image & Record commands & Submit
    void VulkanRenderer::RenderFrame() {
        if (!m_context) throw RuntimeError("Renderer not initialized");
        FrameContext& frame = *m_frames[m_currentFrame];

        // Ensure the previous use of this frame context has fully completed
        // before reusing its semaphores in vkAcquireNextImageKHR.
        VK_VERIFY(vkWaitForFences(m_context->GetDevice(), 1, &frame.InFlightFence, VK_TRUE, UINT64_MAX), "vkWaitForFences");

        if (!FrameBegin()) return;

        // Fence will be signaled by vkQueueSubmit below.
        VK_VERIFY(vkResetFences(m_context->GetDevice(), 1, &frame.InFlightFence), "vkResetFences");

        // Record commands
        VK_VERIFY(vkResetCommandBuffer(frame.CommandBuffer, 0), "vkResetCommandBuffer");
        RecordFrameCommandBuffer(frame, frame.CurrentImageIndex);

        // Submit
        VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        VkSemaphore waitSemaphores[] = {frame.ImageAvailable};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        si.waitSemaphoreCount = 1;
        si.pWaitSemaphores = waitSemaphores;
        si.pWaitDstStageMask = waitStages;
        si.commandBufferCount = 1;
        si.pCommandBuffers = &frame.CommandBuffer;
        VkSemaphore signalSemaphores[] = {frame.RenderFinished};
        si.signalSemaphoreCount = 1;
        si.pSignalSemaphores = signalSemaphores;

        VK_VERIFY(vkQueueSubmit(m_context->GetGraphicsQueue(), 1, &si, frame.InFlightFence), "vkQueueSubmit");
    }

    bool VulkanRenderer::FrameBegin() {
        FrameContext& frame = *m_frames[m_currentFrame];

        while (true) {
            // Acquire image for this frame. Acquire can fail with OUT_OF_DATE during resize/minimize.
            Uint32 imageIndex = 0;
            VkResult res = vkAcquireNextImageKHR(m_context->GetDevice(), m_swapchain->GetSwapchain(), UINT64_MAX,
                                                 frame.ImageAvailable, VK_NULL_HANDLE, &imageIndex);

            if (res == VK_ERROR_OUT_OF_DATE_KHR) {
                MGLOG_D("vkAcquireNextImageKHR: OUT_OF_DATE -> recreate");
                RecreateSwapchainIfNeeded();
                continue;
            }
            if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
                VK_VERIFY(res, "vkAcquireNextImageKHR");
                return false;
            }

            auto& imagesInFlight = m_swapchain->GetImagesInFlight();
            if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
                vkWaitForFences(m_context->GetDevice(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
            }
            imagesInFlight[imageIndex] = frame.InFlightFence;
            frame.CurrentImageIndex = imageIndex;
            return true;
        }
    }

    void VulkanRenderer::Present() {
        if (!m_context) throw RuntimeError("Renderer not initialized");
        FrameContext& frame = *m_frames[m_currentFrame];

        // Present
        VkPresentInfoKHR pi{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        VkSemaphore signalSemaphores[] = {frame.RenderFinished};
        pi.waitSemaphoreCount = 1;
        pi.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR scs[] = {m_swapchain->GetSwapchain()};
        pi.swapchainCount = 1;
        pi.pSwapchains = scs;
        pi.pImageIndices = &frame.CurrentImageIndex;
        VkResult pres = vkQueuePresentKHR(m_context->GetGraphicsQueue(), &pi);
        if (pres == VK_ERROR_OUT_OF_DATE_KHR || pres == VK_SUBOPTIMAL_KHR) {
            MGLOG_D("vkQueuePresentKHR: out_of_date/suboptimal -> recreate");
            RecreateSwapchainIfNeeded();
        } else {
            VK_VERIFY(pres, "vkQueuePresentKHR");
        }

        m_currentFrame = (m_currentFrame + 1) % m_frames.size();
    }

    void VulkanRenderer::RegisterRenderCallback(const String& name, RenderCallback cb) {
        auto it = std::find_if(m_renderCallbacks.begin(), m_renderCallbacks.end(),
                               [&](const auto& kv) { return kv.first == name; });
        if (it != m_renderCallbacks.end()) {
            MGLOG_W("Render callback '%s' already registered", name.c_str());
            return;
        }
        m_renderCallbacks.emplace_back(name, std::move(cb));
    }

    void VulkanRenderer::UnregisterRenderCallback(const String& name) {
        m_renderCallbacks.erase(std::remove_if(m_renderCallbacks.begin(), m_renderCallbacks.end(),
                                             [&](const auto& kv) { return kv.first == name; }),
                              m_renderCallbacks.end());
    }

    VkPipeline VulkanRenderer::CreateGraphicsPipelineFromSpv(const String& key, const Vector<uint32_t>& vsSpv,
                                                             const Vector<uint32_t>& fsSpv) {
        return m_pipelineMgr->CreateGraphicsPipelineFromSpv(key, vsSpv, fsSpv, m_renderPass, m_swapchain->GetExtent());
    }

    VkExtent2D VulkanRenderer::GetExtent() const {
        return m_swapchain ? m_swapchain->GetExtent() : VkExtent2D{0, 0};
    }

    void VulkanRenderer::WaitIdle() {
        if (m_context && m_context->GetDevice() != VK_NULL_HANDLE) vkDeviceWaitIdle(m_context->GetDevice());
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
