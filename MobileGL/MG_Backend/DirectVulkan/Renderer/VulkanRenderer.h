// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include "Config.h"
#include <Includes.h>

#define VK_VERIFY(expr, ...)                                                                                           \
    do {                                                                                                               \
        VkResult _vk_verify_result = (expr);                                                                           \
        MOBILEGL_ASSERT(_vk_verify_result == VK_SUCCESS, "Vulkan error %d at %s:%d" __VA_OPT__(" - ") __VA_ARGS__, _vk_verify_result, __FILE__, __LINE__);  \
    } while (0)

#define ENUM_STR_CASE(c) case c: return #c;

namespace MobileGL::MG_Backend::DirectVulkan {
    using RenderCallback = std::function<void(VkCommandBuffer cmdBuf, uint32_t imageIndex, VkExtent2D extent)>;

    struct RendererConfig {
        Uint32 MaxFramesInFlight = 2;
        String AppName = "MobileGL-VulkanRenderer";
        Version Version = MG_Config::CoreVersion;
        Uint64 CacheVersion = MG_Config::CacheVersion;
        Bool EnableValidationLayers = true;
    };

    class VulkanRenderer {
    public:
        VulkanRenderer(NativeWindowType window, const RendererConfig& cfg = {});
        ~VulkanRenderer();

        void Initialize();
        void Shutdown();

        void Render();
        void Present();

    private:
        struct QueueFamilyIndices {
            Int32 graphicsFamily = -1;
            Int32 presentFamily = -1;
        };

        struct SwapchainCapabilities {
            VkSurfaceCapabilitiesKHR capabilities;
            Vector<VkSurfaceFormatKHR> surfaceFormats;
            Vector<VkPresentModeKHR> presentModes;

            Bool IsComplete() const {
                return !surfaceFormats.empty() && !presentModes.empty();
            }
        };

        struct PhysicalDevice {
            QueueFamilyIndices queueFamilies;
            VkPhysicalDeviceProperties properties;
            SwapchainCapabilities swapchainCapabilities;
            VkPhysicalDevice handle = VK_NULL_HANDLE;

            Bool IsComplete() const {
                return handle != VK_NULL_HANDLE &&
                    queueFamilies.graphicsFamily != -1 &&
                    queueFamilies.presentFamily != -1;
            }
        };

        NativeWindowType m_window = 0;
        RendererConfig m_config;

        // Vulkan objects
        Bool m_validationLayersEnabled = false;
        Vector<VkExtensionProperties> m_extensions;
        VkInstance m_instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
        PhysicalDevice m_physicalDevice;
        // VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        VkSurfaceFormatKHR m_swapchainSurfaceFormat;
        Vector<VkImage> m_swapchainImages;
        VkExtent2D m_swapChainExtent;
        Vector<VkImageView> m_swapChainImageViews;

        // Vector<VkQueueFamilyProperties> m_queueFamilies;
        // QueueFamilyIndices m_queueFamilyIndices;

        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;

        VkCommandPool m_commandPool = VK_NULL_HANDLE;
        VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        Vector<VkFramebuffer> m_framebuffers;

        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_pipeline = VK_NULL_HANDLE;

        void CreateInstance();
        VkResult SetupDebugMessenger();
        VkResult DestroyDebugMessenger();
        VkDebugUtilsMessengerCreateInfoEXT PopulateDebugMessengerCreateInfo();
        void CreateSurface();
        void PickPhysicalDevice();
        void CreateLogicalDeviceAndQueues();
        void CreateSwapchain();
        void CreateSwapchainImageViews();
        void CreateCommandPool();
        void CreateCommandBuffer();
        void CreateDefaultRenderPass();
        void PrepareDemoPipeline();

        static Int GetPresentQueueFamilyIndex(
            const PhysicalDevice& physicalDevice, VkSurfaceKHR surface,
            const Vector<VkQueueFamilyProperties>& queueFamilies, Int preferredFamilyIndex = -1);
        static Vector<VkQueueFamilyProperties> GetQueueFamilyFromPhysicalDevice(VkPhysicalDevice device);
        static Int GetQueueFamilyIndex(const Vector<VkQueueFamilyProperties>& queueFamilies, VkQueueFlagBits flag);
        static Vector<VkExtensionProperties> EnumerateInstanceExtensions();
        static Bool IsNecessaryDeviceExtensionSupported(VkPhysicalDevice device);
        static SwapchainCapabilities GetSwapchainCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        static Bool GetMoreCapablePhysicalDevice(VkPhysicalDevice newVkDevice, VkSurfaceKHR surface, const PhysicalDevice& compareWithDevice, PhysicalDevice& outBetterDevice);
        static VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const Vector<VkSurfaceFormatKHR>& availableFormats);
        static VkPresentModeKHR ChooseSwapchainPresentMode(const Vector<VkPresentModeKHR>& availablePresentModes);
        static constexpr VkPresentModeKHR s_desiredPresentModes[] {
            VK_PRESENT_MODE_MAILBOX_KHR,
            VK_PRESENT_MODE_IMMEDIATE_KHR,
            VK_PRESENT_MODE_FIFO_RELAXED_KHR,
            VK_PRESENT_MODE_FIFO_KHR
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
