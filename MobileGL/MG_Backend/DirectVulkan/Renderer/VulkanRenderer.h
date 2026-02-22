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
#include "UniformDescriptorBinder.h"
#include "VertexInputStateFactory.h"
#include "VkBufferObject.h"
#include "VkFramebufferManager.h"
#include "VkRenderPassManager.h"
#include "VkTextureSamplerManager.h"
#include "MG_Util/Math/VectorTypes.h"
#include <Includes.h>
#include <vk_mem_alloc.h>

#include "../VkIncludes.h"

namespace MobileGL::MG_State::GLState {
    class FramebufferObject;
    class ProgramObject;
    class VertexArrayObject;
} // namespace MobileGL::MG_State::GLState

namespace MobileGL::MG_Backend::DirectVulkan {
    struct DrawArrayPayload {
        GLenum mode = GL_TRIANGLES;
        GLint first = 0;
        GLsizei count = 0;
        const MG_State::GLState::ProgramObject* program = nullptr;
        const MG_State::GLState::VertexArrayObject* vertexArray = nullptr;
    };

    struct DrawElementPayload {
        DrawArrayPayload drawArray;
        GLenum indexType = GL_UNSIGNED_SHORT;
        SizeT indexByteOffset = 0;
        GLint baseVertex = 0;
    };

    struct QueueFamilyIndices {
        Int32 graphicsFamily = -1;
        Int32 presentFamily = -1;
    };

    struct PhysicalDevice {
        QueueFamilyIndices queueFamilies;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDevice handle = VK_NULL_HANDLE;

        Bool IsComplete() const {
            return handle != VK_NULL_HANDLE && queueFamilies.graphicsFamily != -1 && queueFamilies.presentFamily != -1;
        }
    };

    class VulkanRenderer {
    public:
        VulkanRenderer(NativeWindowType window, const VulkanRendererConfig& cfg = {});
        ~VulkanRenderer();

        void Initialize();
        void Shutdown();

        void RequestClear(GLbitfield mask, const FloatVec4& color, Float depth, Uint32 stencil,
                          Uint drawFboExternalIndex, Bool isDefaultFramebufferTarget);
        Bool ConsumePendingColorClear(VkClearColorValue& outClearColor);
        void EnsureFrameRecordingStarted();
        void DrawArrays(const DrawArrayPayload& payload);
        void DrawElements(const DrawElementPayload& payload);
        void MultiDrawElements(const Vector<DrawElementPayload>& payloads);
        Bool BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1,
                             GLint dstY1, GLbitfield mask, GLenum filter, Uint readFboExternalIndex,
                             Uint drawFboExternalIndex, Bool readIsDefaultFramebuffer, Bool drawIsDefaultFramebuffer);
        void Render();
        void Present();

        const PhysicalDevice& GetPhysicalDevice() const;
        Bool IsDrawIndirectCountExtensionEnabled() const;

        void RecreateSwapchain();

    private:
        struct PendingClearState {
            GLbitfield mask = 0;
            VkClearColorValue color = {{0.0f, 0.0f, 0.0f, 1.0f}};
            Float depth = 1.0f;
            Uint32 stencil = 0;
            Uint drawFboExternalIndex = 0;
            Bool targetsDefaultFramebuffer = true;
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
        Bool m_drawIndirectCountExtensionEnabled = false;
        using PFNDrawIndexedIndirectCountFunc = void(VKAPI_PTR*)(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, Uint32 maxDrawCount,
                                                                 Uint32 stride);
        PFNDrawIndexedIndirectCountFunc m_cmdDrawIndexedIndirectCount = nullptr;

        VkCommandPool m_commandPool = VK_NULL_HANDLE;

        VkFormat m_depthStencilFormat = VK_FORMAT_UNDEFINED;
        Vector<VkImage> m_depthStencilImages;
        Vector<VkDeviceMemory> m_depthStencilImageMemories;
        Vector<VkImageView> m_depthStencilImageViews;
        Vector<VkImageLayout> m_depthStencilImageLayouts;

        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        Vector<VkBufferObject> m_frameVertexUploadBuffers;
        Vector<VkDeviceSize> m_frameVertexUploadHeads;
        Vector<VkBufferObject> m_frameIndexUploadBuffers;
        Vector<VkDeviceSize> m_frameIndexUploadHeads;
        Vector<Vector<VkBufferObject>> m_deferredBufferReleases;

        Uint m_imageIndexAcquired = 0;
        FrameContext m_frameContext;
        UnorderedMap<Uint64, PendingClearState> m_pendingClears;
        Bool m_isMainRenderPassActive = false;
        VkRenderPass m_activeRenderPass = VK_NULL_HANDLE;
        VkExtent2D m_activeRenderExtent = {0, 0};
        VkFormat m_activeDepthStencilFormat = VK_FORMAT_UNDEFINED;
        Bool m_activeRenderTargetIsDefault = true;
        Uint m_activeDrawFboExternalIndex = 0;

        UniquePtr<PipelineFactory> m_pipelineFactory;
        UniquePtr<ProgramFactory> m_programFactory;
        UniquePtr<UniformDescriptorBinder> m_uniformDescriptorBinder;
        UniquePtr<VertexInputStateFactory> m_vertexInputStateFactory;
        UniquePtr<VkFramebufferManager> m_framebufferManager;
        UniquePtr<VkRenderPassManager> m_renderPassManager;
        UniquePtr<VkTextureSamplerManager> m_textureSamplerManager;

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
        VkRenderPass GetDefaultLoadRenderPass() const;
        Bool GetDefaultRenderTargetForCurrentImage(VkRenderPass& outRenderPass, VkFramebuffer& outFramebuffer,
                                                   VkExtent2D& outExtent, VkFormat& outDepthStencilFormat) const;
        Bool EnsureOffscreenRenderTarget(Uint glFboExternalIndex, const MG_State::GLState::FramebufferObject& glFbo,
                                         VkRenderPass& outRenderPass, VkFramebuffer& outFramebuffer,
                                         VkExtent2D& outExtent, VkFormat& outDepthStencilFormat);
        void PrepareDemoPipeline();
        VkPipeline GetOrCreatePipeline(const MG_State::GLState::ProgramObject& program, VkPipelineLayout pipelineLayout,
                                       Uint64 vertexInputHash,
                                       const VkPipelineVertexInputStateCreateInfo& vertexInputState);
        void TransitionSwapchainImageToColorAttachment(VkCommandBuffer commandBuffer, Uint32 imageIndex);
        void TransitionDepthStencilImageToAttachment(VkCommandBuffer commandBuffer, Uint32 imageIndex);
        void EndFrameRecordingIfNeeded();
        void DeferDestroyBuffer(VkBufferObject& buffer);
        void CollectDeferredBufferReleases(Uint32 frameIndex);
        Bool EnsureFrameUploadBufferCapacity(Uint32 frameIndex, Bool isIndexBuffer, VkDeviceSize requiredEndOffset,
                                             VkDeviceSize minCapacity, VkBufferUsageFlags usage);
        Bool UploadAndBindVertexStreams(const VertexInputStateFactory::BackendVertexInputState& vertexInputState,
                                        const DrawArrayPayload& payload, VkCommandBuffer commandBuffer);

        void ShutdownSwapchain();

        static Int GetPresentQueueFamilyIndex(const PhysicalDevice& physicalDevice, VkSurfaceKHR surface,
                                              const Vector<VkQueueFamilyProperties>& queueFamilies,
                                              Int preferredFamilyIndex = -1);
        static Vector<VkQueueFamilyProperties> GetQueueFamilyFromPhysicalDevice(VkPhysicalDevice device);
        static Int GetQueueFamilyIndex(const Vector<VkQueueFamilyProperties>& queueFamilies, VkQueueFlagBits flag);
        static Vector<VkExtensionProperties> EnumerateInstanceExtensions();
        static Vector<VkExtensionProperties> EnumerateDeviceExtensions(VkPhysicalDevice device);
        static Bool IsExtensionSupported(const Vector<VkExtensionProperties>& availableExtensions,
                                         const char* extensionName);
        static Bool IsExtensionAlreadyEnabled(const Vector<const char*>& enabledExtensions, const char* extensionName);
        static Bool EnableOptionalDeviceExtension(const Vector<VkExtensionProperties>& availableExtensions,
                                                  Vector<const char*>& inOutEnabledExtensions,
                                                  const char* extensionName);
        void ResolveOptionalDeviceExtensions(const Vector<VkExtensionProperties>& availableExtensions,
                                             Vector<const char*>& inOutEnabledExtensions);
        static Bool IsNecessaryDeviceExtensionSupported(VkPhysicalDevice device);
        static Bool GetMoreCapablePhysicalDevice(VkPhysicalDevice newVkDevice, VkSurfaceKHR surface,
                                                 const PhysicalDevice& compareWithDevice,
                                                 PhysicalDevice& outBetterDevice);
        Uint32 FindMemoryType(Uint32 typeFilter, VkMemoryPropertyFlags properties) const;
        static Uint64 BuildPendingClearKey(Uint drawFboExternalIndex, Bool targetsDefaultFramebuffer);
        static Bool HasStencilComponent(VkFormat format);
        static VkFormat FindSupportedDepthStencilFormat(VkPhysicalDevice physicalDevice);
        static constexpr VkDynamicState s_dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        static constexpr const char* s_validationLayerNames[] = {"VK_LAYER_KHRONOS_validation"};
        static constexpr const char* s_deviceExtensionNames[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        static Bool CheckValidationLayerSupport();

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                            VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                            void* pUserData);
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
