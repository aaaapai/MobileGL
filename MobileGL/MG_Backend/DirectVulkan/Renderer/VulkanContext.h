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

        void Initialize(ANativeWindow* window, const std::string& appName = "VulkanEngineApp");
        void Shutdown();

        VkInstance GetInstance() const { return Instance; }
        VkPhysicalDevice GetPhysicalDevice() const { return PhysicalDevice; }
        VkDevice GetDevice() const { return Device; }
        VkQueue GetGraphicsQueue() const { return GraphicsQueue; }
        uint32_t GetGraphicsQueueFamily() const { return GraphicsQueueFamily; }
        VkSurfaceKHR GetSurface() const { return Surface; }

    private:
        void CreateInstance(const std::string& appName);
        void CreateSurface(ANativeWindow* window);
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