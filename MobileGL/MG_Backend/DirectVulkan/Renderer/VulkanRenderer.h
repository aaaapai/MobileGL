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
#include "VkClearManager.h"
#include "VkRenderPassManager.h"
#include "VkSamplerManager.h"
#include "VkTextureManager.h"
#include "MG_Util/Math/VectorTypes.h"
#include <Includes.h>
#include <vk_mem_alloc.h>

#include "../VkIncludes.h"

namespace MobileGL::MG_State::GLState {
    class FramebufferObject;
    class ProgramObject;
    class SamplerObject;
    class VertexArrayObject;
} // namespace MobileGL::MG_State::GLState

namespace MobileGL::MG_Backend::DirectVulkan {
    enum class DrawSetupAspect: Uint8 {
        FramebufferObject = 1 << 0,
        VertexArrayObject = 1 << 1,
        UniformBuffer     = 1 << 2,
        VertexBuffer      = 1 << 3,
        IndexBuffer       = 1 << 4,
        Viewport          = 1 << 5,
        Scissor           = 1 << 6,
    };

    struct DrawArrayCmd {
        GLenum mode = GL_TRIANGLES;
        GLint first = 0;
        GLsizei count = 0;
    };

    struct DrawElementCmd: public DrawArrayCmd {
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

        void SetupDraw(FrameContext::FrameData& frame, GLenum mode, Flags<DrawSetupAspect> aspects);
        void ClearAttachmentsOnActiveRenderPass(VkCommandBuffer commandBuffer,
                                                const RenderPassEntry& compatibleRenderPassEntry);

        void Clear(GLbitfield mask);
        void BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                             GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                             GLbitfield mask, GLenum filter);
        void DrawArrays(const DrawArrayCmd& payload);
        void DrawElements(const DrawElementCmd& payload);
        void MultiDrawElements(const Vector<DrawElementCmd>& payloads);
        void Present();

        const PhysicalDevice& GetPhysicalDevice() const;
        Bool IsDrawIndirectCountExtensionEnabled() const;

        void RecreateSwapchain();

    private:
        struct BlitUniformData {
            float srcRect[4] = {0.f, 0.f, 1.f, 1.f};
            float dstRect[4] = {0.f, 0.f, 1.f, 1.f};
            Int surfaceTransform = 0;
            Int padding[3] = {0, 0, 0};
        };

        struct BlitResources {
            SharedPtr<MG_State::GLState::ProgramObject> program;
            SharedPtr<MG_State::GLState::SamplerObject> nearestSampler;
            SharedPtr<MG_State::GLState::SamplerObject> linearSampler;
            Int srcRectLocation = -1;
            Int dstRectLocation = -1;
            Int surfaceTransformLocation = -1;
            Uint32 samplerBinding = 0;
        };

        NativeWindowType m_window = 0;
        VulkanRendererConfig m_config;

        // Vulkan objects
        Bool m_validationLayersEnabled = false;
        Vector<VkExtensionProperties> m_extensions;
        VkInstance m_instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
        PhysicalDevice m_physicalDevice;
        VkDevice m_device = VK_NULL_HANDLE;
        VmaAllocator m_allocator = nullptr;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        SwapchainObject m_swapchainObject;

        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;
        Bool m_drawIndirectCountExtensionEnabled = false;
        using PFNDrawIndexedIndirectCountFunc = void(VKAPI_PTR*)(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, Uint32 maxDrawCount,
                                                                 Uint32 stride);
        PFNDrawIndexedIndirectCountFunc m_cmdDrawIndexedIndirectCount = nullptr;

        VkCommandPool m_commandPool = VK_NULL_HANDLE;

        Vector<VkBufferObject> m_frameVertexUploadBuffers;
        Vector<VkDeviceSize> m_frameVertexUploadHeads;
        Vector<VkBufferObject> m_frameIndexUploadBuffers;
        Vector<VkDeviceSize> m_frameIndexUploadHeads;
        Vector<Vector<VkBufferObject>> m_deferredBufferReleases;

        Uint m_imageIndexAcquired = 0;
        FrameContext m_frameContext;

        UniquePtr<PipelineFactory> m_pipelineFactory;
        UniquePtr<ProgramFactory> m_programFactory;
        UniquePtr<UniformDescriptorBinder> m_uniformDescriptorBinder;
        UniquePtr<VertexInputStateFactory> m_vertexInputStateFactory;
        UniquePtr<VkClearManager> m_clearManager;
        UniquePtr<VkRenderPassManager> m_renderPassManager;
        UniquePtr<VkTextureManager> m_textureManager;
        UniquePtr<VkSamplerManager> m_samplerManager;
        BlitResources m_blitResources;

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

        VkPipeline GetOrCreatePipeline(
            GLenum mode,
            const MG_State::GLState::ProgramObject& program,
            const MG_State::GLState::VertexArrayObject& vao,
            const RenderPassEntry& renderPassEntry);

        void DeferDestroyBuffer(VkBufferObject& buffer);
        void CollectDeferredBufferReleases(Uint32 frameIndex);
        Bool EnsureFrameUploadBufferCapacity(Uint32 frameIndex, Bool isIndexBuffer, VkDeviceSize requiredEndOffset,
                                             VkDeviceSize minCapacity, VkBufferUsageFlags usage);
        Bool UploadAndBindVertexStreams(VkCommandBuffer commandBuffer, const MG_State::GLState::VertexArrayObject& vao);
        Bool InitializeBlitResources();
        void ShutdownBlitResources();
        Bool TryBlitToDefaultFramebufferWithShader(FrameContext::FrameData& frame,
                                                   MG_State::GLState::FramebufferObject& readFbo,
                                                   MG_State::GLState::FramebufferObject& drawFbo,
                                                   GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                                                   GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                                                   GLenum filter);
        VkPipeline GetOrCreateBlitPipeline(const RenderPassEntry& renderPassEntry);

        void ShutdownSwapchain();

        // Static functions
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
        static Uint64 BuildPendingClearKey(Uint drawFboExternalIndex, Bool targetsDefaultFramebuffer);
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
