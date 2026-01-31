#pragma once
#include <Includes.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    class VulkanContext;

    class SwapchainManager {
    public:
        SwapchainManager(VulkanContext& ctx);
        ~SwapchainManager();

        void Initialize();
        void Recreate();
        void Cleanup();

        VkSwapchainKHR GetSwapchain() const { return Swapchain; }
        VkFormat GetFormat() const { return ImageFormat; }
        VkExtent2D GetExtent() const { return Extent; }
        const std::vector<VkImageView>& GetImageViews() const { return ImageViews; }
        const std::vector<VkFramebuffer>& GetFramebuffers() const { return Framebuffers; }

        void SetFramebuffers(std::vector<VkFramebuffer>&& fbs);

    private:
        VulkanContext& Ctx;
        VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
        VkFormat ImageFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D Extent{0, 0};
        std::vector<VkImage> Images;
        std::vector<VkImageView> ImageViews;
        std::vector<VkFramebuffer> Framebuffers;

        void CreateSwapchainInternal();
        void CreateImageViews();
        void DestroyImageViews();
    };
} // namespace MobileGL::MG_Backend::DirectVulkan