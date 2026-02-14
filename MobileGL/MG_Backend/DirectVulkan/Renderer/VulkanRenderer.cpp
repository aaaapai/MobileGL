// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VulkanRenderer.h"

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

    VulkanRenderer::VulkanRenderer(NativeWindowType window, const RendererConfig& cfg) : m_window(window), m_config(cfg) {
        // Initialize();
    }

    VulkanRenderer::~VulkanRenderer() {
        Shutdown();
    }

    void VulkanRenderer::Initialize() {
        CreateInstance();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDeviceAndQueues();
        CreateSwapchain();
        CreateSwapchainImageViews();
        CreateCommandPool();
        CreateCommandBuffer();
        CreateDefaultRenderPass();
        MGLOG_D("VulkanRenderer initialized");
    }

    void VulkanRenderer::Shutdown() {
        for (auto fb : m_framebuffers) {
            vkDestroyFramebuffer(m_device, fb, nullptr);
        }
        m_framebuffers.clear();

        if (m_renderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(m_device, m_renderPass, nullptr);
            m_renderPass = VK_NULL_HANDLE;
        }

        if (m_commandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(m_device, m_commandPool, nullptr);
            m_commandPool = VK_NULL_HANDLE;
        }

        for (auto imageView : m_swapChainImageViews) {
            vkDestroyImageView(m_device, imageView, nullptr);
        }
        m_swapChainImageViews.clear();

        if (m_swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
            m_swapchain = VK_NULL_HANDLE;
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

    }

    void VulkanRenderer::Present() {

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
            MOBILEGL_ASSERT(false, "Validation layers requested but not available!");
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
#else
#warning "VulkanContext::CreateInstance: VK_KHR_*_surface extension not defined on this platform"
#endif
}; // TODO: support more platforms

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

        if (m_validationLayersEnabled)
            VK_VERIFY(SetupDebugMessenger());
    }

    VkResult VulkanRenderer::SetupDebugMessenger() {
        auto createInfo = PopulateDebugMessengerCreateInfo();
        auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
        if (!vkCreateDebugUtilsMessengerEXT)
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        VK_VERIFY(vkCreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger));
        return VK_SUCCESS;
    }

    VkResult VulkanRenderer::DestroyDebugMessenger() {
        if (m_debugMessenger != VK_NULL_HANDLE) {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
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
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
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

    Bool VulkanRenderer::GetMoreCapablePhysicalDevice(
        VkPhysicalDevice newVkDevice, VkSurfaceKHR surface, const PhysicalDevice& otherDevice, PhysicalDevice& outBetterDevice) {
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
        MGLOG_I("    %s (Vulkan %d.%d.%d, %s)",
            deviceProperties.deviceName,
            VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion),
            deviceTypeToStr(deviceProperties.deviceType));

        // Check device extensions (including swapchain extension)
        Bool deviceExtSupported = IsNecessaryDeviceExtensionSupported(newVkDevice);
        if (!deviceExtSupported) {
            outBetterDevice = otherDevice;
            MGLOG_I("    Ignored physical device. (Reason: Some of the required device extension not supported on this device)");
            return false;
        }

        // Check swapchain capabilities
        newDevice.swapchainCapabilities = GetSwapchainCapabilities(newVkDevice, surface);
        if (!newDevice.swapchainCapabilities.IsComplete()) {
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

    VkSurfaceFormatKHR VulkanRenderer::ChooseSwapchainSurfaceFormat(
        const Vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        // TODO: Properly rank other formats
        return availableFormats[0];
    }

    VkPresentModeKHR VulkanRenderer::ChooseSwapchainPresentMode(
        const Vector<VkPresentModeKHR>& availablePresentModes) {
        for (auto desiredPresentMode : s_desiredPresentModes) {
            for (const auto& presentMode : availablePresentModes) {
                if (presentMode == desiredPresentMode) {
                    return presentMode;
                }
            }
        }

        // TODO: Properly rank other modes
        return availablePresentModes[0];
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

    VulkanRenderer::SwapchainCapabilities VulkanRenderer::GetSwapchainCapabilities(VkPhysicalDevice device, VkSurfaceKHR surface) {
        SwapchainCapabilities swapchainCapabilities;

        VK_VERIFY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapchainCapabilities.capabilities));
        Uint32 formatCount;
        VK_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr));

        if (formatCount != 0) {
            swapchainCapabilities.surfaceFormats.resize(formatCount);
            VK_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapchainCapabilities.surfaceFormats.data()));
        }

        Uint32 presentModeCount;
        VK_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr));

        if (presentModeCount != 0) {
            swapchainCapabilities.presentModes.resize(presentModeCount);
            VK_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, swapchainCapabilities.presentModes.data()));
        }

        return swapchainCapabilities;
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
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(std::size(s_deviceExtensionNames));
        deviceCreateInfo.ppEnabledExtensionNames = s_deviceExtensionNames;
        VK_VERIFY(vkCreateDevice(m_physicalDevice.handle, &deviceCreateInfo, nullptr, &m_device), "vkCreateDevice");
        MGLOG_I("Logical device created.");

        // Queues
        vkGetDeviceQueue(m_device, m_physicalDevice.queueFamilies.graphicsFamily, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, m_physicalDevice.queueFamilies.presentFamily, 0, &m_presentQueue);
        MGLOG_I("Queues got successfully.");
    }

    void VulkanRenderer::CreateSwapchain() {
        auto surfaceFormatToStr = [](VkFormat format) {
            switch (format) {
                ENUM_STR_CASE(VK_FORMAT_UNDEFINED)
                ENUM_STR_CASE(VK_FORMAT_R4G4_UNORM_PACK8)
                ENUM_STR_CASE(VK_FORMAT_R4G4B4A4_UNORM_PACK16)
                ENUM_STR_CASE(VK_FORMAT_B4G4R4A4_UNORM_PACK16)
                ENUM_STR_CASE(VK_FORMAT_R5G6B5_UNORM_PACK16)
                ENUM_STR_CASE(VK_FORMAT_B5G6R5_UNORM_PACK16)
                ENUM_STR_CASE(VK_FORMAT_R5G5B5A1_UNORM_PACK16)
                ENUM_STR_CASE(VK_FORMAT_B5G5R5A1_UNORM_PACK16)
                ENUM_STR_CASE(VK_FORMAT_A1R5G5B5_UNORM_PACK16)
                ENUM_STR_CASE(VK_FORMAT_R8_UNORM)
                ENUM_STR_CASE(VK_FORMAT_R8_SNORM)
                ENUM_STR_CASE(VK_FORMAT_R8_USCALED)
                ENUM_STR_CASE(VK_FORMAT_R8_SSCALED)
                ENUM_STR_CASE(VK_FORMAT_R8_UINT)
                ENUM_STR_CASE(VK_FORMAT_R8_SINT)
                ENUM_STR_CASE(VK_FORMAT_R8_SRGB)
                ENUM_STR_CASE(VK_FORMAT_R8G8_UNORM)
                ENUM_STR_CASE(VK_FORMAT_R8G8_SNORM)
                ENUM_STR_CASE(VK_FORMAT_R8G8_USCALED)
                ENUM_STR_CASE(VK_FORMAT_R8G8_SSCALED)
                ENUM_STR_CASE(VK_FORMAT_R8G8_UINT)
                ENUM_STR_CASE(VK_FORMAT_R8G8_SINT)
                ENUM_STR_CASE(VK_FORMAT_R8G8_SRGB)
                ENUM_STR_CASE(VK_FORMAT_R8G8B8_UNORM)
                ENUM_STR_CASE(VK_FORMAT_R8G8B8_SNORM)
                ENUM_STR_CASE(VK_FORMAT_R8G8B8_USCALED)
                ENUM_STR_CASE(VK_FORMAT_R8G8B8_SSCALED)
                ENUM_STR_CASE(VK_FORMAT_R8G8B8_UINT)
                ENUM_STR_CASE(VK_FORMAT_R8G8B8_SINT)
                ENUM_STR_CASE(VK_FORMAT_R8G8B8_SRGB)
                ENUM_STR_CASE(VK_FORMAT_B8G8R8_UNORM)
                ENUM_STR_CASE(VK_FORMAT_B8G8R8_SNORM)
                ENUM_STR_CASE(VK_FORMAT_B8G8R8_USCALED)
                ENUM_STR_CASE(VK_FORMAT_B8G8R8_SSCALED)
                ENUM_STR_CASE(VK_FORMAT_B8G8R8_UINT)
                ENUM_STR_CASE(VK_FORMAT_B8G8R8_SINT)
                ENUM_STR_CASE(VK_FORMAT_B8G8R8_SRGB)
                ENUM_STR_CASE(VK_FORMAT_R8G8B8A8_UNORM)
                ENUM_STR_CASE(VK_FORMAT_R8G8B8A8_SNORM)
                ENUM_STR_CASE(VK_FORMAT_R8G8B8A8_USCALED)
                ENUM_STR_CASE(VK_FORMAT_R8G8B8A8_SSCALED)
                ENUM_STR_CASE(VK_FORMAT_R8G8B8A8_UINT)
                ENUM_STR_CASE(VK_FORMAT_R8G8B8A8_SINT)
                ENUM_STR_CASE(VK_FORMAT_R8G8B8A8_SRGB)
                ENUM_STR_CASE(VK_FORMAT_B8G8R8A8_UNORM)
                ENUM_STR_CASE(VK_FORMAT_B8G8R8A8_SNORM)
                ENUM_STR_CASE(VK_FORMAT_B8G8R8A8_USCALED)
                ENUM_STR_CASE(VK_FORMAT_B8G8R8A8_SSCALED)
                ENUM_STR_CASE(VK_FORMAT_B8G8R8A8_UINT)
                ENUM_STR_CASE(VK_FORMAT_B8G8R8A8_SINT)
                ENUM_STR_CASE(VK_FORMAT_B8G8R8A8_SRGB)
                ENUM_STR_CASE(VK_FORMAT_A8B8G8R8_UNORM_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A8B8G8R8_SNORM_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A8B8G8R8_USCALED_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A8B8G8R8_SSCALED_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A8B8G8R8_UINT_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A8B8G8R8_SINT_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A8B8G8R8_SRGB_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A2R10G10B10_UNORM_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A2R10G10B10_SNORM_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A2R10G10B10_USCALED_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A2R10G10B10_SSCALED_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A2R10G10B10_UINT_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A2R10G10B10_SINT_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A2B10G10R10_UNORM_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A2B10G10R10_SNORM_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A2B10G10R10_USCALED_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A2B10G10R10_SSCALED_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A2B10G10R10_UINT_PACK32)
                ENUM_STR_CASE(VK_FORMAT_A2B10G10R10_SINT_PACK32)
                ENUM_STR_CASE(VK_FORMAT_R16_UNORM)
                ENUM_STR_CASE(VK_FORMAT_R16_SNORM)
                ENUM_STR_CASE(VK_FORMAT_R16_USCALED)
                ENUM_STR_CASE(VK_FORMAT_R16_SSCALED)
                ENUM_STR_CASE(VK_FORMAT_R16_UINT)
                ENUM_STR_CASE(VK_FORMAT_R16_SINT)
                ENUM_STR_CASE(VK_FORMAT_R16_SFLOAT)
                ENUM_STR_CASE(VK_FORMAT_R16G16_UNORM)
                ENUM_STR_CASE(VK_FORMAT_R16G16_SNORM)
                ENUM_STR_CASE(VK_FORMAT_R16G16_USCALED)
                ENUM_STR_CASE(VK_FORMAT_R16G16_SSCALED)
                ENUM_STR_CASE(VK_FORMAT_R16G16_UINT)
                ENUM_STR_CASE(VK_FORMAT_R16G16_SINT)
                ENUM_STR_CASE(VK_FORMAT_R16G16_SFLOAT)
                ENUM_STR_CASE(VK_FORMAT_R16G16B16_UNORM)
                ENUM_STR_CASE(VK_FORMAT_R16G16B16_SNORM)
                ENUM_STR_CASE(VK_FORMAT_R16G16B16_USCALED)
                ENUM_STR_CASE(VK_FORMAT_R16G16B16_SSCALED)
                ENUM_STR_CASE(VK_FORMAT_R16G16B16_UINT)
                ENUM_STR_CASE(VK_FORMAT_R16G16B16_SINT)
                ENUM_STR_CASE(VK_FORMAT_R16G16B16_SFLOAT)
                ENUM_STR_CASE(VK_FORMAT_R16G16B16A16_UNORM)
                ENUM_STR_CASE(VK_FORMAT_R16G16B16A16_SNORM)
                ENUM_STR_CASE(VK_FORMAT_R16G16B16A16_USCALED)
                ENUM_STR_CASE(VK_FORMAT_R16G16B16A16_SSCALED)
                ENUM_STR_CASE(VK_FORMAT_R16G16B16A16_UINT)
                ENUM_STR_CASE(VK_FORMAT_R16G16B16A16_SINT)
                ENUM_STR_CASE(VK_FORMAT_R16G16B16A16_SFLOAT)
                ENUM_STR_CASE(VK_FORMAT_R32_UINT)
                ENUM_STR_CASE(VK_FORMAT_R32_SINT)
                ENUM_STR_CASE(VK_FORMAT_R32_SFLOAT)
                ENUM_STR_CASE(VK_FORMAT_R32G32_UINT)
                ENUM_STR_CASE(VK_FORMAT_R32G32_SINT)
                ENUM_STR_CASE(VK_FORMAT_R32G32_SFLOAT)
                ENUM_STR_CASE(VK_FORMAT_R32G32B32_UINT)
                ENUM_STR_CASE(VK_FORMAT_R32G32B32_SINT)
                ENUM_STR_CASE(VK_FORMAT_R32G32B32_SFLOAT)
                ENUM_STR_CASE(VK_FORMAT_R32G32B32A32_UINT)
                ENUM_STR_CASE(VK_FORMAT_R32G32B32A32_SINT)
                ENUM_STR_CASE(VK_FORMAT_R32G32B32A32_SFLOAT)
                ENUM_STR_CASE(VK_FORMAT_R64_UINT)
                ENUM_STR_CASE(VK_FORMAT_R64_SINT)
                ENUM_STR_CASE(VK_FORMAT_R64_SFLOAT)
                ENUM_STR_CASE(VK_FORMAT_R64G64_UINT)
                ENUM_STR_CASE(VK_FORMAT_R64G64_SINT)
                ENUM_STR_CASE(VK_FORMAT_R64G64_SFLOAT)
                ENUM_STR_CASE(VK_FORMAT_R64G64B64_UINT)
                ENUM_STR_CASE(VK_FORMAT_R64G64B64_SINT)
                ENUM_STR_CASE(VK_FORMAT_R64G64B64_SFLOAT)
                ENUM_STR_CASE(VK_FORMAT_R64G64B64A64_UINT)
                ENUM_STR_CASE(VK_FORMAT_R64G64B64A64_SINT)
                ENUM_STR_CASE(VK_FORMAT_R64G64B64A64_SFLOAT)
                ENUM_STR_CASE(VK_FORMAT_B10G11R11_UFLOAT_PACK32)
                ENUM_STR_CASE(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)
                ENUM_STR_CASE(VK_FORMAT_D16_UNORM)
                ENUM_STR_CASE(VK_FORMAT_X8_D24_UNORM_PACK32)
                ENUM_STR_CASE(VK_FORMAT_D32_SFLOAT)
                ENUM_STR_CASE(VK_FORMAT_S8_UINT)
                ENUM_STR_CASE(VK_FORMAT_D16_UNORM_S8_UINT)
                ENUM_STR_CASE(VK_FORMAT_D24_UNORM_S8_UINT)
                ENUM_STR_CASE(VK_FORMAT_D32_SFLOAT_S8_UINT)
                ENUM_STR_CASE(VK_FORMAT_BC1_RGB_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_BC1_RGB_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_BC1_RGBA_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_BC1_RGBA_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_BC2_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_BC2_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_BC3_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_BC3_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_BC4_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_BC4_SNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_BC5_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_BC5_SNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_BC6H_UFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_BC6H_SFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_BC7_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_BC7_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_EAC_R11_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_EAC_R11_SNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_EAC_R11G11_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_EAC_R11G11_SNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_4x4_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_4x4_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_5x4_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_5x4_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_5x5_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_5x5_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_6x5_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_6x5_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_6x6_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_6x6_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_8x5_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_8x5_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_8x6_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_8x6_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_8x8_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_8x8_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_10x5_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_10x5_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_10x6_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_10x6_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_10x8_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_10x8_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_10x10_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_10x10_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_12x10_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_12x10_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_12x12_UNORM_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_12x12_SRGB_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_G8B8G8R8_422_UNORM)
                ENUM_STR_CASE(VK_FORMAT_B8G8R8G8_422_UNORM)
                ENUM_STR_CASE(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM)
                ENUM_STR_CASE(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM)
                ENUM_STR_CASE(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM)
                ENUM_STR_CASE(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM)
                ENUM_STR_CASE(VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM)
                ENUM_STR_CASE(VK_FORMAT_R10X6_UNORM_PACK16)
                ENUM_STR_CASE(VK_FORMAT_R10X6G10X6_UNORM_2PACK16)
                ENUM_STR_CASE(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16)
                ENUM_STR_CASE(VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16)
                ENUM_STR_CASE(VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16)
                ENUM_STR_CASE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16)
                ENUM_STR_CASE(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16)
                ENUM_STR_CASE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16)
                ENUM_STR_CASE(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16)
                ENUM_STR_CASE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16)
                ENUM_STR_CASE(VK_FORMAT_R12X4_UNORM_PACK16)
                ENUM_STR_CASE(VK_FORMAT_R12X4G12X4_UNORM_2PACK16)
                ENUM_STR_CASE(VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16)
                ENUM_STR_CASE(VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16)
                ENUM_STR_CASE(VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16)
                ENUM_STR_CASE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16)
                ENUM_STR_CASE(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16)
                ENUM_STR_CASE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16)
                ENUM_STR_CASE(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16)
                ENUM_STR_CASE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16)
                ENUM_STR_CASE(VK_FORMAT_G16B16G16R16_422_UNORM)
                ENUM_STR_CASE(VK_FORMAT_B16G16R16G16_422_UNORM)
                ENUM_STR_CASE(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM)
                ENUM_STR_CASE(VK_FORMAT_G16_B16R16_2PLANE_420_UNORM)
                ENUM_STR_CASE(VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM)
                ENUM_STR_CASE(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM)
                ENUM_STR_CASE(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM)
                ENUM_STR_CASE(VK_FORMAT_G8_B8R8_2PLANE_444_UNORM)
                ENUM_STR_CASE(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16)
                ENUM_STR_CASE(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16)
                ENUM_STR_CASE(VK_FORMAT_G16_B16R16_2PLANE_444_UNORM)
                ENUM_STR_CASE(VK_FORMAT_A4R4G4B4_UNORM_PACK16)
                ENUM_STR_CASE(VK_FORMAT_A4B4G4R4_UNORM_PACK16)
                ENUM_STR_CASE(VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK)
                ENUM_STR_CASE(VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG)
                ENUM_STR_CASE(VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG)
                ENUM_STR_CASE(VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG)
                ENUM_STR_CASE(VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG)
                ENUM_STR_CASE(VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG)
                ENUM_STR_CASE(VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG)
                ENUM_STR_CASE(VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG)
                ENUM_STR_CASE(VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG)
                ENUM_STR_CASE(VK_FORMAT_R16G16_SFIXED5_NV)
                ENUM_STR_CASE(VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR)
                ENUM_STR_CASE(VK_FORMAT_A8_UNORM_KHR)
                default:
                    return "UNKNOWN_FORMAT";
            }
        };

        auto colorSpaceToStr = [](VkColorSpaceKHR cs) {
            switch (cs) {
                ENUM_STR_CASE(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
                ENUM_STR_CASE(VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT)
                ENUM_STR_CASE(VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT)
                ENUM_STR_CASE(VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT)
                ENUM_STR_CASE(VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT)
                ENUM_STR_CASE(VK_COLOR_SPACE_BT709_LINEAR_EXT)
                ENUM_STR_CASE(VK_COLOR_SPACE_BT709_NONLINEAR_EXT)
                ENUM_STR_CASE(VK_COLOR_SPACE_BT2020_LINEAR_EXT)
                ENUM_STR_CASE(VK_COLOR_SPACE_HDR10_ST2084_EXT)
                ENUM_STR_CASE(VK_COLOR_SPACE_DOLBYVISION_EXT)
                ENUM_STR_CASE(VK_COLOR_SPACE_HDR10_HLG_EXT)
                ENUM_STR_CASE(VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT)
                ENUM_STR_CASE(VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT)
                ENUM_STR_CASE(VK_COLOR_SPACE_PASS_THROUGH_EXT)
                ENUM_STR_CASE(VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT)
                ENUM_STR_CASE(VK_COLOR_SPACE_DISPLAY_NATIVE_AMD)
                default:
                    return "UNKNOWN_COLOR_SPACE";
            }
        };

        auto presentModeToStr = [](VkPresentModeKHR mode) {
            switch (mode) {
                ENUM_STR_CASE(VK_PRESENT_MODE_IMMEDIATE_KHR)
                ENUM_STR_CASE(VK_PRESENT_MODE_MAILBOX_KHR)
                ENUM_STR_CASE(VK_PRESENT_MODE_FIFO_KHR)
                ENUM_STR_CASE(VK_PRESENT_MODE_FIFO_RELAXED_KHR)
                ENUM_STR_CASE(VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR)
                ENUM_STR_CASE(VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR)
                default:
                    return "UNKNOWN_PRESENT_MODE";
            }
        };

        auto transformToStr = [](VkSurfaceTransformFlagBitsKHR transform) {
            switch (transform) {
                ENUM_STR_CASE(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
                ENUM_STR_CASE(VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR)
                ENUM_STR_CASE(VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR)
                ENUM_STR_CASE(VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
                ENUM_STR_CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR)
                ENUM_STR_CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR)
                ENUM_STR_CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR)
                ENUM_STR_CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR)
                ENUM_STR_CASE(VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR)
                default:
                    return "UNKNOWN_TRANSFORM";
            }
        };

        auto& supportedSurfaceFormats = m_physicalDevice.swapchainCapabilities.surfaceFormats;
        MGLOG_I("Got %d surface formats: ", supportedSurfaceFormats.size());
        for (auto& surfaceFormat : supportedSurfaceFormats) {
            MGLOG_I("    [%s, %s]", surfaceFormatToStr(surfaceFormat.format), colorSpaceToStr(surfaceFormat.colorSpace));
        }
        m_swapchainSurfaceFormat = ChooseSwapchainSurfaceFormat(supportedSurfaceFormats);
        MGLOG_I("Picked surface format: [%s, %s]", surfaceFormatToStr(m_swapchainSurfaceFormat.format), colorSpaceToStr(m_swapchainSurfaceFormat.colorSpace));

        auto& supportedPresentModes = m_physicalDevice.swapchainCapabilities.presentModes;
        MGLOG_I("Got %d present mode: ", supportedPresentModes.size());
        for (auto& presentMode : supportedPresentModes) {
            MGLOG_I("    %s", presentModeToStr(presentMode));
        }
        auto presentMode = ChooseSwapchainPresentMode(supportedPresentModes);
        MGLOG_I("Picked present mode: %s", presentModeToStr(presentMode));

        const auto& swapchainCaps = m_physicalDevice.swapchainCapabilities.capabilities;
        auto imageCount = std::max<uint32_t>(2, swapchainCaps.minImageCount);
        MGLOG_I("Set minImageCount = %u", imageCount);

        VkSwapchainCreateInfoKHR sci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        sci.surface = m_surface;
        sci.minImageCount = imageCount;
        sci.imageFormat = m_swapchainSurfaceFormat.format;
        sci.imageColorSpace = m_swapchainSurfaceFormat.colorSpace;
        sci.imageExtent = swapchainCaps.currentExtent;
        sci.imageArrayLayers = 1;
        sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        Uint32 queueFamilyIndices[] = { (Uint)m_physicalDevice.queueFamilies.graphicsFamily, (Uint)m_physicalDevice.queueFamilies.presentFamily };
        if (m_physicalDevice.queueFamilies.graphicsFamily != m_physicalDevice.queueFamilies.presentFamily) {
            sci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            sci.queueFamilyIndexCount = 2;
            sci.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            sci.queueFamilyIndexCount = 0; // Optional
            sci.pQueueFamilyIndices = nullptr; // Optional
        }
        sci.preTransform = swapchainCaps.currentTransform;
        MGLOG_I("Swapchain currentTransform = %s", transformToStr(swapchainCaps.currentTransform));
        MGLOG_I("Set swapchain preTransform = %s", transformToStr(sci.preTransform));
        sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        sci.presentMode = presentMode;
        sci.clipped = VK_TRUE;
        sci.oldSwapchain = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateSwapchainKHR(m_device, &sci, nullptr, &m_swapchain));

        Uint32 gotImageCount = 0;
        VK_VERIFY(vkGetSwapchainImagesKHR(m_device, m_swapchain, &gotImageCount, nullptr));
        m_swapchainImages.resize(gotImageCount);
        VK_VERIFY(vkGetSwapchainImagesKHR(m_device, m_swapchain, &gotImageCount, m_swapchainImages.data()));
        m_swapChainExtent = swapchainCaps.currentExtent;

        MGLOG_I("Swapchain created, extent = %dx%d", m_swapChainExtent.width, m_swapChainExtent.height);
    }

    void VulkanRenderer::CreateSwapchainImageViews() {
        m_swapChainImageViews.resize(m_swapchainImages.size());
        for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_swapchainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_swapchainSurfaceFormat.format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            VK_VERIFY(vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImageViews[i]));
        }
        MGLOG_I("Swapchain image views created");
    }

    void VulkanRenderer::CreateCommandPool() {
        VkCommandPoolCreateInfo createInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = m_physicalDevice.queueFamilies.graphicsFamily;
        VK_VERIFY(vkCreateCommandPool(m_device, &createInfo, nullptr, &m_commandPool));
        MGLOG_I("Command pool created");
    }

    void VulkanRenderer::CreateCommandBuffer() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        VK_VERIFY(vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer));
        MGLOG_I("Command buffer created");
    }

    void VulkanRenderer::CreateDefaultRenderPass() {
        VkAttachmentDescription color{};
        color.format = m_swapchainSurfaceFormat.format;
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

        // Create framebuffers now (use swapchain imageviews)
        const auto& imageViews = m_swapChainImageViews;
        Vector<VkFramebuffer>& fbs = m_framebuffers;
        fbs.reserve(imageViews.size());
        for (auto iv : imageViews) {
            VkImageView attachments[] = {iv};
            VkFramebufferCreateInfo fbci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            fbci.renderPass = m_renderPass;
            fbci.attachmentCount = 1;
            fbci.pAttachments = attachments;
            fbci.width = m_swapChainExtent.width;
            fbci.height = m_swapChainExtent.height;
            fbci.layers = 1;
            VkFramebuffer fb;
            VK_VERIFY(vkCreateFramebuffer(m_device, &fbci, nullptr, &fb), "vkCreateFramebuffer");
            fbs.push_back(fb);
        }
        MGLOG_D("RenderPass created and framebuffers set");
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
#else
        //#warning "VulkanRenderer::Initialize called on a platform which is not supported yet"
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

    Int VulkanRenderer::GetPresentQueueFamilyIndex(
        const PhysicalDevice& physicalDevice, VkSurfaceKHR surface,
        const Vector<VkQueueFamilyProperties>& queueFamilies, Int preferredFamilyIndex) {
        if (preferredFamilyIndex != -1) {
            VkBool32 supportsPresent = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.handle, preferredFamilyIndex, surface, &supportsPresent);
            if (supportsPresent)
                return preferredFamilyIndex;
        }

        for (Uint32 i = 0; i < queueFamilies.size(); i++) {
            VkBool32 supportsPresent = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.handle, i, surface, &supportsPresent);
            if (supportsPresent)
                return i;
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

} // namespace MobileGL::MG_Backend::DirectVulkan
