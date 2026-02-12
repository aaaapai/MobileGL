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
        MGLOG_D("VulkanRenderer initialized");
    }

    void VulkanRenderer::Shutdown() {
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
            MGLOG_I("    Ignored physical device: Some of the required device extension not supported on this device.");
            return false;
        }

        // Check swapchain capabilities
        newDevice.swapchainCapabilities = GetSwapchainCapabilities(newVkDevice, surface);
        if (!newDevice.swapchainCapabilities.IsComplete()) {
            outBetterDevice = otherDevice;
            MGLOG_I("    Ignored physical device: Swapchain capabilities not met.");
            return false;
        }

        // Check queue families
        Vector<VkQueueFamilyProperties> queueFamilies = GetQueueFamilyFromPhysicalDevice(newVkDevice);
        newDevice.queueFamilies.graphicsFamily = GetQueueFamilyIndex(queueFamilies, VK_QUEUE_GRAPHICS_BIT);
        if (newDevice.queueFamilies.graphicsFamily == -1) {
            outBetterDevice = otherDevice;
            MGLOG_I("    Ignored physical device: No graphics queue family.");
            return false;
        }

        newDevice.queueFamilies.presentFamily =
            GetPresentQueueFamilyIndex(newDevice, surface, queueFamilies, newDevice.queueFamilies.graphicsFamily);
        if (newDevice.queueFamilies.presentFamily == -1) {
            outBetterDevice = otherDevice;
            MGLOG_I("    Ignored physical device: No present queue family.");
            return false;
        }

        // Pick discrete GPU
        if (newDevice.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            otherDevice.properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            outBetterDevice = newDevice;
            MGLOG_I("    Picked physical device: Discrete GPU.");
            return true;
        }

        // Pick integrated GPU if no discrete GPU
        if (newDevice.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU &&
            otherDevice.properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            outBetterDevice = newDevice;
            MGLOG_I("    Picked physical device: Integrated GPU and no discrete one found yet.");
            return true;
        }

        // Ignore other GPU when discrete GPU found
        if (newDevice.properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            otherDevice.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            outBetterDevice = newDevice;
            MGLOG_I("    Ignored physical device: Already picked discrete GPU.");
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
        for (const auto& presentMode : availablePresentModes) {
            if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                return presentMode;
            } else if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return presentMode;
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

    void VulkanRenderer::CreateSwapchain() {}

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
