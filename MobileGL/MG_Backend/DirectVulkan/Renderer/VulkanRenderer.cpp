// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VulkanRenderer.h"
#include "VertexInputStateFactory.h"
#include "VertexInputStateBuilder.h"

#include "MG_State/GLState/ProgramState/ProgramObject.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    VkBool32 VulkanRenderer::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                           VkDebugUtilsMessageTypeFlagsEXT messageType,
                                           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        auto typeToString = [](VkDebugUtilsMessageTypeFlagsEXT messageType) {
            switch (messageType) {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
                return "General";
            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
                return "Validation";
            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
                return "Performance";
            case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
                return "DeviceAddressBinding";
            default:
                return "Other";
            }
        };

        switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            MGLOG_E("Vulkan Debug: [%s] %s", typeToString(messageType), pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            MGLOG_W("Vulkan Debug: [%s] %s", typeToString(messageType), pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            MGLOG_I("Vulkan Debug: [%s] %s", typeToString(messageType), pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            MGLOG_D("Vulkan Debug: [%s] %s", typeToString(messageType), pCallbackData->pMessage);
            break;
        default:
            break;
        }
        return VK_FALSE;
    }

    VulkanRenderer::VulkanRenderer(NativeWindowType window, const VulkanRendererConfig& cfg)
        : m_window(window), m_config(cfg) {
        // Initialize();
    }

    VulkanRenderer::~VulkanRenderer() {
        Shutdown();
    }

    VkPipeline VulkanRenderer::GetOrCreatePipeline(
        const MG_State::GLState::ProgramObject& program, Uint64 vertexInputHash,
        const VkPipelineVertexInputStateCreateInfo& vertexInputState) {
        MOBILEGL_ASSERT(m_pipelineFactory != nullptr, "PipelineFactory is not initialized");
        MOBILEGL_ASSERT(m_programFactory != nullptr, "ProgramFactory is not initialized");
        auto& stages = m_programFactory->GetOrCreatePipelineShaderStages(program, ProgramFactory::CompileOptionBit::None);
        if (stages.empty()) {
            MGLOG_W("GetOrCreatePipeline skipped: program has no shader stages");
            return VK_NULL_HANDLE;
        }
        const Uint64 programHash = m_programFactory->ComputeHash(program, ProgramFactory::CompileOptionBit::None);

        PipelineFactory::PipelineCreatePayload payload{};
        payload.programHash = programHash;
        payload.vertexInputHash = vertexInputHash;
        payload.pipelineLayout = m_pipelineLayout;
        payload.renderPass = m_renderPassLoad;
        payload.subpass = 0;
        payload.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        payload.stages = &stages;
        payload.vertexInputState = &vertexInputState;
        return m_pipelineFactory->GetOrCreatePipeline(payload);
    }

    void VulkanRenderer::PrepareDemoPipeline() {
        MGLOG_D("PrepareDemoPipeline called");

        VkPipelineLayoutCreateInfo plci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        VkDescriptorSetLayout descriptorSetLayouts[1] = {VK_NULL_HANDLE};
        if (m_uniformDescriptorBinder) {
            descriptorSetLayouts[0] = m_uniformDescriptorBinder->GetDescriptorSetLayout();
        }
        if (descriptorSetLayouts[0] != VK_NULL_HANDLE) {
            plci.setLayoutCount = 1;
            plci.pSetLayouts = descriptorSetLayouts;
        }
        VK_VERIFY(vkCreatePipelineLayout(m_device, &plci, nullptr, &m_pipelineLayout), "vkCreatePipelineLayout");

        MGLOG_I("PrepareDemoPipeline completed");
    }

    void VulkanRenderer::CreateFrameContexts() {
        VK_VERIFY(m_frameContext.Initialize(m_device, m_commandPool, m_config.MaxFramesInFlight),
                  "CreateFrameContexts");
        MGLOG_I("CreateFrameContexts completed");
    }

    void VulkanRenderer::Initialize() {
        CreateInstance();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDeviceAndQueues();
        CreateAllocator();

        CreateCommandPool();

        RecreateSwapchain();

        m_pipelineFactory = MakeUnique<PipelineFactory>(m_device, m_config);
        m_programFactory = MakeUnique<ProgramFactory>(m_device, m_config);
        m_uniformDescriptorBinder = MakeUnique<UniformDescriptorBinder>();
        if (!m_uniformDescriptorBinder->Initialize(m_device, m_allocator,
                                                   m_physicalDevice.properties.limits.minUniformBufferOffsetAlignment,
                                                   m_config.MaxFramesInFlight)) {
            MGLOG_E("UniformDescriptorBinder initialization failed. UBO sync on Vulkan backend is disabled.");
            m_uniformDescriptorBinder.reset();
        }
        m_vertexInputStateFactory = MakeUnique<VertexInputStateFactory>(m_config);

        PrepareDemoPipeline();
        CreateFrameContexts();

        // Prime the first frame so Render() always targets an acquired swapchain image.
        VK_VERIFY(m_frameContext.WaitAndAcquireNextImage(m_device, m_swapchainObject.GetHandle(), m_imageIndexAcquired),
                  "Initialize, WaitAndAcquireNextImage");

        MGLOG_D("VulkanRenderer initialized");
    }

    void VulkanRenderer::Shutdown() {
        VK_VERIFY(vkDeviceWaitIdle(m_device));

        m_pipelineFactory.reset();
        m_programFactory.reset();
        m_vertexInputStateFactory.reset();
        for (auto& vertexBuffer : m_vertexBuffers) {
            if (vertexBuffer) {
                vertexBuffer->Destroy();
            }
        }
        m_vertexBuffers.clear();
        m_indexBuffer.Destroy();

        m_frameContext.Destroy(m_device, m_commandPool);

        if (m_pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
            m_pipelineLayout = VK_NULL_HANDLE;
        }
        if (m_uniformDescriptorBinder) {
            m_uniformDescriptorBinder->Shutdown();
            m_uniformDescriptorBinder.reset();
        }

        ShutdownSwapchain();

        if (m_commandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(m_device, m_commandPool, nullptr);
            m_commandPool = VK_NULL_HANDLE;
        }

        DestroyAllocator();

        if (m_device != VK_NULL_HANDLE) {
            vkDestroyDevice(m_device, nullptr);
            m_device = VK_NULL_HANDLE;
        }

        if (m_surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
            m_surface = VK_NULL_HANDLE;
        }

        if (m_debugMessenger != VK_NULL_HANDLE) {
            DestroyDebugMessenger();
            m_debugMessenger = VK_NULL_HANDLE;
        }

        if (m_instance != VK_NULL_HANDLE) {
            vkDestroyInstance(m_instance, nullptr);
            m_instance = VK_NULL_HANDLE;
        }
        MGLOG_I("VulkanRenderer shut down completed");
    }

    void VulkanRenderer::RequestClear(GLbitfield mask, const FloatVec4& color, Float depth, Uint32 stencil) {
        const GLbitfield supportedMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
        const GLbitfield requestMask = (mask & supportedMask);
        if (requestMask == 0) {
            return;
        }

        if ((requestMask & GL_COLOR_BUFFER_BIT) != 0) {
            m_pendingClearColor.float32[0] = color.x();
            m_pendingClearColor.float32[1] = color.y();
            m_pendingClearColor.float32[2] = color.z();
            m_pendingClearColor.float32[3] = color.w();
        }
        if ((requestMask & GL_DEPTH_BUFFER_BIT) != 0) {
            m_pendingClearDepth = depth;
        }
        if ((requestMask & GL_STENCIL_BUFFER_BIT) != 0) {
            m_pendingClearStencil = stencil;
        }
        m_pendingClearMask |= requestMask;
    }

    Bool VulkanRenderer::ConsumePendingColorClear(VkClearColorValue& outClearColor) {
        if ((m_pendingClearMask & GL_COLOR_BUFFER_BIT) == 0) {
            return false;
        }

        outClearColor = m_pendingClearColor;
        m_pendingClearMask &= ~GL_COLOR_BUFFER_BIT;
        return true;
    }

    void VulkanRenderer::TransitionSwapchainImageToColorAttachment(VkCommandBuffer commandBuffer, Uint32 imageIndex) {
        const auto oldLayout = m_swapchainObject.GetImageLayout(imageIndex);
        if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            return;
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_swapchainObject.GetImage(imageIndex);
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);
        m_swapchainObject.SetImageLayout(imageIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    void VulkanRenderer::RecordColorClear(VkCommandBuffer commandBuffer, const VkClearColorValue& clearColor) {
        VkClearAttachment clearAttachment{};
        clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clearAttachment.colorAttachment = 0;
        clearAttachment.clearValue.color = clearColor;

        VkClearRect clearRect{};
        clearRect.rect.offset = {0, 0};
        clearRect.rect.extent = m_swapchainObject.GetExtent();
        clearRect.baseArrayLayer = 0;
        clearRect.layerCount = 1;

        vkCmdClearAttachments(commandBuffer, 1, &clearAttachment, 1, &clearRect);
    }

    void VulkanRenderer::RecordDepthStencilClear(VkCommandBuffer commandBuffer, GLbitfield mask, Float depth, Uint32 stencil) {
        VkImageAspectFlags aspectMask = 0;
        if ((mask & GL_DEPTH_BUFFER_BIT) != 0) {
            aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        if ((mask & GL_STENCIL_BUFFER_BIT) != 0 && HasStencilComponent(m_depthStencilFormat)) {
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
        clearRect.rect.extent = m_swapchainObject.GetExtent();
        clearRect.baseArrayLayer = 0;
        clearRect.layerCount = 1;

        vkCmdClearAttachments(commandBuffer, 1, &clearAttachment, 1, &clearRect);
    }

    void VulkanRenderer::TransitionDepthStencilImageToAttachment(VkCommandBuffer commandBuffer, Uint32 imageIndex) {
        if (m_depthStencilImageLayouts.empty()) {
            return;
        }

        const auto oldLayout = m_depthStencilImageLayouts[imageIndex];
        if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            return;
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_depthStencilImages[imageIndex];
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (HasStencilComponent(m_depthStencilFormat)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);
        m_depthStencilImageLayouts[imageIndex] = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    void VulkanRenderer::EnsureFrameRecordingStarted() {
        auto& frame = m_frameContext.GetCurrent();
        if (frame.isCommandRecording) {
            return;
        }

        if (frame.hasCommandBufferRecorded) {
            MGLOG_W("EnsureFrameRecordingStarted skipped: current frame command buffer is already finalized");
            return;
        }

        VkCommandBuffer& commandBuffer = m_frameContext.BeginCommandRecording();
        if (m_uniformDescriptorBinder) {
            m_uniformDescriptorBinder->BeginFrame(m_frameContext.GetCurrentFrameIndex());
        }
        TransitionSwapchainImageToColorAttachment(commandBuffer, m_imageIndexAcquired);
        TransitionDepthStencilImageToAttachment(commandBuffer, m_imageIndexAcquired);

        const Bool useRenderPassClearValue = (m_pendingClearMask & GL_COLOR_BUFFER_BIT) != 0;
        VkClearValue clearValues[1]{};
        if (useRenderPassClearValue) {
            clearValues[0].color = m_pendingClearColor;
            m_pendingClearMask &= ~GL_COLOR_BUFFER_BIT;
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = useRenderPassClearValue ? m_renderPassClear : m_renderPassLoad;
        renderPassInfo.framebuffer = m_framebuffers[m_imageIndexAcquired];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapchainObject.GetExtent();
        renderPassInfo.clearValueCount = useRenderPassClearValue ? 1 : 0;
        renderPassInfo.pClearValues = useRenderPassClearValue ? clearValues : nullptr;
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        m_isMainRenderPassActive = true;

        if ((m_pendingClearMask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) != 0) {
            RecordDepthStencilClear(commandBuffer, m_pendingClearMask, m_pendingClearDepth, m_pendingClearStencil);
            m_pendingClearMask &= ~(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }
    }

    void VulkanRenderer::EndFrameRecordingIfNeeded() {
        auto& frame = m_frameContext.GetCurrent();
        if (!frame.isCommandRecording) {
            return;
        }

        if (m_isMainRenderPassActive) {
            vkCmdEndRenderPass(frame.commandBuffer);
            m_isMainRenderPassActive = false;
        }
        m_frameContext.EndCommandRecording();
    }

    Bool VulkanRenderer::UploadAndBindVertexStreams(
        const VertexInputStateFactory::BackendVertexInputState& vertexInputState,
        const MG_State::GLState::VertexArrayObject& vertexArray,
        VkCommandBuffer commandBuffer) {
        if (vertexInputState.bindings.empty()) {
            return true;
        }

        const auto bindingCount = vertexInputState.bindings.size();
        if (vertexInputState.bindingBufferKeys.size() != bindingCount) {
            MGLOG_E("UploadAndBindVertexStreams failed: binding metadata mismatch");
            return false;
        }

        if (m_vertexBuffers.size() < bindingCount) {
            m_vertexBuffers.resize(bindingCount);
        }

        Vector<VkBuffer> vkBuffers(bindingCount, VK_NULL_HANDLE);
        Vector<VkDeviceSize> vkOffsets(bindingCount, 0);

        for (SizeT binding = 0; binding < bindingCount; ++binding) {
            const SizeT bufferKey = vertexInputState.bindingBufferKeys[binding];
            const MG_State::GLState::BufferObject* sourceBuffer = nullptr;
            for (Uint32 location = 0; location < MG_State::GLState::VertexArrayObject::MAX_VERTEX_ATTRIBS; ++location) {
                const auto& attr = vertexArray.GetAttribute(location);
                if (!attr.Enabled || !attr.Buffer) {
                    continue;
                }
                if (reinterpret_cast<SizeT>(attr.Buffer.get()) == bufferKey) {
                    sourceBuffer = attr.Buffer.get();
                    break;
                }
            }

            if (!sourceBuffer) {
                MGLOG_W("UploadAndBindVertexStreams skipped: no source buffer for binding %zu", binding);
                return false;
            }

            const auto sourceData = sourceBuffer->GetDataReadOnly();
            if (!sourceData || sourceData->empty()) {
                MGLOG_W("UploadAndBindVertexStreams skipped: source buffer has no data for binding %zu", binding);
                return false;
            }

            if (!m_vertexBuffers[binding]) {
                m_vertexBuffers[binding] = MakeUnique<VkBufferObject>();
            }

            auto& backendBuffer = *m_vertexBuffers[binding];
            const SizeT sourceSize = sourceBuffer->GetSize();
            if (!backendBuffer.IsValid() || backendBuffer.GetSize() < sourceSize) {
                backendBuffer.Destroy();
                const Bool created = backendBuffer.Create(
                    m_allocator, static_cast<VkDeviceSize>(sourceSize),
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VMA_MEMORY_USAGE_AUTO,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
                if (!created) {
                    MGLOG_E("UploadAndBindVertexStreams skipped: failed to create backend buffer for binding %zu", binding);
                    return false;
                }
            }

            if (!backendBuffer.Upload(sourceData->data(), static_cast<VkDeviceSize>(sourceSize), 0)) {
                MGLOG_E("UploadAndBindVertexStreams skipped: failed to upload binding %zu", binding);
                return false;
            }

            vkBuffers[binding] = backendBuffer.GetHandle();
        }

        vkCmdBindVertexBuffers(commandBuffer, 0, static_cast<Uint32>(bindingCount), vkBuffers.data(), vkOffsets.data());
        return true;
    }

    void VulkanRenderer::DrawArrays(const DrawArrayPayload& payload) {
        if (payload.mode != GL_TRIANGLES) {
            MGLOG_W("DrawArrays skipped: primitive mode %u is not supported yet", payload.mode);
            return;
        }

        const VertexInputStateFactory::BackendVertexInputState* vertexInputState = nullptr;
        if (payload.vertexArray && m_vertexInputStateFactory) {
            vertexInputState = &m_vertexInputStateFactory->GetOrCreateVertexInputState(*payload.vertexArray);
        }

        EnsureFrameRecordingStarted();
        auto& frame = m_frameContext.GetCurrent();
        if (!frame.isCommandRecording || !m_isMainRenderPassActive) {
            MGLOG_W("DrawArrays skipped: frame recording was not started");
            return;
        }

        VkCommandBuffer& commandBuffer = frame.commandBuffer;
        const auto swapchainExtent = m_swapchainObject.GetExtent();

        if (payload.program == nullptr) {
            MGLOG_W("DrawArrays skipped: no current program is bound");
            return;
        }

        const Uint64 vertexInputHash = vertexInputState ? vertexInputState->hash : 0;
        const VkPipelineVertexInputStateCreateInfo* vertexInputInfo =
            vertexInputState ? &vertexInputState->state : nullptr;
        if (!vertexInputInfo) {
            VertexInputStateBuilder emptyVertexInputBuilder;
            vertexInputInfo = &emptyVertexInputBuilder.Build();
        }

        VkPipeline pipelineToBind = GetOrCreatePipeline(*payload.program, vertexInputHash, *vertexInputInfo);
        if (pipelineToBind == VK_NULL_HANDLE) {
            MGLOG_W("DrawArrays skipped: failed to create/get pipeline");
            return;
        }

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineToBind);
        if (m_uniformDescriptorBinder &&
            !m_uniformDescriptorBinder->BindProgramUniformBuffers(commandBuffer, m_pipelineLayout, *payload.program,
                                                                  m_frameContext.GetCurrentFrameIndex())) {
            MGLOG_W("DrawArrays skipped: failed to bind uniform descriptors");
            return;
        }

        if (vertexInputState && !vertexInputState->bindings.empty()) {
            if (!payload.vertexArray) {
                MGLOG_W("DrawArrays skipped: vertex input requires VAO");
                return;
            }
            if (!UploadAndBindVertexStreams(*vertexInputState, *payload.vertexArray, commandBuffer)) {
                return;
            }
        }

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapchainExtent.width);
        viewport.height = static_cast<float>(swapchainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapchainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdDraw(commandBuffer, static_cast<Uint32>(payload.count), 1, static_cast<Uint32>(payload.first), 0);
    }

    void VulkanRenderer::DrawElements(const DrawElementPayload& payload) {
        if (payload.drawArray.mode != GL_TRIANGLES) {
            MGLOG_W("DrawElements skipped: primitive mode %u is not supported yet", payload.drawArray.mode);
            return;
        }

        EnsureFrameRecordingStarted();
        auto& frame = m_frameContext.GetCurrent();
        if (!frame.isCommandRecording || !m_isMainRenderPassActive) {
            MGLOG_W("DrawElements skipped: frame recording was not started");
            return;
        }

        VkIndexType vkIndexType = VK_INDEX_TYPE_MAX_ENUM;
        switch (payload.indexType) {
        case GL_UNSIGNED_SHORT:
            vkIndexType = VK_INDEX_TYPE_UINT16;
            break;
        case GL_UNSIGNED_INT:
            vkIndexType = VK_INDEX_TYPE_UINT32;
            break;
        default:
            MGLOG_W("DrawElements skipped: index type %u is not supported yet", payload.indexType);
            return;
        }

        MOBILEGL_ASSERT(payload.indexData != nullptr, "DrawElements requires non-null indexData");
        MOBILEGL_ASSERT(payload.indexDataSizeBytes > 0, "DrawElements requires non-zero index data size");

        const VertexInputStateFactory::BackendVertexInputState* vertexInputState = nullptr;
        if (payload.drawArray.vertexArray && m_vertexInputStateFactory) {
            vertexInputState = &m_vertexInputStateFactory->GetOrCreateVertexInputState(*payload.drawArray.vertexArray);
        }

        if (payload.drawArray.program == nullptr) {
            MGLOG_W("DrawElements skipped: no current program is bound");
            return;
        }

        const Uint64 vertexInputHash = vertexInputState ? vertexInputState->hash : 0;
        const VkPipelineVertexInputStateCreateInfo* vertexInputInfo =
            vertexInputState ? &vertexInputState->state : nullptr;
        if (!vertexInputInfo) {
            VertexInputStateBuilder emptyVertexInputBuilder;
            vertexInputInfo = &emptyVertexInputBuilder.Build();
        }

        VkPipeline pipelineToBind = GetOrCreatePipeline(*payload.drawArray.program, vertexInputHash, *vertexInputInfo);
        if (pipelineToBind == VK_NULL_HANDLE) {
            MGLOG_W("DrawElements skipped: failed to create/get pipeline");
            return;
        }

        if (!m_indexBuffer.IsValid() || m_indexBuffer.GetSize() < payload.indexDataSizeBytes) {
            m_indexBuffer.Destroy();
            const Bool created = m_indexBuffer.Create(
                m_allocator, static_cast<VkDeviceSize>(payload.indexDataSizeBytes),
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VMA_MEMORY_USAGE_AUTO,
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
            if (!created) {
                MGLOG_E("DrawElements skipped: failed to create index buffer");
                return;
            }
        }

        if (!m_indexBuffer.Upload(payload.indexData, static_cast<VkDeviceSize>(payload.indexDataSizeBytes), 0)) {
            MGLOG_E("DrawElements skipped: failed to upload index data");
            return;
        }

        VkCommandBuffer& commandBuffer = frame.commandBuffer;
        const auto swapchainExtent = m_swapchainObject.GetExtent();

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineToBind);
        if (m_uniformDescriptorBinder &&
            !m_uniformDescriptorBinder->BindProgramUniformBuffers(commandBuffer, m_pipelineLayout,
                                                                  *payload.drawArray.program,
                                                                  m_frameContext.GetCurrentFrameIndex())) {
            MGLOG_W("DrawElements skipped: failed to bind uniform descriptors");
            return;
        }

        if (vertexInputState && !vertexInputState->bindings.empty()) {
            if (!payload.drawArray.vertexArray) {
                MGLOG_W("DrawElements skipped: vertex input requires VAO");
                return;
            }
            if (!UploadAndBindVertexStreams(*vertexInputState, *payload.drawArray.vertexArray, commandBuffer)) {
                return;
            }
        }

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapchainExtent.width);
        viewport.height = static_cast<float>(swapchainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapchainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.GetHandle(), 0, vkIndexType);
        vkCmdDrawIndexed(commandBuffer, static_cast<Uint32>(payload.drawArray.count), 1, 0, 0, 0);
    }

    void VulkanRenderer::Render() {
        // Route test rendering through the same frame-start logic used by draw calls,
        // so pending glClear() state can be consumed consistently.
        EnsureFrameRecordingStarted();
        auto& frame = m_frameContext.GetCurrent();
        if (!frame.isCommandRecording || !m_isMainRenderPassActive) {
            MGLOG_W("Render skipped: frame recording was not started");
            return;
        }

        VkCommandBuffer& commandBuffer = frame.commandBuffer;
        const auto swapchainExtent = m_swapchainObject.GetExtent();

        (void)commandBuffer;
        (void)swapchainExtent;
    }

    void VulkanRenderer::Present() {
        MOBILEGL_ASSERT(m_imageIndexAcquired < m_swapchainObject.GetImageCount(),
                        "Present, acquired image index out of range");
        auto& frame = m_frameContext.GetCurrent();
        if (m_pendingClearMask != 0 && frame.isCommandRecording && m_isMainRenderPassActive) {
            if ((m_pendingClearMask & GL_COLOR_BUFFER_BIT) != 0) {
                RecordColorClear(frame.commandBuffer, m_pendingClearColor);
            }
            if ((m_pendingClearMask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) != 0) {
                RecordDepthStencilClear(frame.commandBuffer, m_pendingClearMask, m_pendingClearDepth, m_pendingClearStencil);
            }
            m_pendingClearMask = 0;
        } else if (m_pendingClearMask != 0 && frame.hasCommandBufferRecorded) {
            MGLOG_W("Dropping pending clear for current frame because command buffer is already finalized");
            m_pendingClearMask = 0;
        } else if (m_pendingClearMask != 0) {
            EnsureFrameRecordingStarted();
        }
        EndFrameRecordingIfNeeded();

        const auto acquiredImageLayout = m_swapchainObject.GetImageLayout(m_imageIndexAcquired);
        const Bool needsLayoutTransitionForPresent =
            m_frameContext.TransitionToPresent(m_swapchainObject.GetImage(m_imageIndexAcquired), acquiredImageLayout);
        const Bool shouldSubmitCommandBuffer = frame.hasCommandBufferRecorded || needsLayoutTransitionForPresent;

        // 1) Submit current frame work.
        auto submitPacket = m_frameContext.GetSubmitInfo(shouldSubmitCommandBuffer);
        VK_VERIFY(vkQueueSubmit(m_graphicsQueue, 1, &submitPacket.submitInfo, frame.imageInFlightFence));
        frame.isCommandRecording = false;
        frame.hasCommandBufferRecorded = false;
        m_swapchainObject.SetImageLayout(m_imageIndexAcquired, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        // 2) Present current frame.
        auto presentPacket = m_frameContext.GetPresentInfo(m_swapchainObject.GetHandle(), m_imageIndexAcquired);
        auto result = vkQueuePresentKHR(m_presentQueue, &presentPacket.presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            MGLOG_D("Present, vkQueuePresentKHR got %d, recreating swapchain", result);
            RecreateSwapchain();
            result = VK_SUCCESS;
        }
        VK_VERIFY(result, "Present, vkQueuePresentKHR");

        // 3) Advance frame slot.
        m_frameContext.AdvanceToNext();

        // 4) Wait/reset/acquire for next frame.
        result = m_frameContext.WaitAndAcquireNextImage(m_device, m_swapchainObject.GetHandle(), m_imageIndexAcquired);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            MGLOG_D("Present, vkAcquireNextImageKHR got %d, recreating swapchain", result);
            RecreateSwapchain();
            result = VK_SUCCESS;
        }
        VK_VERIFY(result, "Present, vkAcquireNextImageKHR");
    }

    void VulkanRenderer::CreateInstance() {
        m_extensions = EnumerateInstanceExtensions();
        MGLOG_I("Got %d Vulkan instance extensions: ", m_extensions.size());
        for (auto& extension : m_extensions) {
            MGLOG_I("    %s (r.%u)", extension.extensionName, extension.specVersion);
        }

        Bool validationLayerAvailable = CheckValidationLayerSupport();
        MGLOG_I("Validation layers %s.", validationLayerAvailable ? "available" : "not available");
        MGLOG_I("Validation layers %s.", m_config.EnableValidationLayers ? "requested" : "not requested");

        if (m_config.EnableValidationLayers && !validationLayerAvailable) {
            MGLOG_I("Validation layers not available! Disabling validation layers.");
        }

        m_validationLayersEnabled = m_config.EnableValidationLayers && validationLayerAvailable;

        // ---------------- App info -------------------
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = m_config.AppName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(m_config.CacheVersion, 0, 0);
        appInfo.pEngineName = "MobileGL";
        appInfo.engineVersion = VK_MAKE_VERSION(m_config.Version.Major, m_config.Version.Minor, m_config.Version.Patch);
#ifdef VK_USE_PLATFORM_WIN32_KHR
        appInfo.apiVersion = VK_API_VERSION_1_3;
#else
        appInfo.apiVersion = VK_API_VERSION_1_1;
#endif

        // ---------------- Instance info -------------------
        VkInstanceCreateInfo instanceInfo = {};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pApplicationInfo = &appInfo;

        // Extensions
        Vector<const char*> exts = {VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_ANDROID_KHR
                                    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#elif defined VK_USE_PLATFORM_WIN32_KHR
                                    VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined VK_USE_PLATFORM_METAL_EXT
                                    VK_EXT_METAL_SURFACE_EXTENSION_NAME
#else
#warning "VulkanContext::CreateInstance: VK_KHR_*_surface extension not defined on this platform"
#endif
        }; // TODO: support more platforms

#if defined(VK_USE_PLATFORM_METAL_EXT)
        exts.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

        if (m_validationLayersEnabled) {
            exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        instanceInfo.enabledExtensionCount = exts.size();
        instanceInfo.ppEnabledExtensionNames = exts.data();

        auto debugMessengerCreateInfo = PopulateDebugMessengerCreateInfo();
        // Layers
        if (m_validationLayersEnabled) {
            MGLOG_I("Enabling validation layer...");
            instanceInfo.enabledLayerCount = static_cast<uint32_t>(std::size(s_validationLayerNames));
            instanceInfo.ppEnabledLayerNames = s_validationLayerNames;
            instanceInfo.pNext = &debugMessengerCreateInfo;
        } else {
            instanceInfo.enabledLayerCount = 0;
            instanceInfo.pNext = nullptr;
        }

        VK_VERIFY(vkCreateInstance(&instanceInfo, nullptr, &m_instance), "vkCreateInstance failed");

        if (m_validationLayersEnabled) VK_VERIFY(SetupDebugMessenger());
    }

    VkResult VulkanRenderer::SetupDebugMessenger() {
        auto createInfo = PopulateDebugMessengerCreateInfo();
        auto vkCreateDebugUtilsMessengerEXT =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
        if (!vkCreateDebugUtilsMessengerEXT) return VK_ERROR_EXTENSION_NOT_PRESENT;
        VK_VERIFY(vkCreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger));
        return VK_SUCCESS;
    }

    VkResult VulkanRenderer::DestroyDebugMessenger() {
        if (m_debugMessenger != VK_NULL_HANDLE) {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance,
                                                                                   "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr) {
                func(m_instance, m_debugMessenger, nullptr);
            } else {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
        }
        return VK_SUCCESS;
    }

    VkDebugUtilsMessengerCreateInfoEXT VulkanRenderer::PopulateDebugMessengerCreateInfo() {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
        createInfo.pUserData = this;
        return createInfo;
    }

    void VulkanRenderer::PickPhysicalDevice() {
        Uint32 deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            MGLOG_E("No physical devices supporting Vulkan found.");
        } else {
            MGLOG_I("Found %d physical device(s).", deviceCount);
        }

        MOBILEGL_ASSERT(deviceCount > 0, "No physical devices found.");

        Vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
        for (Int i = 0; i < deviceCount; i++) {
            if (GetMoreCapablePhysicalDevice(devices[i], m_surface, m_physicalDevice, m_physicalDevice))
                MGLOG_I("Picked physical device %d.", i);
        }

        if (m_physicalDevice.handle == VK_NULL_HANDLE) {
            m_physicalDevice.handle = devices[0];
            vkGetPhysicalDeviceProperties(devices[0], &m_physicalDevice.properties);
            MGLOG_I("No suitable physical device picked yet, defaulting to device 0.");
            MGLOG_W("No graphics queue found on physical device. Picking a device that doesn't do graphics?");
        }
    }

    Bool VulkanRenderer::GetMoreCapablePhysicalDevice(VkPhysicalDevice newVkDevice, VkSurfaceKHR surface,
                                                      const PhysicalDevice& otherDevice,
                                                      PhysicalDevice& outBetterDevice) {
        const auto deviceTypeToStr = [](VkPhysicalDeviceType type) {
            switch (type) {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                return "INTEGRATED_GPU";
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                return "DISCRETE_GPU";
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                return "CPU";
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                return "VIRTUAL_GPU";
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                return "OTHER";
            default:
                return "UNKNOWN";
            }
        };

        PhysicalDevice newDevice;
        newDevice.handle = newVkDevice;

        vkGetPhysicalDeviceProperties(newVkDevice, &newDevice.properties);
        const auto& deviceProperties = newDevice.properties;
        auto apiVersion = deviceProperties.apiVersion;
        MGLOG_I("    %s (Vulkan %d.%d.%d, %s)", deviceProperties.deviceName, VK_VERSION_MAJOR(apiVersion),
                VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion),
                deviceTypeToStr(deviceProperties.deviceType));

        // Check device extensions (including swapchain extension)
        Bool deviceExtSupported = IsNecessaryDeviceExtensionSupported(newVkDevice);
        if (!deviceExtSupported) {
            outBetterDevice = otherDevice;
            MGLOG_I("    Ignored physical device. (Reason: Some of the required device extension not supported on this "
                    "device)");
            return false;
        }

        // Check swapchain capabilities
        auto swapchainCapabilities = SwapchainObject::GetSwapchainCapabilities(newVkDevice, surface);
        if (!swapchainCapabilities.IsComplete()) {
            outBetterDevice = otherDevice;
            MGLOG_I("    Ignored physical device. (Reason: Swapchain capabilities not met)");
            return false;
        }

        // Check queue families
        Vector<VkQueueFamilyProperties> queueFamilies = GetQueueFamilyFromPhysicalDevice(newVkDevice);
        newDevice.queueFamilies.graphicsFamily = GetQueueFamilyIndex(queueFamilies, VK_QUEUE_GRAPHICS_BIT);
        if (newDevice.queueFamilies.graphicsFamily == -1) {
            outBetterDevice = otherDevice;
            MGLOG_I("    Ignored physical device. (Reason: No graphics queue family)");
            return false;
        }

        newDevice.queueFamilies.presentFamily =
            GetPresentQueueFamilyIndex(newDevice, surface, queueFamilies, newDevice.queueFamilies.graphicsFamily);
        if (newDevice.queueFamilies.presentFamily == -1) {
            outBetterDevice = otherDevice;
            MGLOG_I("    Ignored physical device. (Reason: No present queue family)");
            return false;
        }

        // Pick discrete GPU
        if (newDevice.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            otherDevice.properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            outBetterDevice = newDevice;
            MGLOG_I("    Picked physical device. (Reason: Discrete GPU)");
            return true;
        }

        // Pick integrated GPU if no discrete GPU
        if (newDevice.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU &&
            otherDevice.properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            outBetterDevice = newDevice;
            MGLOG_I("    Picked physical device. (Reason: Integrated GPU and no discrete one found yet)");
            return true;
        }

        // Ignore other GPU when discrete GPU found
        if (newDevice.properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            otherDevice.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            outBetterDevice = newDevice;
            MGLOG_I("    Ignored physical device. (Reason: Already picked discrete GPU)");
            return false;
        }

        return false;
    }

    Bool VulkanRenderer::IsNecessaryDeviceExtensionSupported(VkPhysicalDevice device) {
        Uint32 extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        MGLOG_I("Got %u Vulkan device extensions: ", extensionCount);
        for (auto& extension : availableExtensions) {
            MGLOG_I("    %s (r.%u)", extension.extensionName, extension.specVersion);
        }

        for (SizeT i = 0; i < std::size(s_deviceExtensionNames); ++i) {
            Bool found = false;
            for (const auto& extension : availableExtensions) {
                if (strcmp(extension.extensionName, s_deviceExtensionNames[i]) == 0) {
                    MGLOG_I("Required extension found: %s", s_deviceExtensionNames[i]);
                    found = true;
                    break;
                }
            }
            if (!found) {
                MGLOG_I("Required extension not found: %s", s_deviceExtensionNames[i]);
                return false;
            }
        }

        return true;
    }

    void VulkanRenderer::CreateLogicalDeviceAndQueues() {
        Float queuePriority = 1.0f;

        Vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        MOBILEGL_ASSERT(m_physicalDevice.queueFamilies.graphicsFamily != -1, "Graphics queue family not found.");
        VkDeviceQueueCreateInfo& gfxQueueCreateInfo = queueCreateInfos.emplace_back();
        gfxQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        gfxQueueCreateInfo.queueFamilyIndex = m_physicalDevice.queueFamilies.graphicsFamily;
        gfxQueueCreateInfo.queueCount = 1;
        gfxQueueCreateInfo.pQueuePriorities = &queuePriority;

        if (m_physicalDevice.queueFamilies.graphicsFamily != m_physicalDevice.queueFamilies.presentFamily) {
            MOBILEGL_ASSERT(m_physicalDevice.queueFamilies.presentFamily != -1, "Present queue family not found.");
            VkDeviceQueueCreateInfo& presentQueueCreateInfo = queueCreateInfos.emplace_back();
            presentQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            presentQueueCreateInfo.queueFamilyIndex = m_physicalDevice.queueFamilies.presentFamily;
            presentQueueCreateInfo.queueCount = 1;
            presentQueueCreateInfo.pQueuePriorities = &queuePriority;
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        // TODO: query and make use of device features

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        if (m_validationLayersEnabled) {
            deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(std::size(s_validationLayerNames));
            deviceCreateInfo.ppEnabledLayerNames = s_validationLayerNames;
        } else {
            deviceCreateInfo.enabledLayerCount = 0;
        }

        Vector<const char*> enabledDeviceExtensions;
        enabledDeviceExtensions.reserve(std::size(s_deviceExtensionNames) + 1);
        for (const char* extensionName : s_deviceExtensionNames) {
            enabledDeviceExtensions.push_back(extensionName);
        }

#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
        Uint32 extensionCount = 0;
        VK_VERIFY(vkEnumerateDeviceExtensionProperties(m_physicalDevice.handle, nullptr, &extensionCount, nullptr));
        Vector<VkExtensionProperties> availableExtensions(extensionCount);
        VK_VERIFY(vkEnumerateDeviceExtensionProperties(m_physicalDevice.handle, nullptr, &extensionCount,
                                                       availableExtensions.data()));

        for (const auto& extension : availableExtensions) {
            if (strcmp(extension.extensionName, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) == 0) {
                enabledDeviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
                MGLOG_I("Enabled optional device extension: %s", VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
                break;
            }
        }
#endif

        deviceCreateInfo.enabledExtensionCount = static_cast<Uint32>(enabledDeviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();
        VK_VERIFY(vkCreateDevice(m_physicalDevice.handle, &deviceCreateInfo, nullptr, &m_device), "vkCreateDevice");
        MGLOG_I("Logical device created.");

        // Queues
        vkGetDeviceQueue(m_device, m_physicalDevice.queueFamilies.graphicsFamily, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, m_physicalDevice.queueFamilies.presentFamily, 0, &m_presentQueue);
        MGLOG_I("Queues got successfully.");
    }

    void VulkanRenderer::CreateAllocator() {
        MOBILEGL_ASSERT(m_instance != VK_NULL_HANDLE, "CreateAllocator requires valid VkInstance");
        MOBILEGL_ASSERT(m_physicalDevice.handle != VK_NULL_HANDLE, "CreateAllocator requires valid physical device");
        MOBILEGL_ASSERT(m_device != VK_NULL_HANDLE, "CreateAllocator requires valid VkDevice");

        if (m_allocator != nullptr) {
            return;
        }

        VmaAllocatorCreateInfo allocatorInfo{};
        VmaVulkanFunctions vulkanFunctions{};
        vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        allocatorInfo.instance = m_instance;
        allocatorInfo.physicalDevice = m_physicalDevice.handle;
        allocatorInfo.device = m_device;
        allocatorInfo.pVulkanFunctions = &vulkanFunctions;
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;

        VK_VERIFY(vmaCreateAllocator(&allocatorInfo, &m_allocator), "vmaCreateAllocator");
    }

    void VulkanRenderer::DestroyAllocator() {
        if (m_allocator != nullptr) {
            vmaDestroyAllocator(m_allocator);
            m_allocator = nullptr;
        }
    }

    void VulkanRenderer::CreateSwapchain() {
        m_swapchainObject.Create(m_device, m_physicalDevice.handle, m_surface,
                                 static_cast<Uint32>(m_physicalDevice.queueFamilies.graphicsFamily),
                                 static_cast<Uint32>(m_physicalDevice.queueFamilies.presentFamily),
                                 m_config.MaxFramesInFlight);
    }

    Uint32 VulkanRenderer::FindMemoryType(Uint32 typeFilter, VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties memProperties{};
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice.handle, &memProperties);

        for (Uint32 i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        MOBILEGL_ASSERT(false, "Failed to find suitable memory type.");
        return 0;
    }

    Bool VulkanRenderer::HasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT;
    }

    VkFormat VulkanRenderer::FindSupportedDepthStencilFormat(VkPhysicalDevice physicalDevice) {
        const VkFormat candidates[] = {
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D32_SFLOAT
        };
        for (VkFormat format : candidates) {
            VkFormatProperties props{};
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
            if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
                return format;
            }
        }
        return VK_FORMAT_UNDEFINED;
    }

    void VulkanRenderer::CreateDepthStencilResources() {
        const auto imageCount = static_cast<Uint32>(m_swapchainObject.GetImageCount());
        if (imageCount == 0) {
            return;
        }

        m_depthStencilFormat = FindSupportedDepthStencilFormat(m_physicalDevice.handle);
        MOBILEGL_ASSERT(m_depthStencilFormat != VK_FORMAT_UNDEFINED, "No supported depth/stencil format found.");

        const auto extent = m_swapchainObject.GetExtent();
        m_depthStencilImages.assign(imageCount, VK_NULL_HANDLE);
        m_depthStencilImageMemories.assign(imageCount, VK_NULL_HANDLE);
        m_depthStencilImageViews.assign(imageCount, VK_NULL_HANDLE);
        m_depthStencilImageLayouts.assign(imageCount, VK_IMAGE_LAYOUT_UNDEFINED);

        for (Uint32 i = 0; i < imageCount; ++i) {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = extent.width;
            imageInfo.extent.height = extent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = m_depthStencilFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            VK_VERIFY(vkCreateImage(m_device, &imageInfo, nullptr, &m_depthStencilImages[i]), "vkCreateImage(depth)");

            VkMemoryRequirements memRequirements{};
            vkGetImageMemoryRequirements(m_device, m_depthStencilImages[i], &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VK_VERIFY(vkAllocateMemory(m_device, &allocInfo, nullptr, &m_depthStencilImageMemories[i]), "vkAllocateMemory(depth)");
            VK_VERIFY(vkBindImageMemory(m_device, m_depthStencilImages[i], m_depthStencilImageMemories[i], 0), "vkBindImageMemory(depth)");

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_depthStencilImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_depthStencilFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (HasStencilComponent(m_depthStencilFormat)) {
                viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;
            VK_VERIFY(vkCreateImageView(m_device, &viewInfo, nullptr, &m_depthStencilImageViews[i]), "vkCreateImageView(depth)");
        }
    }

    void VulkanRenderer::DestroyDepthStencilResources() {
        for (auto view : m_depthStencilImageViews) {
            if (view != VK_NULL_HANDLE) {
                vkDestroyImageView(m_device, view, nullptr);
            }
        }
        m_depthStencilImageViews.clear();

        for (auto image : m_depthStencilImages) {
            if (image != VK_NULL_HANDLE) {
                vkDestroyImage(m_device, image, nullptr);
            }
        }
        m_depthStencilImages.clear();

        for (auto memory : m_depthStencilImageMemories) {
            if (memory != VK_NULL_HANDLE) {
                vkFreeMemory(m_device, memory, nullptr);
            }
        }
        m_depthStencilImageMemories.clear();
        m_depthStencilImageLayouts.clear();
        m_depthStencilFormat = VK_FORMAT_UNDEFINED;
    }

    void VulkanRenderer::CreateCommandPool() {
        VkCommandPoolCreateInfo createInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = m_physicalDevice.queueFamilies.graphicsFamily;
        VK_VERIFY(vkCreateCommandPool(m_device, &createInfo, nullptr, &m_commandPool));
        MGLOG_I("Command pool created");
    }

    VkRenderPass VulkanRenderer::CreateDefaultRenderPass(VkAttachmentLoadOp loadOp) {
        VkAttachmentDescription color{};
        color.format = m_swapchainObject.GetSurfaceFormat().format;
        color.samples = VK_SAMPLE_COUNT_1_BIT;
        color.loadOp = loadOp;
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
        VkSubpassDescription sub{};
        sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sub.colorAttachmentCount = 1;
        sub.pColorAttachments = &colorRef;
        sub.pDepthStencilAttachment = &depthStencilRef;

        VkAttachmentDescription attachments[] = {color, depthStencil};

        VkRenderPassCreateInfo rpci{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        rpci.attachmentCount = static_cast<Uint32>(std::size(attachments));
        rpci.pAttachments = attachments;
        rpci.subpassCount = 1;
        rpci.pSubpasses = &sub;

        VkRenderPass renderPass = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateRenderPass(m_device, &rpci, nullptr, &renderPass), "vkCreateRenderPass");
        return renderPass;
    }

    void VulkanRenderer::CreateDefaultRenderPass() {
        m_renderPassLoad = CreateDefaultRenderPass(VK_ATTACHMENT_LOAD_OP_LOAD);
        m_renderPassClear = CreateDefaultRenderPass(VK_ATTACHMENT_LOAD_OP_CLEAR);
        MGLOG_D("RenderPasses created (LOAD/CLEAR).");
    }

    void VulkanRenderer::CreateDefaultFramebuffers() {
        // Create framebuffers now (use swapchain imageviews)
        const auto& imageViews = m_swapchainObject.GetImageViews();
        const auto swapchainExtent = m_swapchainObject.GetExtent();
        Vector<VkFramebuffer>& fbs = m_framebuffers;
        fbs.reserve(imageViews.size());
        for (SizeT i = 0; i < imageViews.size(); ++i) {
            VkImageView attachments[] = {imageViews[i], m_depthStencilImageViews[i]};
            VkFramebufferCreateInfo fbci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            fbci.renderPass = m_renderPassLoad;
            fbci.attachmentCount = static_cast<Uint32>(std::size(attachments));
            fbci.pAttachments = attachments;
            fbci.width = swapchainExtent.width;
            fbci.height = swapchainExtent.height;
            fbci.layers = 1;
            VkFramebuffer fb;
            VK_VERIFY(vkCreateFramebuffer(m_device, &fbci, nullptr, &fb), "vkCreateFramebuffer");
            fbs.push_back(fb);
        }
        MGLOG_D("Default framebuffer created.");
    }

    void VulkanRenderer::CreateSurface() {
#if defined VK_USE_PLATFORM_ANDROID_KHR
        auto* nativeWindow = static_cast<ANativeWindow*>(m_window);
        if (!nativeWindow) throw RuntimeError("ANativeWindowType is null");

        VkAndroidSurfaceCreateInfoKHR sci{VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};
        sci.window = nativeWindow;
        VK_VERIFY(vkCreateAndroidSurfaceKHR(m_instance, &sci, nullptr, &m_surface), "vkCreateAndroidSurfaceKHR failed");
#elif defined VK_USE_PLATFORM_WIN32_KHR
        auto hwnd = static_cast<HWND>(m_window);
        MOBILEGL_ASSERT(hwnd, "HWND is null");

        VkWin32SurfaceCreateInfoKHR sci{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
        sci.hwnd = hwnd;
        VK_VERIFY(vkCreateWin32SurfaceKHR(m_instance, &sci, nullptr, &m_surface), "vkCreateWin32SurfaceKHR failed");
#elif defined VK_USE_PLATFORM_METAL_EXT
        MOBILEGL_ASSERT(m_window, "CAMetalLayer is null");

        VkMetalSurfaceCreateInfoEXT sci{VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT};
        sci.pLayer = reinterpret_cast<const void*>(m_window);
        VK_VERIFY(vkCreateMetalSurfaceEXT(m_instance, &sci, nullptr, &m_surface), "vkCreateMetalSurfaceEXT failed");
#else
        // #warning "VulkanRenderer::Initialize called on a platform which is not supported yet"
        MGLOG_W("VulkanRenderer::Initialize called on a platform which is not supported yet"); // TODO: support more
                                                                                               // platforms
#endif
    }

    Vector<VkQueueFamilyProperties> VulkanRenderer::GetQueueFamilyFromPhysicalDevice(VkPhysicalDevice device) {
        Uint32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        Vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        return queueFamilies;
    }

    Int VulkanRenderer::GetQueueFamilyIndex(const Vector<VkQueueFamilyProperties>& queueFamilies,
                                            VkQueueFlagBits flag) {
        for (Uint32 i = 0; i < queueFamilies.size(); i++) {
            if (queueFamilies[i].queueFlags & flag) {
                return i;
            }
        }
        return -1;
    }

    Int VulkanRenderer::GetPresentQueueFamilyIndex(const PhysicalDevice& physicalDevice, VkSurfaceKHR surface,
                                                   const Vector<VkQueueFamilyProperties>& queueFamilies,
                                                   Int preferredFamilyIndex) {
        if (preferredFamilyIndex != -1) {
            VkBool32 supportsPresent = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.handle, preferredFamilyIndex, surface,
                                                 &supportsPresent);
            if (supportsPresent) return preferredFamilyIndex;
        }

        for (Uint32 i = 0; i < queueFamilies.size(); i++) {
            VkBool32 supportsPresent = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.handle, i, surface, &supportsPresent);
            if (supportsPresent) return i;
        }
        return -1;
    }

    Vector<VkExtensionProperties> VulkanRenderer::EnumerateInstanceExtensions() {
        Uint32 extensionCount = 0;
        VK_VERIFY(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
        Vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        return extensions;
    }

    Bool VulkanRenderer::CheckValidationLayerSupport() {
        Uint32 layerCount = 0;
        VK_VERIFY(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

        Vector<VkLayerProperties> layers(layerCount);
        VK_VERIFY(vkEnumerateInstanceLayerProperties(&layerCount, layers.data()));

        for (const char* layerName : s_validationLayerNames) {
            for (const auto& layerProperties : layers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    return true;
                }
            }
        }
        return false;
    }

    void VulkanRenderer::ShutdownSwapchain() {
        for (auto fb : m_framebuffers) {
            vkDestroyFramebuffer(m_device, fb, nullptr);
        }
        m_framebuffers.clear();

        DestroyDepthStencilResources();

        if (m_renderPassClear != VK_NULL_HANDLE) {
            vkDestroyRenderPass(m_device, m_renderPassClear, nullptr);
            m_renderPassClear = VK_NULL_HANDLE;
        }

        if (m_renderPassLoad != VK_NULL_HANDLE) {
            vkDestroyRenderPass(m_device, m_renderPassLoad, nullptr);
            m_renderPassLoad = VK_NULL_HANDLE;
        }

        m_swapchainObject.Shutdown(m_device);
    }

    void VulkanRenderer::RecreateSwapchain() {
        // Handle cases like minimize on Windows, where swapchain could return a 0x0 extent
        const auto swapchainCapabilities = SwapchainObject::GetSwapchainCapabilities(m_physicalDevice.handle, m_surface);
        if (swapchainCapabilities.capabilities.currentExtent.width == 0 ||
            swapchainCapabilities.capabilities.currentExtent.height == 0) {
            return;
        }

        vkDeviceWaitIdle(m_device);

        ShutdownSwapchain();

        CreateSwapchain();
        CreateDepthStencilResources();
        CreateDefaultRenderPass();
        CreateDefaultFramebuffers();
        if (m_pipelineFactory) {
            m_pipelineFactory->DestroyAll();
        }
        if (m_frameContext.GetFrameCount() > 0) {
            m_frameContext.GetCurrent().isCommandRecording = false;
            m_frameContext.GetCurrent().hasCommandBufferRecorded = false;
        }
        m_isMainRenderPassActive = false;
    }

} // namespace MobileGL::MG_Backend::DirectVulkan
