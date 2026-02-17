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
#include "PipelineFactory.h"
#include "ProgramFactory.h"
#include "SwapchainObject.h"
#include "VkBufferObject.h"
#include "MG_Util/Math/VectorTypes.h"
#include <Includes.h>
#include <vk_mem_alloc.h>

#include "../VkIncludes.h"

namespace MobileGL::MG_State::GLState {
    class VertexArrayObject;
}

namespace MobileGL::MG_Backend::DirectVulkan {
    class VertexInputStateFactory;

    struct DrawArrayPayload {
        GLenum mode = GL_TRIANGLES;
        GLint first = 0;
        GLsizei count = 0;
        const MG_State::GLState::VertexArrayObject* vertexArray = nullptr;
        Bool hasPositionStream = false;
        const void* positionData = nullptr;
        SizeT positionDataSizeBytes = 0;
        SizeT positionStrideBytes = 0;
        SizeT positionOffsetBytes = 0;
        GLenum positionType = GL_FLOAT;
        GLint positionSize = 0;
        Bool positionNormalized = false;
    };

    struct DrawElementPayload {
        DrawArrayPayload drawArray;
        GLenum indexType = GL_UNSIGNED_SHORT;
        const void* indexData = nullptr;
        SizeT indexDataSizeBytes = 0;
    };

    class VulkanRenderer {
    public:
        VulkanRenderer(NativeWindowType window, const VulkanRendererConfig& cfg = {});
        ~VulkanRenderer();

        void Initialize();
        void Shutdown();

        void RequestClear(GLbitfield mask, const FloatVec4& color, Float depth, Uint32 stencil);
        Bool ConsumePendingColorClear(VkClearColorValue& outClearColor);
        void EnsureFrameRecordingStarted();
        void DrawArrays(const DrawArrayPayload& payload);
        void DrawElements(const DrawElementPayload& payload);
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
        VmaAllocator m_allocator = nullptr;
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
        VkFormat m_depthStencilFormat = VK_FORMAT_UNDEFINED;
        Vector<VkImage> m_depthStencilImages;
        Vector<VkDeviceMemory> m_depthStencilImageMemories;
        Vector<VkImageView> m_depthStencilImageViews;
        Vector<VkImageLayout> m_depthStencilImageLayouts;

        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_pipeline = VK_NULL_HANDLE;
        Uint64 m_demoProgramHash = 0;
        Vector<VkPipelineShaderStageCreateInfo> m_demoPipelineStages;
        VkBufferObject m_vertexBuffer;
        VkBufferObject m_indexBuffer;

        Uint m_imageIndexAcquired = 0;
        FrameContext m_frameContext;
        GLbitfield m_pendingClearMask = 0;
        VkClearColorValue m_pendingClearColor = {{0.0f, 0.0f, 0.0f, 1.0f}};
        Float m_pendingClearDepth = 1.0f;
        Uint32 m_pendingClearStencil = 0;
        Bool m_isMainRenderPassActive = false;

        UniquePtr<PipelineFactory> m_pipelineFactory;
        UniquePtr<ProgramFactory> m_programFactory;
        UniquePtr<VertexInputStateFactory> m_vertexInputStateFactory;

        void CreateInstance();
        VkResult SetupDebugMessenger();
        VkResult DestroyDebugMessenger();
        VkDebugUtilsMessengerCreateInfoEXT PopulateDebugMessengerCreateInfo();
        void CreateSurface();
        void PickPhysicalDevice();
        void CreateLogicalDeviceAndQueues();
        void CreateAllocator();
        void DestroyAllocator();
        void CreateSwapchain();
        void CreateCommandPool();
        void CreateFrameContexts();
        void CreateDepthStencilResources();
        void DestroyDepthStencilResources();
        void CreateDefaultRenderPass();
        VkRenderPass CreateDefaultRenderPass(VkAttachmentLoadOp loadOp);
        void CreateDefaultFramebuffers();
        void PrepareDemoPipeline();
        VkPipeline GetOrCreatePipeline(Uint64 programHash, Uint64 vertexInputHash,
                                       const VkPipelineVertexInputStateCreateInfo& vertexInputState);
        void TransitionSwapchainImageToColorAttachment(VkCommandBuffer commandBuffer, Uint32 imageIndex);
        void TransitionDepthStencilImageToAttachment(VkCommandBuffer commandBuffer, Uint32 imageIndex);
        void RecordColorClear(VkCommandBuffer commandBuffer, const VkClearColorValue& clearColor);
        void RecordDepthStencilClear(VkCommandBuffer commandBuffer, GLbitfield mask, Float depth, Uint32 stencil);
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
        Uint32 FindMemoryType(Uint32 typeFilter, VkMemoryPropertyFlags properties) const;
        static Bool HasStencilComponent(VkFormat format);
        static VkFormat FindSupportedDepthStencilFormat(VkPhysicalDevice physicalDevice);
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
