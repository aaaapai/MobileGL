// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanContext.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>

static inline void ThrowIfFailed(VkResult r, const char* msg) {
    if (r != VK_SUCCESS) {
        MGLOG_E("%s (VkResult=%d)", msg, int(r));
        throw MobileGL::RuntimeError(msg);
    }
}

namespace MobileGL::MG_Backend::DirectVulkan {
    class VulkanContext {
    public:
        VulkanContext() = default;
        ~VulkanContext();

        void Initialize(NativeWindowType window, const std::string& appName = "MobileGL-VulkanRenderer");
        void Shutdown();

        VkInstance GetInstance() const { return Instance; }
        VkPhysicalDevice GetPhysicalDevice() const { return PhysicalDevice; }
        VkDevice GetDevice() const { return Device; }
        VkQueue GetGraphicsQueue() const { return GraphicsQueue; }
        uint32_t GetGraphicsQueueFamily() const { return GraphicsQueueFamily; }
        VkSurfaceKHR GetSurface() const { return Surface; }

    private:
        void CreateInstance(const std::string& appName);
        void CreateSurface(NativeWindowType window);
        void PickPhysicalDevice();
        void CreateLogicalDevice();

        VkInstance Instance = VK_NULL_HANDLE;
        VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
        VkDevice Device = VK_NULL_HANDLE;
        VkQueue GraphicsQueue = VK_NULL_HANDLE;
        uint32_t GraphicsQueueFamily = UINT32_MAX;
        VkSurfaceKHR Surface = VK_NULL_HANDLE;
        bool Initialized = false;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
