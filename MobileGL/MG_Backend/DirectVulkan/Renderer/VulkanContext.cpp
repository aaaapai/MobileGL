// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanContext.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VulkanContext.h"
#include "MG_Util/Types.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    VulkanContext::~VulkanContext() {
        Shutdown();
    }

    void VulkanContext::Initialize(NativeWindowType window, const std::string& appName) {
        if (Initialized) return;
        CreateInstance(appName);
        CreateSurface(window);
        PickPhysicalDevice();
        CreateLogicalDevice();
        Initialized = true;
        MGLOG_D("VulkanContext initialized");
    }

    void VulkanContext::Shutdown() {
        if (!Initialized && Instance == VK_NULL_HANDLE) return;
        if (Device != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(Device);
            vkDestroyDevice(Device, nullptr);
            Device = VK_NULL_HANDLE;
        }
        if (Surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(Instance, Surface, nullptr);
            Surface = VK_NULL_HANDLE;
        }
        if (Instance != VK_NULL_HANDLE) {
            vkDestroyInstance(Instance, nullptr);
            Instance = VK_NULL_HANDLE;
        }
        Initialized = false;
        MGLOG_D("VulkanContext shutdown");
    }

    void VulkanContext::CreateInstance(const std::string& appName) {
        VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        appInfo.pApplicationName = appName.c_str();
        appInfo.apiVersion = VK_API_VERSION_1_0;

        const char* exts[] = {VK_KHR_SURFACE_EXTENSION_NAME,
#if __ANDROID__
                              VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#endif
        }; // TODO: support more platforms

        VkInstanceCreateInfo ci{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        ci.pApplicationInfo = &appInfo;
        ci.enabledExtensionCount = 2;
        ci.ppEnabledExtensionNames = exts;

        ThrowIfFailed(vkCreateInstance(&ci, nullptr, &Instance), "vkCreateInstance failed");
    }

    void VulkanContext::CreateSurface(NativeWindowType window) {
#if __ANDROID__
        if (!Instance) throw MobileGL::RuntimeError("Instance not created");

        auto* nativeWindow = static_cast<ANativeWindow*>(window);
        if (!nativeWindow) throw MobileGL::RuntimeError("ANativeWindowType is null");

        VkAndroidSurfaceCreateInfoKHR sci{VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};
        sci.window = nativeWindow;
        ThrowIfFailed(vkCreateAndroidSurfaceKHR(Instance, &sci, nullptr, &Surface), "vkCreateAndroidSurfaceKHR failed");
#else
        MGLOG_W("VulkanRenderer::Initialize called on a platform which is not supported yet"); // TODO: support more
                                                                                               // platforms
#endif
    }

    void VulkanContext::PickPhysicalDevice() {
        uint32_t count = 0;
        ThrowIfFailed(vkEnumeratePhysicalDevices(Instance, &count, nullptr), "vkEnumeratePhysicalDevices count");
        if (count == 0) throw MobileGL::RuntimeError("No physical devices");
        std::vector<VkPhysicalDevice> devs(count);
        ThrowIfFailed(vkEnumeratePhysicalDevices(Instance, &count, devs.data()), "vkEnumeratePhysicalDevices");

        for (auto d : devs) {
            uint32_t qcount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(d, &qcount, nullptr);
            std::vector<VkQueueFamilyProperties> qprops(qcount);
            vkGetPhysicalDeviceQueueFamilyProperties(d, &qcount, qprops.data());
            for (uint32_t i = 0; i < qcount; ++i) {
                VkBool32 present = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(d, i, Surface, &present);
                if ((qprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present) {
                    PhysicalDevice = d;
                    GraphicsQueueFamily = i;
                    MGLOG_D("Picked physical device, queue family %u", i);
                    return;
                }
            }
        }
        throw MobileGL::RuntimeError("No suitable physical device");
    }

    void VulkanContext::CreateLogicalDevice() {
        float prio = 1.0f;
        VkDeviceQueueCreateInfo dqci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        dqci.queueFamilyIndex = GraphicsQueueFamily;
        dqci.queueCount = 1;
        dqci.pQueuePriorities = &prio;

        const char* devExts[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        dci.queueCreateInfoCount = 1;
        dci.pQueueCreateInfos = &dqci;
        dci.enabledExtensionCount = 1;
        dci.ppEnabledExtensionNames = devExts;

        ThrowIfFailed(vkCreateDevice(PhysicalDevice, &dci, nullptr, &Device), "vkCreateDevice failed");
        vkGetDeviceQueue(Device, GraphicsQueueFamily, 0, &GraphicsQueue);
        MGLOG_D("Logical device created");
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
