// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VulkanRenderer.h"

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

    VulkanRenderer::VulkanRenderer(NativeWindowType window, const RendererConfig& cfg)
        : m_window(window), m_config(cfg) {
        // Initialize();
    }

    VulkanRenderer::~VulkanRenderer() {
        Shutdown();
    }

    const char* demoFS = R"(#version 460
        layout(location = 0) in vec3 fragColor;
        layout(location = 0) out vec4 outColor;
        void main() {
            outColor = vec4(fragColor, 1.0);
        }
)";
    const char* demoVS = R"(#version 460
    layout(location = 0) out vec3 fragColor;
    vec2 positions[3] = vec2[](vec2(0.0, -0.5), vec2(0.5, 0.5), vec2(-0.5, 0.5));
    vec3 colors[3] = vec3[](vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));
    void main() {
        gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
        fragColor = colors[gl_VertexID];
    }
)";
    void VulkanRenderer::PrepareDemoPipeline() {
        MGLOG_D("PrepareDemoRes called");

        // Create shader&program object
        auto programObject = MG_State::GLState::ProgramObject(0);
        auto vsObject = MakeShared<MG_State::GLState::ShaderObject>(ShaderStage::Vertex, 0);
        vsObject->SetShaderSource(demoVS);
        vsObject->Compile();
        if (!vsObject->GetCompileStatus()) {
            MGLOG_E("Vertex shader compilation failed: %s", vsObject->GetInfoLog().c_str());
            return;
        }
        auto fsObject = MakeShared<MG_State::GLState::ShaderObject>(ShaderStage::Fragment, 1);
        fsObject->SetShaderSource(demoFS);
        fsObject->Compile();
        if (!fsObject->GetCompileStatus()) {
            MGLOG_E("Fragment shader compilation failed: %s", fsObject->GetInfoLog().c_str());
            return;
        }
        programObject.AttachShader(vsObject);
        programObject.AttachShader(fsObject);
        programObject.Link();
        if (!programObject.GetLinkStatus()) {
            MGLOG_E("Program linking failed: %s", programObject.GetInfoLog().c_str());
            return;
        }

        Vector<Uint> vsSpv;
        Vector<Uint> fsSpv;

        auto& shaderSpirvs = programObject.GetGeneratedSpirv();
        auto& attachedShaders = programObject.GetAttachedShaders();
        for (int index = 0; index < attachedShaders.size(); ++index) {
            auto& shader = attachedShaders[index];
            auto& spirvCode = shaderSpirvs[index];
            if (shader->GetShaderStage() == ShaderStage::Vertex) {
                vsSpv = spirvCode;
            } else if (shader->GetShaderStage() == ShaderStage::Fragment) {
                fsSpv = spirvCode;
            }
        }

        VkShaderModuleCreateInfo smci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        smci.codeSize = vsSpv.size() * sizeof(uint32_t);
        smci.pCode = vsSpv.data();
        VkShaderModule vs;
        VK_VERIFY(vkCreateShaderModule(m_device, &smci, nullptr, &vs), "vkCreateShaderModule VS");

        smci.codeSize = fsSpv.size() * sizeof(uint32_t);
        smci.pCode = fsSpv.data();
        VkShaderModule fs;
        VK_VERIFY(vkCreateShaderModule(m_device, &smci, nullptr, &fs), "vkCreateShaderModule FS");

        VkPipelineShaderStageCreateInfo stages[2]{};
        stages[0] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = vs;
        stages[0].pName = "main";
        stages[1] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = fs;
        stages[1].pName = "main";

        VkPipelineLayoutCreateInfo plci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        VK_VERIFY(vkCreatePipelineLayout(m_device, &plci, nullptr, &m_pipelineLayout), "vkCreatePipelineLayout");

        // Create the rest of pipeline component

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(std::size(s_dynamicStates));
        dynamicState.pDynamicStates = s_dynamicStates;

        VkPipelineVertexInputStateCreateInfo vertexInput{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        vertexInput.vertexBindingDescriptionCount = 0;
        vertexInput.vertexAttributeDescriptionCount = 0;

        VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkViewport vp{};
        vp.x = 0;
        vp.y = 0;
        const auto swapchainExtent = m_swapchainObject.GetExtent();
        vp.width = (float)swapchainExtent.width;
        vp.height = (float)swapchainExtent.height;
        vp.minDepth = 0;
        vp.maxDepth = 1;
        VkRect2D scissor{{0, 0}, swapchainExtent};
        VkPipelineViewportStateCreateInfo vpci{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        vpci.viewportCount = 1;
        vpci.pViewports = &vp;
        vpci.scissorCount = 1;
        vpci.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        raster.polygonMode = VK_POLYGON_MODE_FILL;
        raster.cullMode = VK_CULL_MODE_NONE;
        raster.frontFace = VK_FRONT_FACE_CLOCKWISE;
        raster.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorAttach{};
        colorAttach.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorAttach.blendEnable = VK_FALSE;
        VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        blend.attachmentCount = 1;
        blend.pAttachments = &colorAttach;

        // Create Pipeline
        VkGraphicsPipelineCreateInfo gpi{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        gpi.stageCount = 2;
        gpi.pStages = stages;
        gpi.pVertexInputState = &vertexInput;
        gpi.pInputAssemblyState = &ia;
        gpi.pViewportState = &vpci;
        gpi.pRasterizationState = &raster;
        gpi.pMultisampleState = &ms;
        gpi.pColorBlendState = &blend;
        gpi.pDynamicState = &dynamicState;
        gpi.layout = m_pipelineLayout;
        gpi.renderPass = m_renderPass;
        gpi.subpass = 0;

        VK_VERIFY(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &gpi, nullptr, &m_pipeline),
                  "vkCreateGraphicsPipelines");

        vkDestroyShaderModule(m_device, vs, nullptr);
        vkDestroyShaderModule(m_device, fs, nullptr);

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

        CreateCommandPool();

        RecreateSwapchain();

        PrepareDemoPipeline();
        CreateFrameContexts();

        // Prime the first frame so Render() always targets an acquired swapchain image.
        VK_VERIFY(vkWaitForFences(m_device, 1, &m_frameContext.GetCurrentImageInFlightFence(), VK_TRUE, UINT64_MAX));
        VK_VERIFY(vkResetFences(m_device, 1, &m_frameContext.GetCurrentImageInFlightFence()));
        VK_VERIFY(vkAcquireNextImageKHR(m_device, m_swapchainObject.GetHandle(), UINT64_MAX,
                                        m_frameContext.GetCurrentImageAvailableSemaphore(), VK_NULL_HANDLE,
                                        &m_imageIndexAcquired));

        MGLOG_D("VulkanRenderer initialized");
    }

    void VulkanRenderer::Shutdown() {
        VK_VERIFY(vkDeviceWaitIdle(m_device));

        m_frameContext.Destroy(m_device, m_commandPool);

        if (m_pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(m_device, m_pipeline, nullptr);
        }

        if (m_pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
        }

        ShutdownSwapchain();

        if (m_commandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(m_device, m_commandPool, nullptr);
            m_commandPool = VK_NULL_HANDLE;
        }

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

    void VulkanRenderer::Render() {
        VkCommandBuffer& commandBuffer = m_frameContext.GetCurrentCommandBuffer();
        VK_VERIFY(vkResetCommandBuffer(commandBuffer, 0));

        // Begin command buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        VK_VERIFY(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        // Begin render pass
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_framebuffers[m_imageIndexAcquired];
        renderPassInfo.renderArea.offset = {0, 0};
        const auto swapchainExtent = m_swapchainObject.GetExtent();
        renderPassInfo.renderArea.extent = swapchainExtent;
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Render commands
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

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

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        // End render pass
        vkCmdEndRenderPass(commandBuffer);

        // End command buffer
        VK_VERIFY(vkEndCommandBuffer(commandBuffer));
        m_frameContext.SetCurrentHasCommandBufferRecorded(true);
    }

    void VulkanRenderer::Present() {
        MOBILEGL_ASSERT(m_imageIndexAcquired < m_swapchainObject.GetImageCount(),
                        "Present, acquired image index out of range");
        VkCommandBuffer& commandBuffer = m_frameContext.GetCurrentCommandBuffer();
        const Bool hasRecordedWork = m_frameContext.GetCurrentHasCommandBufferRecorded();
        Bool needsLayoutTransitionForPresent = false;

        if (!hasRecordedWork) {
            const auto acquiredImageLayout = m_swapchainObject.GetImageLayout(m_imageIndexAcquired);
            if (acquiredImageLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
                acquiredImageLayout != VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR) {
                needsLayoutTransitionForPresent = true;

                VK_VERIFY(vkResetCommandBuffer(commandBuffer, 0),
                          "Present, vkResetCommandBuffer(layout transition)");

                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                VK_VERIFY(vkBeginCommandBuffer(commandBuffer, &beginInfo),
                          "Present, vkBeginCommandBuffer(layout transition)");

                VkImageMemoryBarrier presentBarrier{};
                presentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                presentBarrier.srcAccessMask = 0;
                presentBarrier.dstAccessMask = 0;
                presentBarrier.oldLayout = acquiredImageLayout;
                presentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                presentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                presentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                presentBarrier.image = m_swapchainObject.GetImage(m_imageIndexAcquired);
                presentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                presentBarrier.subresourceRange.baseMipLevel = 0;
                presentBarrier.subresourceRange.levelCount = 1;
                presentBarrier.subresourceRange.baseArrayLayer = 0;
                presentBarrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                     VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
                                     &presentBarrier);

                VK_VERIFY(vkEndCommandBuffer(commandBuffer),
                          "Present, vkEndCommandBuffer(layout transition)");
            }
        }

        const Bool shouldSubmitCommandBuffer = hasRecordedWork || needsLayoutTransitionForPresent;

        // 1) Submit current frame work.
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = {m_frameContext.GetCurrentImageAvailableSemaphore()};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = shouldSubmitCommandBuffer ? 1U : 0U;
        submitInfo.pCommandBuffers = shouldSubmitCommandBuffer ? &commandBuffer : nullptr;
        VkSemaphore signalSemaphores[] = {m_frameContext.GetCurrentRenderFinishedSemaphore()};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        VK_VERIFY(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_frameContext.GetCurrentImageInFlightFence()));
        m_frameContext.SetCurrentHasCommandBufferRecorded(false);
        m_swapchainObject.SetImageLayout(m_imageIndexAcquired, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        // 2) Present current frame.
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = {m_swapchainObject.GetHandle()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &m_imageIndexAcquired;
        presentInfo.pResults = nullptr;
        auto result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            MGLOG_D("Present, vkQueuePresentKHR got %d, recreating swapchain", result);
            RecreateSwapchain();
            result = VK_SUCCESS;
        }
        VK_VERIFY(result, "Present, vkQueuePresentKHR");

        // 3) Advance frame slot.
        m_frameContext.AdvanceFrame();

        // 4) Wait/reset/acquire for next frame.
        VK_VERIFY(vkWaitForFences(m_device, 1, &m_frameContext.GetCurrentImageInFlightFence(), VK_TRUE, UINT64_MAX),
                  "Present, vkWaitForFences");
        VK_VERIFY(vkResetFences(m_device, 1, &m_frameContext.GetCurrentImageInFlightFence()),
                  "Present, vkResetFences");
        result =
            vkAcquireNextImageKHR(m_device, m_swapchainObject.GetHandle(), UINT64_MAX,
                                  m_frameContext.GetCurrentImageAvailableSemaphore(), VK_NULL_HANDLE,
                                  &m_imageIndexAcquired);
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

    void VulkanRenderer::CreateSwapchain() {
        m_swapchainObject.Create(m_device, m_physicalDevice.handle, m_surface,
                                 static_cast<Uint32>(m_physicalDevice.queueFamilies.graphicsFamily),
                                 static_cast<Uint32>(m_physicalDevice.queueFamilies.presentFamily),
                                 m_config.MaxFramesInFlight);
    }

    void VulkanRenderer::CreateCommandPool() {
        VkCommandPoolCreateInfo createInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = m_physicalDevice.queueFamilies.graphicsFamily;
        VK_VERIFY(vkCreateCommandPool(m_device, &createInfo, nullptr, &m_commandPool));
        MGLOG_I("Command pool created");
    }

    void VulkanRenderer::CreateDefaultRenderPass() {
        VkAttachmentDescription color{};
        color.format = m_swapchainObject.GetSurfaceFormat().format;
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

        VK_VERIFY(vkCreateRenderPass(m_device, &rpci, nullptr, &m_renderPass), "vkCreateRenderPass");

        MGLOG_D("RenderPass created.");
    }

    void VulkanRenderer::CreateDefaultFramebuffers() {
        // Create framebuffers now (use swapchain imageviews)
        const auto& imageViews = m_swapchainObject.GetImageViews();
        const auto swapchainExtent = m_swapchainObject.GetExtent();
        Vector<VkFramebuffer>& fbs = m_framebuffers;
        fbs.reserve(imageViews.size());
        for (auto iv : imageViews) {
            VkImageView attachments[] = {iv};
            VkFramebufferCreateInfo fbci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            fbci.renderPass = m_renderPass;
            fbci.attachmentCount = 1;
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

        if (m_renderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(m_device, m_renderPass, nullptr);
            m_renderPass = VK_NULL_HANDLE;
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
        CreateDefaultRenderPass();
        CreateDefaultFramebuffers();
        m_frameContext.ResetPerFrameState();
    }

} // namespace MobileGL::MG_Backend::DirectVulkan
