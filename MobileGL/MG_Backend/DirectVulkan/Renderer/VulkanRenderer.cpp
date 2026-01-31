#include "VulkanRenderer.h"
#include "VulkanContext.h"
#include "SwapchainManager.h"
#include "PipelineManager.h"
#include "FrameContext.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    VulkanRenderer::VulkanRenderer(ANativeWindow* window, const RendererConfig& cfg) : Window(window), Config(cfg) {
        Ctx = std::make_unique<VulkanContext>();
    }

    VulkanRenderer::~VulkanRenderer() {
        Shutdown();
    }

    void VulkanRenderer::Initialize() {
        if (!Window) throw MobileGL::RuntimeError("ANativeWindow is null");
        Ctx->Initialize(Window, Config.AppName);

        Swapchain = std::make_unique<SwapchainManager>(*Ctx);
        Swapchain->Initialize();

        CreateRenderPass();

        PipelineMgr = std::make_unique<PipelineManager>(*Ctx);

        CreateCommandPool();
        CreateFrameResources();

        MGLOG_D("VulkanRenderer initialized");
    }

    void VulkanRenderer::Shutdown() {
        if (!Ctx) return;

        vkDeviceWaitIdle(Ctx->GetDevice());

        DestroyFrameResources();
        DestroyCommandPool();

        if (PipelineMgr) {
            PipelineMgr->Cleanup();
            PipelineMgr.reset();
        }
        DestroyRenderPass();
        if (Swapchain) {
            Swapchain->Cleanup();
            Swapchain.reset();
        }
        if (Ctx) {
            Ctx->Shutdown();
            Ctx.reset();
        }
        MGLOG_D("VulkanRenderer shutdown");
    }

    void VulkanRenderer::CreateRenderPass() {
        VkAttachmentDescription color{};
        color.format = Swapchain->GetFormat();
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

        ThrowIfFailed(vkCreateRenderPass(Ctx->GetDevice(), &rpci, nullptr, &RenderPass), "vkCreateRenderPass");

        // Create framebuffers now (use swapchain imageviews)
        const auto& imageViews = Swapchain->GetImageViews();
        std::vector<VkFramebuffer> fbs;
        fbs.reserve(imageViews.size());
        for (auto iv : imageViews) {
            VkImageView attachments[] = {iv};
            VkFramebufferCreateInfo fbci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            fbci.renderPass = RenderPass;
            fbci.attachmentCount = 1;
            fbci.pAttachments = attachments;
            fbci.width = Swapchain->GetExtent().width;
            fbci.height = Swapchain->GetExtent().height;
            fbci.layers = 1;
            VkFramebuffer fb;
            ThrowIfFailed(vkCreateFramebuffer(Ctx->GetDevice(), &fbci, nullptr, &fb), "vkCreateFramebuffer");
            fbs.push_back(fb);
        }
        Swapchain->SetFramebuffers(std::move(fbs));
        MGLOG_D("RenderPass created and framebuffers set");
    }

    void VulkanRenderer::DestroyRenderPass() {
        if (RenderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(Ctx->GetDevice(), RenderPass, nullptr);
            RenderPass = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::CreateCommandPool() {
        VkCommandPoolCreateInfo cpci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        cpci.queueFamilyIndex = Ctx->GetGraphicsQueueFamily();
        cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        ThrowIfFailed(vkCreateCommandPool(Ctx->GetDevice(), &cpci, nullptr, &CommandPool), "vkCreateCommandPool");
    }

    void VulkanRenderer::DestroyCommandPool() {
        if (CommandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(Ctx->GetDevice(), CommandPool, nullptr);
            CommandPool = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::CreateFrameResources() {
        uint32_t imageCount = static_cast<uint32_t>(Swapchain->GetImageViews().size());
        if (imageCount == 0) throw MobileGL::RuntimeError("Swapchain has zero images");
        uint32_t frames = std::min<uint32_t>(Config.MaxFramesInFlight, imageCount);
        Frames.clear();
        for (uint32_t i = 0; i < frames; ++i) {
            auto fr = std::make_unique<FrameContext>();
            fr->Initialize(*Ctx, CommandPool);
            Frames.push_back(std::move(fr));
        }
        CurrentFrame = 0;
        MGLOG_D("FrameResources created: %u", (uint32_t)Frames.size());
    }

    void VulkanRenderer::DestroyFrameResources() {
        for (auto& f : Frames) {
            if (f) f->Cleanup(*Ctx);
        }
        Frames.clear();
    }

    void VulkanRenderer::RecordFrameCommandBuffer(FrameContext& frame, uint32_t imageIndex) {
        // Begin
        VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        ThrowIfFailed(vkBeginCommandBuffer(frame.CommandBuffer, &bi), "vkBeginCommandBuffer");

        VkClearValue clear{};
        clear.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        VkRenderPassBeginInfo rpbi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        rpbi.renderPass = RenderPass;
        rpbi.framebuffer = Swapchain->GetFramebuffers()[imageIndex];
        rpbi.renderArea.offset = {0, 0};
        rpbi.renderArea.extent = Swapchain->GetExtent();
        rpbi.clearValueCount = 1;
        rpbi.pClearValues = &clear;

        vkCmdBeginRenderPass(frame.CommandBuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);

        for (auto& kv : RenderCallbacks) {
            if (kv.second) {
                kv.second(frame.CommandBuffer, imageIndex, Swapchain->GetExtent());
            }
        }

        vkCmdEndRenderPass(frame.CommandBuffer);
        ThrowIfFailed(vkEndCommandBuffer(frame.CommandBuffer), "vkEndCommandBuffer");
    }

    void VulkanRenderer::RecreateSwapchainIfNeeded() {
        vkDeviceWaitIdle(Ctx->GetDevice());
        DestroyFrameResources();
        DestroyRenderPass();
        Swapchain->Recreate();
        CreateRenderPass();
        CreateFrameResources();
    }

    // Wait fence & Acquire image & Record commands & Submit & Present
    void VulkanRenderer::RenderFrame() {
        if (!Ctx) throw MobileGL::RuntimeError("Renderer not initialized");
        FrameContext& frame = *Frames[CurrentFrame];

        // Wait fence and reset
        ThrowIfFailed(vkWaitForFences(Ctx->GetDevice(), 1, &frame.InFlightFence, VK_TRUE, UINT64_MAX),
                      "vkWaitForFences");
        ThrowIfFailed(vkResetFences(Ctx->GetDevice(), 1, &frame.InFlightFence), "vkResetFences");

        // Acquire image
        uint32_t imageIndex = 0;
        VkResult res = vkAcquireNextImageKHR(Ctx->GetDevice(), Swapchain->GetSwapchain(), UINT64_MAX,
                                             frame.ImageAvailable, VK_NULL_HANDLE, &imageIndex);
        if (res == VK_ERROR_OUT_OF_DATE_KHR) {
            MGLOG_D("vkAcquireNextImageKHR: OUT_OF_DATE -> recreate");
            RecreateSwapchainIfNeeded();
            return;
        }
        ThrowIfFailed(res, "vkAcquireNextImageKHR");

        // Record commands
        ThrowIfFailed(vkResetCommandBuffer(frame.CommandBuffer, 0), "vkResetCommandBuffer");
        RecordFrameCommandBuffer(frame, imageIndex);

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

        ThrowIfFailed(vkQueueSubmit(Ctx->GetGraphicsQueue(), 1, &si, frame.InFlightFence), "vkQueueSubmit");

        // Present
        VkPresentInfoKHR pi{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        pi.waitSemaphoreCount = 1;
        pi.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR scs[] = {Swapchain->GetSwapchain()};
        pi.swapchainCount = 1;
        pi.pSwapchains = scs;
        pi.pImageIndices = &imageIndex;
        VkResult pres = vkQueuePresentKHR(Ctx->GetGraphicsQueue(), &pi);
        if (pres == VK_ERROR_OUT_OF_DATE_KHR || pres == VK_SUBOPTIMAL_KHR) {
            MGLOG_D("vkQueuePresentKHR: out_of_date/suboptimal -> recreate");
            RecreateSwapchainIfNeeded();
        } else {
            ThrowIfFailed(pres, "vkQueuePresentKHR");
        }

        CurrentFrame = (CurrentFrame + 1) % Frames.size();
    }

    void VulkanRenderer::RegisterRenderCallback(const std::string& name, RenderCallback cb) {
        auto it = std::find_if(RenderCallbacks.begin(), RenderCallbacks.end(),
                               [&](const auto& kv) { return kv.first == name; });
        if (it != RenderCallbacks.end()) {
            MGLOG_W("Render callback '%s' already registered", name.c_str());
            return;
        }
        RenderCallbacks.emplace_back(name, std::move(cb));
    }

    void VulkanRenderer::UnregisterRenderCallback(const std::string& name) {
        RenderCallbacks.erase(std::remove_if(RenderCallbacks.begin(), RenderCallbacks.end(),
                                             [&](const auto& kv) { return kv.first == name; }),
                              RenderCallbacks.end());
    }

    VkPipeline VulkanRenderer::CreateGraphicsPipelineFromSpv(const std::string& key, const std::vector<uint32_t>& vsSpv,
                                                             const std::vector<uint32_t>& fsSpv) {
        return PipelineMgr->CreateGraphicsPipelineFromSpv(key, vsSpv, fsSpv, RenderPass, Swapchain->GetExtent());
    }

    VkExtent2D VulkanRenderer::GetExtent() const {
        return Swapchain ? Swapchain->GetExtent() : VkExtent2D{0, 0};
    }

    void VulkanRenderer::WaitIdle() {
        if (Ctx && Ctx->GetDevice() != VK_NULL_HANDLE) vkDeviceWaitIdle(Ctx->GetDevice());
    }
} // namespace MobileGL::MG_Backend::DirectVulkan