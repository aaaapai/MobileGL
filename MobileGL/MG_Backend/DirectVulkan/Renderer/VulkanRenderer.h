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

namespace MobileGL::MG_Backend::DirectVulkan {
    class VulkanContext;
    class SwapchainManager;
    class PipelineManager;
    struct FrameContext;

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
        NativeWindowType m_window = 0;
        RendererConfig m_config;

        // Vulkan objects
        Bool m_validationLayersEnabled = false;
        Vector<VkExtensionProperties> m_extensions;
        VkInstance m_instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        Int m_graphicsQueueFamilyIndex = -1;
        VkDevice m_device = VK_NULL_HANDLE;
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;

        void CreateInstance();
        void DestroyInstance();
        VkResult SetupDebugMessenger();
        VkResult DestroyDebugMessenger();
        VkDebugUtilsMessengerCreateInfoEXT PopulateDebugMessengerCreateInfo();
        void PickPhysicalDevice();
        void CreateLogicalDevice();

        static Int GetGraphicsQueueFamilyIndexOfPhysicalDevice(VkPhysicalDevice device);
        static Vector<VkExtensionProperties> EnumerateInstanceExtensions();
        static constexpr const char* s_validationLayerNames[] = {
            "VK_LAYER_KHRONOS_validation"
        };
        static Bool CheckValidationLayerSupport();

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
