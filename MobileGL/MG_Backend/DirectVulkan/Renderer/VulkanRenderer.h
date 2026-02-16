// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include "Config.h"
#include "FrameContext.h"
#include "ProgramFactory.h"
#include "SwapchainObject.h"
#include "MG_Util/Math/VectorTypes.h"
#include <Includes.h>

#include "../VkIncludes.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    class VulkanRenderer {
    public:
        VulkanRenderer(NativeWindowType window, const VulkanRendererConfig& cfg = {});
        ~VulkanRenderer();

        void Initialize();
        void Shutdown();

        void RequestClear(GLbitfield mask, const FloatVec4& color);
        Bool ConsumePendingColorClear(VkClearColorValue& outClearColor);
        void EnsureFrameRecordingStarted();
        void Render();
        void Present();

        void RecreateSwapchain();

    private:
        struct QueueFamilyIndices {
            Int32 graphicsFamily = -1;
            Int32 presentFamily = -1;
        };

        struct PhysicalDevice {
            QueueFamilyIndices queueFamilies;
            VkPhysicalDeviceProperties properties;
            VkPhysicalDevice handle = VK_NULL_HANDLE;

            Bool IsComplete() const {
                return handle != VK_NULL_HANDLE &&
                    queueFamilies.graphicsFamily != -1 &&
                    queueFamilies.presentFamily != -1;
            }
        };

        NativeWindowType m_window = 0;
        VulkanRendererConfig m_config;

        // Vulkan objects
        Bool m_validationLayersEnabled = false;
        Vector<VkExtensionProperties> m_extensions;
        VkInstance m_instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
        PhysicalDevice m_physicalDevice;
        // VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        SwapchainObject m_swapchainObject;

        // Vector<VkQueueFamilyProperties> m_queueFamilies;
        // QueueFamilyIndices m_queueFamilyIndices;

        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;

        VkCommandPool m_commandPool = VK_NULL_HANDLE;

        VkRenderPass m_renderPassLoad = VK_NULL_HANDLE;
        VkRenderPass m_renderPassClear = VK_NULL_HANDLE;
        Vector<VkFramebuffer> m_framebuffers;

        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_pipeline = VK_NULL_HANDLE;

        Uint m_imageIndexAcquired = 0;
        FrameContext m_frameContext;
        Bool m_pendingColorClear = false;
        VkClearColorValue m_pendingClearColor = {{0.0f, 0.0f, 0.0f, 1.0f}};
        Bool m_isMainRenderPassActive = false;

        UniquePtr<ProgramFactory> m_programFactory;

        void CreateInstance();
        VkResult SetupDebugMessenger();
        VkResult DestroyDebugMessenger();
        VkDebugUtilsMessengerCreateInfoEXT PopulateDebugMessengerCreateInfo();
        void CreateSurface();
        void PickPhysicalDevice();
        void CreateLogicalDeviceAndQueues();
        void CreateSwapchain();
        void CreateCommandPool();
        void CreateFrameContexts();
        void CreateDefaultRenderPass();
        VkRenderPass CreateDefaultRenderPass(VkAttachmentLoadOp loadOp);
        void CreateDefaultFramebuffers();
        void PrepareDemoPipeline();
        void TransitionSwapchainImageToColorAttachment(VkCommandBuffer commandBuffer, Uint32 imageIndex);
        void RecordColorClear(VkCommandBuffer commandBuffer, const VkClearColorValue& clearColor);
        void EndFrameRecordingIfNeeded();

        void ShutdownSwapchain();

        static Int GetPresentQueueFamilyIndex(
            const PhysicalDevice& physicalDevice, VkSurfaceKHR surface,
            const Vector<VkQueueFamilyProperties>& queueFamilies, Int preferredFamilyIndex = -1);
        static Vector<VkQueueFamilyProperties> GetQueueFamilyFromPhysicalDevice(VkPhysicalDevice device);
        static Int GetQueueFamilyIndex(const Vector<VkQueueFamilyProperties>& queueFamilies, VkQueueFlagBits flag);
        static Vector<VkExtensionProperties> EnumerateInstanceExtensions();
        static Bool IsNecessaryDeviceExtensionSupported(VkPhysicalDevice device);
        static Bool GetMoreCapablePhysicalDevice(VkPhysicalDevice newVkDevice, VkSurfaceKHR surface, const PhysicalDevice& compareWithDevice, PhysicalDevice& outBetterDevice);
        static constexpr VkDynamicState s_dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        static constexpr const char* s_validationLayerNames[] = {
            "VK_LAYER_KHRONOS_validation"
        };
        static constexpr const char* s_deviceExtensionNames[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        static Bool CheckValidationLayerSupport();

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
