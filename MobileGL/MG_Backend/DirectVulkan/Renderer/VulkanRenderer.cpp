// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRenderer.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VulkanRenderer.h"
#include "VertexInputStateFactory.h"
#include "VertexInputStateBuilder.h"

#include "MG_State/GLState/Core.h"
#include "MG_State/GLState/ProgramState/ProgramObject.h"
#include "MG_State/GLState/ProgramState/ShaderObject.h"
#include "MG_State/GLState/SamplerState/SamplerObject.h"
#include "MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.h"
#include "MG_Util/Converters/MGToVk/RenderStateEnumConverter.h"
#include <vulkan/vulkan_core.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    static const char* VkImageLayoutToString(VkImageLayout layout) {
        switch (layout) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                return "VK_IMAGE_LAYOUT_UNDEFINED";
            case VK_IMAGE_LAYOUT_GENERAL:
                return "VK_IMAGE_LAYOUT_GENERAL";
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                return "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL";
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                return "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL";
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                return "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL";
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                return "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL";
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                return "VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL";
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                return "VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL";
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                return "VK_IMAGE_LAYOUT_PRESENT_SRC_KHR";
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
                return "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL";
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                return "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL";
            default:
                return "VK_IMAGE_LAYOUT_OTHER";
        }
    }

    static Bool ActiveRenderPassUsesTexture(const ActiveRenderPassInfo& activeRenderPass,
                                            const MG_State::GLState::ITextureObject& texture) {
        for (const auto& trackedAttachment : activeRenderPass.trackedAttachmentLayouts) {
            if (trackedAttachment.target != TrackedAttachmentTarget::Texture) {
                continue;
            }
            if (trackedAttachment.texture == &texture) {
                return true;
            }
        }
        return false;
    }

    static Bool IsValidSampledImageLayout(VkImageLayout layout) {
        switch (layout) {
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            case VK_IMAGE_LAYOUT_GENERAL:
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                return true;
            default:
                return false;
        }
    }

    namespace {
        static constexpr Uint kHiddenBlitProgramId = 0xFFFFFFF0u;
        static constexpr Uint kHiddenBlitVertexShaderId = 0xFFFFFFF1u;
        static constexpr Uint kHiddenBlitFragmentShaderId = 0xFFFFFFF2u;
        static constexpr Uint kHiddenBlitNearestSamplerId = 0xFFFFFFF3u;
        static constexpr Uint kHiddenBlitLinearSamplerId = 0xFFFFFFF4u;

        enum class BlitSurfaceTransform : Uint32 {
            Identity = 0,
            Rotate90 = 1,
            Rotate180 = 2,
            Rotate270 = 3,
        };

        struct BlitImageBinding {
            VkImage image = VK_NULL_HANDLE;
            VkImageLayout* trackedLayout = nullptr;
            VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_NONE;
            IntVec2 extent = {0, 0};
            Uint32 mipLevel = 0;
            Uint32 mipLevelCount = 1;
            const char* label = nullptr;
        };

        static void GetImageTransitionSourceState(VkImageLayout oldLayout, VkPipelineStageFlags& outSrcStageMask,
                                                  VkAccessFlags& outSrcAccessMask) {
            switch (oldLayout) {
                case VK_IMAGE_LAYOUT_UNDEFINED:
                case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                    outSrcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    outSrcAccessMask = 0;
                    break;
                case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                    outSrcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    outSrcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    break;
                case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                    outSrcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    outSrcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    break;
                case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                    outSrcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    outSrcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    break;
                case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                    outSrcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                    outSrcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    break;
                default:
                    outSrcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                    outSrcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
                    break;
            }
        }

        static Bool ResolveColorBlitBinding(MG_State::GLState::FramebufferObject& fbo, Bool isReadFramebuffer,
                                            Uint32 swapchainImageIndex, SwapchainObject& swapchainObject,
                                            VkTextureManager& textureManager, BlitImageBinding& outBinding) {
            const Bool isDefaultFbo = (&fbo == MG_Impl::GLImpl::FramebufferImpl::pDefaultFramebufferInfo->defaultFBO.get());
            const FramebufferAttachmentType attachmentType =
                isReadFramebuffer ? fbo.GetReadBuffer() : fbo.GetDrawBuffers()[0];
            if (attachmentType < FramebufferAttachmentType::Color0 || attachmentType > FramebufferAttachmentType::Color31) {
                MGLOG_E("BlitFramebuffer only supports color attachments right now (attachment=%d)",
                        static_cast<Int>(attachmentType));
                return false;
            }

            const auto& attachment = fbo.GetAttachment(attachmentType);
            if (!attachment.IsComplete()) {
                MGLOG_E("BlitFramebuffer skipped: %s framebuffer color attachment is incomplete",
                        isReadFramebuffer ? "read" : "draw");
                return false;
            }
            if (attachment.IsRenderbuffer()) {
                MGLOG_E("BlitFramebuffer skipped: renderbuffer attachments are not supported yet");
                return false;
            }
            if (!attachment.IsTexture()) {
                MGLOG_E("BlitFramebuffer skipped: unsupported framebuffer attachment type");
                return false;
            }

            auto* texture = attachment.GetTexture().get();
            MOBILEGL_ASSERT(texture != nullptr, "ResolveColorBlitBinding: texture attachment is null");
            outBinding.label = isReadFramebuffer ? "read" : "draw";

            if (isDefaultFbo) {
                outBinding.image = swapchainObject.GetImage(swapchainImageIndex);
                outBinding.trackedLayout = nullptr;
                outBinding.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                const auto extent = swapchainObject.GetExtent();
                outBinding.extent = {static_cast<Int>(extent.width), static_cast<Int>(extent.height)};
                outBinding.mipLevel = 0;
                outBinding.mipLevelCount = 1;
                return true;
            }

            auto* resource = textureManager.SyncTextureAndGetDescriptor(*texture);
            if (resource == nullptr) {
                MGLOG_E("BlitFramebuffer skipped: failed to sync %s framebuffer textureId=%d",
                        outBinding.label, texture->GetExternalIndex());
                return false;
            }
            if ((resource->aspect & VK_IMAGE_ASPECT_COLOR_BIT) == 0) {
                MGLOG_E("BlitFramebuffer skipped: %s framebuffer attachment textureId=%d is not a color image",
                        outBinding.label, texture->GetExternalIndex());
                return false;
            }

            outBinding.image = resource->image;
            outBinding.trackedLayout = &resource->layout;
            outBinding.aspectMask = resource->aspect;
            const auto attachmentExtent = attachment.GetSize();
            outBinding.extent = {attachmentExtent.x(), attachmentExtent.y()};
            outBinding.mipLevel = static_cast<Uint32>(std::max(attachment.GetTextureLevel(), 0));
            outBinding.mipLevelCount = resource->mipLevels;
            return true;
        }

        static BlitSurfaceTransform ToBlitSurfaceTransform(VkSurfaceTransformFlagBitsKHR preTransform) {
            switch (preTransform) {
                case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:
                    return BlitSurfaceTransform::Rotate90;
                case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:
                    return BlitSurfaceTransform::Rotate180;
                case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:
                    return BlitSurfaceTransform::Rotate270;
                default:
                    return BlitSurfaceTransform::Identity;
            }
        }
    } // namespace

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

    VulkanRenderer::VulkanRenderer(NativeWindowType window, const VulkanRendererConfig& cfg)
        : m_window(window), m_config(cfg) {
        // Initialize();
    }

    VulkanRenderer::~VulkanRenderer() {
        Shutdown();
    }

    inline ProgramFactory::CompileOptionFlags GetShaderTransformFlags(VkSurfaceTransformFlagBitsKHR preTransform) {
        ProgramFactory::CompileOptionFlags flags = ProgramFactory::CompileOptionBit::PositionZRemap;
        const auto& currentDrawFBO =
            MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw).GetBoundObject();
        if (currentDrawFBO == MG_Impl::GLImpl::FramebufferImpl::pDefaultFramebufferInfo->defaultFBO) {
            flags |= ProgramFactory::CompileOptionBit::PositionYFlip;
            switch (preTransform) {
            case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:
                flags |= ProgramFactory::CompileOptionBit::SurfaceRotate90;
                break;
            case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:
                flags |= ProgramFactory::CompileOptionBit::SurfaceRotate180;
                break;
            case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:
                flags |= ProgramFactory::CompileOptionBit::SurfaceRotate270;
                break;
            default:
                break;
            }
        }
        return flags;
    }

    void VulkanRenderer::Initialize() {
        CreateInstance();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDeviceAndQueues();
        CreateAllocator();

        CreateCommandPool();
        VK_VERIFY(m_frameContext.Initialize(m_device, m_commandPool, m_config.MaxFramesInFlight),
                  "CreateFrameContexts");
        MGLOG_I("CreateFrameContexts completed");
        auto succeeded = false;
        succeeded = m_bufferManager.Initialize({
            .allocator = m_allocator,
            .frameCount = m_frameContext.GetFrameCount(),
            .minVertexUploadBytes = 4 * 1024 * 1024,
            .minIndexUploadBytes = 1 * 1024 * 1024,
            .minUniformUploadBytes = 4 * 1024 * 1024,
            .transientMemoryUsage = VMA_MEMORY_USAGE_AUTO,
            .transientAllocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .transientPersistentMapping = false,
        });
        MOBILEGL_ASSERT(succeeded, "VkBufferManager initialization failed.");
        m_textureManager = MakeUnique<VkTextureManager>();
        MOBILEGL_ASSERT(m_textureManager != nullptr, "VkTextureManager creation failed.");
        succeeded = m_textureManager->Initialize(
            {m_device, m_physicalDevice.handle, m_allocator, m_commandPool, m_graphicsQueue});
        MOBILEGL_ASSERT(succeeded, "VkTextureManager initialization failed.");
        m_clearManager = MakeUnique<VkClearManager>();
        MOBILEGL_ASSERT(m_clearManager != nullptr, "VkClearManager creation failed.");
        succeeded = m_clearManager->Initialize();
        MOBILEGL_ASSERT(succeeded, "VkClearManager initialization failed.");
        m_renderPassManager =
            MakeUnique<VkRenderPassManager>(m_device, m_config, *m_clearManager, *m_textureManager, m_swapchainObject);
        MOBILEGL_ASSERT(m_renderPassManager != nullptr, "VkRenderPassManager creation failed.");
        succeeded = m_renderPassManager->Initialize();
        MOBILEGL_ASSERT(succeeded, "VkRenderPassManager initialization failed.");

        RecreateSwapchain();

        m_pipelineFactory = MakeUnique<PipelineFactory>(m_device, m_config);
        MOBILEGL_ASSERT(m_pipelineFactory != nullptr, "PipelineFactory creation failed.");
        m_programFactory = MakeUnique<ProgramFactory>(m_device, m_config);
        MOBILEGL_ASSERT(m_programFactory != nullptr, "ProgramFactory creation failed.");

        m_samplerManager = MakeUnique<VkSamplerManager>();
        MOBILEGL_ASSERT(m_samplerManager != nullptr, "VkSamplerManager creation failed.");
        succeeded = m_samplerManager->Initialize({m_device, &m_config});
        MOBILEGL_ASSERT(succeeded, "VkSamplerManager initialization failed.");
        succeeded = InitializeBlitResources();
        MOBILEGL_ASSERT(succeeded, "Blit pipeline resource initialization failed.");

        m_uniformDescriptorBinder = MakeUnique<UniformDescriptorBinder>();
        MOBILEGL_ASSERT(m_uniformDescriptorBinder != nullptr, "UniformDescriptorBinder creation failed.");
        succeeded = m_uniformDescriptorBinder->Initialize(m_device, &m_bufferManager,
                                                   m_physicalDevice.properties.limits.minUniformBufferOffsetAlignment,
                                                   m_config.MaxFramesInFlight, 16, 64,
                                                   m_textureManager.get(), m_samplerManager.get());
        MOBILEGL_ASSERT(succeeded, "UniformDescriptorBinder initialization failed.");
        m_vertexInputStateFactory = MakeUnique<VertexInputStateFactory>(m_config);
        MOBILEGL_ASSERT(m_vertexInputStateFactory != nullptr, "VertexInputStateFactory creation failed.");

        // Prime the first frame so Render() always targets an acquired swapchain image.
        VK_VERIFY(m_frameContext.WaitAndAcquireNextImage(m_device, m_swapchainObject.GetHandle(), m_imageIndexAcquired),
                  "Initialize, WaitAndAcquireNextImage");
        m_bufferManager.BeginFrame(m_frameContext.GetCurrentFrameIndex());

        MGLOG_D("VulkanRenderer initialized");
    }

    void VulkanRenderer::Shutdown() {
        VK_VERIFY(vkDeviceWaitIdle(m_device));

        m_pipelineFactory.reset();
        m_programFactory.reset();
        ShutdownBlitResources();
        if (m_samplerManager) {
            m_samplerManager->Shutdown();
            m_samplerManager.reset();
        }
        if (m_textureManager) {
            m_textureManager->Shutdown();
            m_textureManager.reset();
        }
        m_vertexInputStateFactory.reset();
        m_bufferManager.Shutdown();

        m_frameContext.Destroy(m_device, m_commandPool);

        if (m_uniformDescriptorBinder) {
            m_uniformDescriptorBinder->Shutdown();
            m_uniformDescriptorBinder.reset();
        }

        ShutdownSwapchain();
        m_renderPassManager.reset();
        if (m_clearManager) {
            m_clearManager->Shutdown();
            m_clearManager.reset();
        }
        if (m_commandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(m_device, m_commandPool, nullptr);
            m_commandPool = VK_NULL_HANDLE;
        }

        DestroyAllocator();

        if (m_device != VK_NULL_HANDLE) {
            vkDestroyDevice(m_device, nullptr);
            m_device = VK_NULL_HANDLE;
        }
        s_vkCmdDrawIndexedIndirectCount = nullptr;

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

    Bool VulkanRenderer::UploadAndBindVertexStreams(
        VkCommandBuffer commandBuffer, const MG_State::GLState::VertexArrayObject& vao) {
        auto& vertexInputState = m_vertexInputStateFactory->GetOrCreateVertexInputState(vao);

        const auto bindingCount = vertexInputState.bindings.size();

        Vector<VkBuffer> vkBuffers(bindingCount, VK_NULL_HANDLE);
        Vector<VkDeviceSize> vkOffsets(bindingCount, 0);
        const Uint32 frameIndex = m_frameContext.GetCurrentFrameIndex();

        auto findBufferByKey = [&](SizeT bufferKey) -> const MG_State::GLState::BufferObject* {
            const auto& attrs = vao.GetAllAttributes();
            for (Uint32 location = 0; location < MG_State::GLState::VertexArrayObject::MAX_VERTEX_ATTRIBS; ++location) {
                const auto& attr = attrs[location];
                if (!attr.Buffer) {
                    continue;
                }
                const auto* buffer = attr.Buffer.get();
                if (reinterpret_cast<SizeT>(buffer) == bufferKey) {
                    return buffer;
                }
            }
            return nullptr;
        };

        for (SizeT binding = 0; binding < bindingCount; ++binding) {
            const SizeT bufferKey = vertexInputState.bindingBufferKeys[binding];
            const MG_State::GLState::BufferObject* sourceBuffer = findBufferByKey(bufferKey);
            MOBILEGL_ASSERT(sourceBuffer != nullptr, "UploadAndBindVertexStreams failed to resolve source buffer");

            const auto sourceData = sourceBuffer->GetDataReadOnly();

            const SizeT sourceSize = sourceBuffer->GetSize();
            BufferSlice slice{};
            if (!m_bufferManager.UploadTransient(TransientBufferKind::Vertex, frameIndex, sourceData->data(),
                                                 static_cast<VkDeviceSize>(sourceSize), 16, slice)) {
                MGLOG_E("UploadAndBindVertexStreams skipped: failed to upload binding %zu", binding);
                return false;
            }
            vkBuffers[binding] = slice.buffer;
            vkOffsets[binding] = slice.offset;
        }

        vkCmdBindVertexBuffers(commandBuffer, 0, static_cast<Uint32>(bindingCount), vkBuffers.data(), vkOffsets.data());
        return true;
    }

    Bool VulkanRenderer::UploadAndBindIndexBuffer(FrameContext::FrameData& frame,
                                                  const MG_State::GLState::VertexArrayObject& vao,
                                                  const IndexBufferView* pIndexBufferView) {
        VkIndexType vkIndexType = VK_INDEX_TYPE_MAX_ENUM;
        switch (pIndexBufferView->indexType) {
        case GL_UNSIGNED_SHORT:
            vkIndexType = VK_INDEX_TYPE_UINT16;
            break;
        case GL_UNSIGNED_INT:
            vkIndexType = VK_INDEX_TYPE_UINT32;
            break;
        default:
            MGLOG_D("DrawElements skipped: index type %u is not supported yet", pIndexBufferView->indexType);
            return false;
        }

        const auto* indexBuffer = vao.GetIndexBufferBindingSlot().GetBoundObject().get();
        MOBILEGL_ASSERT(indexBuffer != nullptr, "UploadAndBindIndexBuffer requires bound EBO");
        const auto indexData = indexBuffer->GetDataReadOnly();
        MOBILEGL_ASSERT(indexData != nullptr && !indexData->empty(), "DrawElements requires non-empty EBO data");

        const SizeT indexSize = MG_Util::GetGLTypeSize(pIndexBufferView->indexType);
        const SizeT indexDataSizeBytes = pIndexBufferView->indexByteSize;
        MOBILEGL_ASSERT(pIndexBufferView->indexByteOffset + indexDataSizeBytes <= indexBuffer->GetSize(),
                        "DrawElements index range out of bounds");

        const Uint32 frameIndex = m_frameContext.GetCurrentFrameIndex();
        const VkDeviceSize alignment = indexSize;
        BufferSlice slice{};
        if (!m_bufferManager.UploadTransient(TransientBufferKind::Index, frameIndex,
                                             indexData->data() + pIndexBufferView->indexByteOffset,
                                             static_cast<VkDeviceSize>(indexDataSizeBytes), alignment, slice)) {
            MGLOG_E("DrawElements skipped: failed to prepare index upload buffer");
            return false;
        }
        vkCmdBindIndexBuffer(frame.commandBuffer, slice.buffer, slice.offset, vkIndexType);
        return true;
    }

    Bool VulkanRenderer::InitializeBlitResources() {
        ShutdownBlitResources();

        static constexpr const char* kVertexShaderSource = R"(#version 460 core
uniform vec4 uSrcRect;
uniform vec4 uDstRect;
uniform int uSurfaceTransform;

layout(location = 0) out vec2 vTexCoord;

vec2 ApplySurfaceTransform(vec2 position, int transform) {
    vec2 p = position;
    p.y = -p.y;
    if (transform == 1) {
        p = vec2(-p.y, p.x);
    } else if (transform == 2) {
        p = -p;
    } else if (transform == 3) {
        p = vec2(p.y, -p.x);
    }
    return p;
}

void main() {
    const vec2 uvTri[3] = vec2[](
        vec2(0.0, 0.0),
        vec2(2.0, 0.0),
        vec2(0.0, 2.0)
    );
    vec2 uv = uvTri[gl_VertexID];
    vec2 dst = uDstRect.xy + uv * uDstRect.zw;
    vec2 clip = dst * 2.0 - 1.0;
    clip = ApplySurfaceTransform(clip, uSurfaceTransform);
    gl_Position = vec4(clip, 0.0, 1.0);
    vTexCoord = uSrcRect.xy + uv * uSrcRect.zw;
}
)";
        static constexpr const char* kFragmentShaderSource = R"(#version 460 core
layout(binding = 0) uniform sampler2D uSource;
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(uSource, vTexCoord);
}
)";

        auto vertexShader = MakeShared<MG_State::GLState::ShaderObject>(ShaderStage::Vertex, kHiddenBlitVertexShaderId);
        vertexShader->SetShaderSource(kVertexShaderSource);
        vertexShader->Compile();
        if (!vertexShader->GetCompileStatus()) {
            MGLOG_E("InitializeBlitResources failed: vertex shader compile error: %s", vertexShader->GetInfoLog().c_str());
            return false;
        }

        auto fragmentShader = MakeShared<MG_State::GLState::ShaderObject>(ShaderStage::Fragment, kHiddenBlitFragmentShaderId);
        fragmentShader->SetShaderSource(kFragmentShaderSource);
        fragmentShader->Compile();
        if (!fragmentShader->GetCompileStatus()) {
            MGLOG_E("InitializeBlitResources failed: fragment shader compile error: %s", fragmentShader->GetInfoLog().c_str());
            return false;
        }

        m_blitResources.program = MakeShared<MG_State::GLState::ProgramObject>(kHiddenBlitProgramId);
        m_blitResources.program->AttachShader(vertexShader);
        m_blitResources.program->AttachShader(fragmentShader);
        m_blitResources.program->Link(false);
        if (!m_blitResources.program->GetLinkStatus()) {
            MGLOG_E("InitializeBlitResources failed: program link error: %s", m_blitResources.program->GetInfoLog().c_str());
            return false;
        }

        m_blitResources.srcRectLocation = m_blitResources.program->GetUniformLocation("uSrcRect");
        m_blitResources.dstRectLocation = m_blitResources.program->GetUniformLocation("uDstRect");
        m_blitResources.surfaceTransformLocation = m_blitResources.program->GetUniformLocation("uSurfaceTransform");
        MOBILEGL_ASSERT(m_blitResources.srcRectLocation >= 0, "InitializeBlitResources: missing uSrcRect");
        MOBILEGL_ASSERT(m_blitResources.dstRectLocation >= 0, "InitializeBlitResources: missing uDstRect");
        MOBILEGL_ASSERT(m_blitResources.surfaceTransformLocation >= 0,
                        "InitializeBlitResources: missing uSurfaceTransform");
        MOBILEGL_ASSERT(m_blitResources.program->GetUBOSize() > 0,
                        "InitializeBlitResources: blit program global UBO is empty");

        auto createSampler = [](Uint externalIndex, SamplerFilterMode filter) {
            auto sampler = MakeShared<MG_State::GLState::SamplerObject>(externalIndex);
            sampler->SetWrapS(SamplerWrapMode::ClampToEdge);
            sampler->SetWrapT(SamplerWrapMode::ClampToEdge);
            sampler->SetWrapR(SamplerWrapMode::ClampToEdge);
            sampler->SetMinFilter(filter);
            sampler->SetMagFilter(filter);
            sampler->SetMipmapMode(SamplerMipmapMode::None);
            sampler->SetLodRange(0.0f, 0.0f);
            return sampler;
        };

        m_blitResources.nearestSampler = createSampler(kHiddenBlitNearestSamplerId, SamplerFilterMode::Nearest);
        m_blitResources.linearSampler = createSampler(kHiddenBlitLinearSamplerId, SamplerFilterMode::Linear);
        m_blitResources.samplerBinding = 0;
        return true;
    }

    void VulkanRenderer::ShutdownBlitResources() {
        m_blitResources = {};
    }

    VkPipeline VulkanRenderer::GetOrCreateBlitPipeline(const RenderPassEntry& renderPassEntry) {
        MOBILEGL_ASSERT(m_blitResources.program != nullptr, "GetOrCreateBlitPipeline: blit program is null");
        MOBILEGL_ASSERT(m_programFactory != nullptr, "GetOrCreateBlitPipeline: program factory is null");
        MOBILEGL_ASSERT(m_uniformDescriptorBinder != nullptr, "GetOrCreateBlitPipeline: descriptor binder is null");

        static const VkPipelineVertexInputStateCreateInfo kEmptyVertexInputState {
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
        };
        ProgramFactory::CompileOptionFlags transformFlags = 0;
        auto& stages = m_programFactory->GetOrCreatePipelineShaderStages(*m_blitResources.program, transformFlags);
        PipelineFactory::PipelineCreatePayload payload{
            .programHash = m_programFactory->ComputeHash(*m_blitResources.program, transformFlags),
            .vertexInputHash = 0,
            .pipelineLayout = m_uniformDescriptorBinder->GetOrCreatePipelineLayout(*m_blitResources.program),
            .renderPass = renderPassEntry.renderPass,
            .subpass = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthTestEnable = false,
            .depthWriteEnable = false,
            .depthCompareOp = VK_COMPARE_OP_ALWAYS,
            .blendEnable = false,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            .stages = &stages,
            .vertexInputState = &kEmptyVertexInputState
        };
        return m_pipelineFactory->GetOrCreatePipeline(payload);
    }

    VkPipeline VulkanRenderer::GetOrCreatePipeline(
            GLenum mode,
            const MG_State::GLState::ProgramObject& program,
            const MG_State::GLState::VertexArrayObject& vao,
            const RenderPassEntry& renderPassEntry) {
        ProgramFactory::CompileOptionFlags transformFlags = GetShaderTransformFlags(m_swapchainObject.GetPreTransform());
        Bool invertClockwise = transformFlags & ProgramFactory::CompileOptionBit::PositionYFlip;
        auto& stages = m_programFactory->GetOrCreatePipelineShaderStages(program, transformFlags);
        if (stages.empty()) {
            MGLOG_D("GetOrCreatePipeline skipped: program has no shader stages");
            return VK_NULL_HANDLE;
        }
        const Uint64 programHash = m_programFactory->ComputeHash(program, transformFlags);

        auto vertexInputHash = m_vertexInputStateFactory->ComputeHash(vao);
        auto& vis = m_vertexInputStateFactory->GetOrCreateVertexInputState(vao);
        auto pipelineLayout = m_uniformDescriptorBinder->GetOrCreatePipelineLayout(program);
        auto cullFaceEnabled = MG_State::pGLContext->IsCapabilityEnabled(CapabilityInput::CullFace);
        auto depthTestEnabled = MG_State::pGLContext->IsCapabilityEnabled(CapabilityInput::DepthTest);
        BlendFactor srcRGB = BlendFactor::One;
        BlendFactor dstRGB = BlendFactor::Zero;
        BlendFactor srcAlpha = BlendFactor::One;
        BlendFactor dstAlpha = BlendFactor::Zero;
        MG_State::pGLContext->GetBlendFunc(srcRGB, dstRGB, srcAlpha, dstAlpha);
        auto mask = MG_State::pGLContext->GetColorMask();

        PipelineFactory::PipelineCreatePayload payload {
            .programHash = programHash,
            .vertexInputHash = vertexInputHash,
            .pipelineLayout = pipelineLayout,
            .renderPass = renderPassEntry.renderPass,
            .subpass = 0,
            .topology = MG_Util::ConvertPrimitiveModeToVkEnum(mode),
            .cullMode = cullFaceEnabled
                ? MG_Util::ConvertCullFaceModeToVkEnum(MG_State::pGLContext->GetCullFaceMode(), invertClockwise)
                : VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthTestEnable = depthTestEnabled,
            .depthWriteEnable = depthTestEnabled && MG_State::pGLContext->GetDepthMask(),
            .depthCompareOp = MG_Util::ConvertDepthTestFuncToVkEnum(MG_State::pGLContext->GetDepthFunc()),
            .blendEnable = MG_State::pGLContext->IsCapabilityEnabled(CapabilityInput::Blend),
            .srcColorBlendFactor = MG_Util::ConvertBlendFactorToVkEnum(srcRGB),
            .dstColorBlendFactor = MG_Util::ConvertBlendFactorToVkEnum(dstRGB),
            .srcAlphaBlendFactor = MG_Util::ConvertBlendFactorToVkEnum(srcAlpha),
            .dstAlphaBlendFactor = MG_Util::ConvertBlendFactorToVkEnum(dstAlpha),
            .colorWriteMask = (
                (mask.r() ? VK_COLOR_COMPONENT_R_BIT : 0u) |
                (mask.g() ? VK_COLOR_COMPONENT_G_BIT : 0u) |
                (mask.b() ? VK_COLOR_COMPONENT_B_BIT : 0u) |
                (mask.a() ? VK_COLOR_COMPONENT_A_BIT : 0u) ),
            .stages = &stages,
            .vertexInputState = &vis.state
        };
        return m_pipelineFactory->GetOrCreatePipeline(payload);
    }

    void VulkanRenderer::SetupDraw(FrameContext::FrameData& frame, GLenum mode, Flags<DrawSetupAspect> aspects,
                                   const IndexBufferView* pIndexBufferView) {
        m_textureManager->CollectGarbage();
        const auto& drawFbo =
                MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw).GetBoundObject();
        const auto& vao = *MG_State::pGLContext->GetBoundVertexArray();
        const auto& program = *MG_State::pGLContext->GetCurrentProgram();

        // Begin command recording if not yet
        if (!frame.isCommandRecording) {
            m_frameContext.BeginCommandRecording();
            m_uniformDescriptorBinder->BeginFrame(m_frameContext.GetCurrentFrameIndex());
        }

        auto* activeRenderPass = VkRenderPassManager::GetActiveRenderPass();

        // Check if any of the textures to sample have pending clears,
        // which probably indicates it's been gone through codepath like `fbo attach` -> `clear` -> `fbo detach`, and
        // without draws in between to give it a chance to materialize such clear.
        // Deal with this situation here.
        Vector<MG_State::GLState::ITextureObject*> sampledTextures;
        Bool hasSampledTextures = m_uniformDescriptorBinder->CollectSampledTextures(program, sampledTextures);
        MOBILEGL_ASSERT(hasSampledTextures, "%s: CollectSampledTextures failed", __func__);
        MGLOG_D("SetupDraw: program=%u drawFbo=%u sampledTextureCount=%zu activeRenderPass=%s",
                program.GetExternalIndex(), drawFbo ? drawFbo->GetExternalIndex() : 0u, sampledTextures.size(),
                activeRenderPass ? "true" : "false");
        Bool activeRenderPassUsesSampledTexture = false;
        if (activeRenderPass != nullptr) {
            for (auto* sampledTexture : sampledTextures) {
                if (sampledTexture == nullptr) {
                    continue;
                }
                if (ActiveRenderPassUsesTexture(*activeRenderPass, *sampledTexture)) {
                    MGLOG_D("SetupDraw: active render pass is still using sampled textureId=%d; ending render pass before descriptor preparation",
                            sampledTexture->GetExternalIndex());
                    activeRenderPassUsesSampledTexture = true;
                    break;
                }
            }
        }
        if (activeRenderPassUsesSampledTexture) {
            VkRenderPassManager::EndRenderPass(frame.commandBuffer);
            activeRenderPass = nullptr;
        }
        Bool needSampledTextureTransitions = false;
        for (auto* sampledTexture : sampledTextures) {
            if (!sampledTexture) {
                continue;
            }

            auto* textureResource = m_textureManager->SyncTextureAndGetDescriptor(*sampledTexture);
            MOBILEGL_ASSERT(textureResource != nullptr,
                            "%s: SyncTextureAndGetDescriptor failed for textureId=%d",
                            __func__, sampledTexture->GetExternalIndex());
            MGLOG_D("SetupDraw: sampled textureId=%d layout(before)=%s(%d)",
                    sampledTexture->GetExternalIndex(), VkImageLayoutToString(textureResource->layout),
                    static_cast<Int>(textureResource->layout));
            if (m_clearManager->HasPendingClear(sampledTexture) ||
                !IsValidSampledImageLayout(textureResource->layout)) {
                needSampledTextureTransitions = true;
                break;
            }
        }

        if (activeRenderPass && needSampledTextureTransitions) {
            MGLOG_D("SetupDraw: ending active render pass before sampled texture transitions");
            VkRenderPassManager::EndRenderPass(frame.commandBuffer);
            activeRenderPass = nullptr;
        }

        for (auto* sampledTexture : sampledTextures) {
            if (!sampledTexture) {
                continue;
            }
            const Bool clearReady = MaterializePendingClearForTexture(frame.commandBuffer, *sampledTexture);
            MOBILEGL_ASSERT(clearReady, "%s: MaterializePendingClearForTexture failed for textureId=%d",
                            __func__, sampledTexture->GetExternalIndex());
            const Bool ready = m_textureManager->TransitionTextureForSampling(frame.commandBuffer, *sampledTexture);
            MOBILEGL_ASSERT(ready, "%s: TransitionTextureForSampling failed for textureId=%d",
                            __func__, sampledTexture->GetExternalIndex());
            auto* transitionedResource = m_textureManager->SyncTextureAndGetDescriptor(*sampledTexture);
            MOBILEGL_ASSERT(transitionedResource != nullptr,
                            "%s: post-transition SyncTextureAndGetDescriptor failed for textureId=%d",
                            __func__, sampledTexture->GetExternalIndex());
            MGLOG_D("SetupDraw: sampled textureId=%d layout(after)=%s(%d)",
                    sampledTexture->GetExternalIndex(), VkImageLayoutToString(transitionedResource->layout),
                    static_cast<Int>(transitionedResource->layout));
        }

        auto& renderPassEntry = m_renderPassManager->GetOrCreateRenderPass(*drawFbo, m_imageIndexAcquired);
        auto pipeline = GetOrCreatePipeline(mode, program, vao, renderPassEntry);
        activeRenderPass = VkRenderPassManager::GetActiveRenderPass();

        // Begin render pass, and handle clear
        if (activeRenderPass && activeRenderPass->CompatibleWith(renderPassEntry)) {
            ClearAttachmentsOnActiveRenderPass(frame.commandBuffer, renderPassEntry);
        } else {
            // No active render pass or active one not compatible.
            // Restart a new render pass
            if (activeRenderPass)
                VkRenderPassManager::EndRenderPass(frame.commandBuffer);
            Bool ok = VkRenderPassManager::BeginRenderPass(frame.commandBuffer, renderPassEntry);
            MOBILEGL_ASSERT(ok, "%s: BeginRenderPass failed", __func__);
        }

        vkCmdBindPipeline(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        m_uniformDescriptorBinder->BindProgramUniformBuffers(frame.commandBuffer, program,
                                                                  m_frameContext.GetCurrentFrameIndex());

        auto vtxUploadOk = UploadAndBindVertexStreams(frame.commandBuffer, vao);
        MOBILEGL_ASSERT(vtxUploadOk, "SetupDraw skipped: failed to upload vertex streams");

        if (aspects & DrawSetupAspect::IndexBuffer) {
            auto idxUploadOk = UploadAndBindIndexBuffer(frame, vao, pIndexBufferView);
            MOBILEGL_ASSERT(idxUploadOk, "SetupDraw skipped: failed to upload index buffer");
        }

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(renderPassEntry.extent.x());
        viewport.height = static_cast<float>(renderPassEntry.extent.y());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(frame.commandBuffer, 0, 1, &viewport);

        Bool scissorEnabled = MG_State::pGLContext->IsCapabilityEnabled(CapabilityInput::ScissorTest);
        VkRect2D scissor{};
        if (scissorEnabled) {
            const auto& scissorBox = MG_State::pGLContext->GetScissorBox();
            scissor.offset = { scissorBox[0], scissorBox[1] };
            scissor.extent = { (Uint)scissorBox[2], (Uint)scissorBox[3] };
        } else {
            scissor.offset = {0, 0};
            scissor.extent = { (Uint)renderPassEntry.extent.x(), (Uint)renderPassEntry.extent.y() };
        }
        vkCmdSetScissor(frame.commandBuffer, 0, 1, &scissor);
    }

    void VulkanRenderer::Clear(GLbitfield mask) {
        m_clearManager->CollectGarbage();
        auto* fbo = MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw).GetBoundObject().get();
        MOBILEGL_ASSERT(fbo, "VulkanRenderer::Clear: draw framebuffer not found (fbo == nullptr)");

        ClearFramebufferPayload payload {
            .color = MG_State::pGLContext->GetClearColor(),
            .depth = MG_State::pGLContext->GetClearDepth(),
            .stencil = MG_State::pGLContext->GetClearStencil()
        };
        m_clearManager->QueueClear(mask, payload, *fbo);
    }

    Bool VulkanRenderer::MaterializePendingClearForTexture(VkCommandBuffer commandBuffer,
                                                           MG_State::GLState::ITextureObject& texture) {
        ClearAttachmentPayload clearPayload{};
        if (!m_clearManager->GetPendingClear(&texture, clearPayload)) {
            return true;
        }
        MOBILEGL_ASSERT(VkRenderPassManager::GetActiveRenderPass() == nullptr,
                        "MaterializePendingClearForTexture requires no active render pass");

        auto* resource = m_textureManager->SyncTextureAndGetDescriptor(texture);
        MOBILEGL_ASSERT(resource != nullptr,
                        "MaterializePendingClearForTexture: SyncTextureAndGetDescriptor failed for textureId=%d",
                        texture.GetExternalIndex());

        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkAccessFlags srcAccessMask = 0;
        GetImageTransitionSourceState(resource->layout, srcStageMask, srcAccessMask);

        Bool ok = VkTextureManager::TransitionImageLayout(
            commandBuffer, resource->image, resource->layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            srcStageMask, VK_PIPELINE_STAGE_TRANSFER_BIT, srcAccessMask, VK_ACCESS_TRANSFER_WRITE_BIT,
            resource->aspect, 0, resource->mipLevels);
        MOBILEGL_ASSERT(ok,
                        "MaterializePendingClearForTexture: failed to transition textureId=%d to TRANSFER_DST",
                        texture.GetExternalIndex());

        VkImageSubresourceRange subresourceRange{};
        subresourceRange.aspectMask = resource->aspect;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;

        VkImageLayout sampledLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        if ((resource->aspect & VK_IMAGE_ASPECT_COLOR_BIT) != 0) {
            VkClearColorValue clearValue{};
            clearValue.float32[0] = clearPayload.color.x();
            clearValue.float32[1] = clearPayload.color.y();
            clearValue.float32[2] = clearPayload.color.z();
            clearValue.float32[3] = clearPayload.color.w();
            vkCmdClearColorImage(commandBuffer, resource->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 &clearValue, 1, &subresourceRange);
        } else {
            VkClearDepthStencilValue clearValue{};
            clearValue.depth = clearPayload.depth;
            clearValue.stencil = clearPayload.stencil;
            vkCmdClearDepthStencilImage(commandBuffer, resource->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        &clearValue, 1, &subresourceRange);
            sampledLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        }

        ok = VkTextureManager::TransitionImageLayout(
            commandBuffer, resource->image, resource->layout, sampledLayout,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, resource->aspect, 0, resource->mipLevels);
        MOBILEGL_ASSERT(ok,
                        "MaterializePendingClearForTexture: failed to transition textureId=%d to sampled layout",
                        texture.GetExternalIndex());

        m_clearManager->PopPendingClear(&texture);
        MGLOG_D("MaterializePendingClearForTexture: textureId=%d pending clear materialized",
                texture.GetExternalIndex());
        return true;
    }

    Bool VulkanRenderer::TryBlitToDefaultFramebufferWithShader(FrameContext::FrameData& frame,
                                                               MG_State::GLState::FramebufferObject& readFbo,
                                                               MG_State::GLState::FramebufferObject& drawFbo,
                                                               GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                                                               GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                                                               GLenum filter) {
        const Bool drawIsDefaultFbo =
            (&drawFbo == MG_Impl::GLImpl::FramebufferImpl::pDefaultFramebufferInfo->defaultFBO.get());
        if (!drawIsDefaultFbo) {
            return false;
        }

        BlitImageBinding srcBinding{};
        BlitImageBinding dstBinding{};
        if (!ResolveColorBlitBinding(readFbo, true, m_imageIndexAcquired, m_swapchainObject, *m_textureManager, srcBinding) ||
            !ResolveColorBlitBinding(drawFbo, false, m_imageIndexAcquired, m_swapchainObject, *m_textureManager, dstBinding)) {
            return false;
        }
        if (srcBinding.trackedLayout == nullptr) {
            MGLOG_E("BlitFramebuffer skipped: shader blit to default framebuffer requires a texture-backed source framebuffer");
            return false;
        }

        auto* activeRenderPass = VkRenderPassManager::GetActiveRenderPass();
        if (activeRenderPass != nullptr) {
            VkRenderPassManager::EndRenderPass(frame.commandBuffer);
        }

        const auto& attachment = readFbo.GetAttachment(readFbo.GetReadBuffer());
        auto sourceTexture = attachment.GetTexture();
        MOBILEGL_ASSERT(sourceTexture != nullptr, "TryBlitToDefaultFramebufferWithShader: source texture is null");
        const Bool clearReady = MaterializePendingClearForTexture(frame.commandBuffer, *sourceTexture);
        MOBILEGL_ASSERT(clearReady,
                        "TryBlitToDefaultFramebufferWithShader: failed to materialize pending clear for textureId=%d",
                        sourceTexture->GetExternalIndex());
        const Bool ready = m_textureManager->TransitionTextureForSampling(frame.commandBuffer, *sourceTexture);
        if (!ready) {
            MGLOG_E("BlitFramebuffer skipped: failed to transition source textureId=%d for sampling",
                    sourceTexture->GetExternalIndex());
            return false;
        }
        if (m_textureManager->SyncTextureAndGetDescriptor(*sourceTexture) == nullptr) {
            MGLOG_E("BlitFramebuffer skipped: failed to resolve source textureId=%d after sampling transition",
                    sourceTexture->GetExternalIndex());
            return false;
        }

        auto& renderPassEntry = m_renderPassManager->GetOrCreateRenderPass(drawFbo, m_imageIndexAcquired);
        const Bool ok = VkRenderPassManager::BeginRenderPass(frame.commandBuffer, renderPassEntry);
        MOBILEGL_ASSERT(ok, "%s: BeginRenderPass failed", __func__);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(renderPassEntry.extent.x());
        viewport.height = static_cast<float>(renderPassEntry.extent.y());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(frame.commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {static_cast<Uint32>(renderPassEntry.extent.x()), static_cast<Uint32>(renderPassEntry.extent.y())};
        vkCmdSetScissor(frame.commandBuffer, 0, 1, &scissor);

        MOBILEGL_ASSERT(m_blitResources.program != nullptr, "TryBlitToDefaultFramebufferWithShader: blit program is null");
        const VkPipeline pipeline = GetOrCreateBlitPipeline(renderPassEntry);
        MOBILEGL_ASSERT(pipeline != VK_NULL_HANDLE, "TryBlitToDefaultFramebufferWithShader: blit pipeline is null");
        vkCmdBindPipeline(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        auto* blitProgramData = static_cast<Uint8*>(m_blitResources.program->MapUBO());
        MOBILEGL_ASSERT(blitProgramData != nullptr, "TryBlitToDefaultFramebufferWithShader: blit UBO is null");
        std::fill(blitProgramData, blitProgramData + m_blitResources.program->GetUBOSize(), Uint8{0});

        BlitUniformData blitUniformData{};
        const float srcWidth = static_cast<float>(srcBinding.extent.x());
        const float srcHeight = static_cast<float>(srcBinding.extent.y());
        const float dstWidth = static_cast<float>(dstBinding.extent.x());
        const float dstHeight = static_cast<float>(dstBinding.extent.y());
        float dstNormWidth = dstWidth;
        float dstNormHeight = dstHeight;
        switch (m_swapchainObject.GetPreTransform()) {
            case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:
            case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:
                dstNormWidth = dstHeight;
                dstNormHeight = dstWidth;
                break;
            default:
                break;
        }
        blitUniformData.srcRect[0] = static_cast<float>(srcX0) / srcWidth;
        blitUniformData.srcRect[1] = static_cast<float>(srcY0) / srcHeight;
        blitUniformData.srcRect[2] = static_cast<float>(srcX1 - srcX0) / srcWidth;
        blitUniformData.srcRect[3] = static_cast<float>(srcY1 - srcY0) / srcHeight;
        blitUniformData.dstRect[0] = static_cast<float>(dstX0) / dstNormWidth;
        blitUniformData.dstRect[1] = static_cast<float>(dstY0) / dstNormHeight;
        blitUniformData.dstRect[2] = static_cast<float>(dstX1 - dstX0) / dstNormWidth;
        blitUniformData.dstRect[3] = static_cast<float>(dstY1 - dstY0) / dstNormHeight;
        blitUniformData.surfaceTransform = static_cast<Int>(ToBlitSurfaceTransform(m_swapchainObject.GetPreTransform()));

        auto writeUniform = [&](Int location, const void* data, SizeT size) {
            MOBILEGL_ASSERT(location >= 0, "TryBlitToDefaultFramebufferWithShader: invalid uniform location");
            const Uint offset = m_blitResources.program->GetUniformOffset(static_cast<Uint>(location));
            MOBILEGL_ASSERT(offset + size <= m_blitResources.program->GetUBOSize(),
                            "TryBlitToDefaultFramebufferWithShader: uniform write out of bounds");
            memcpy(blitProgramData + offset, data, size);
        };
        writeUniform(m_blitResources.srcRectLocation, blitUniformData.srcRect, sizeof(blitUniformData.srcRect));
        writeUniform(m_blitResources.dstRectLocation, blitUniformData.dstRect, sizeof(blitUniformData.dstRect));
        writeUniform(m_blitResources.surfaceTransformLocation, &blitUniformData.surfaceTransform,
                     sizeof(blitUniformData.surfaceTransform));

        const auto samplerBindingOverride = UniformDescriptorBinder::SamplerBindingOverride{
            .binding = m_blitResources.samplerBinding,
            .texture = sourceTexture.get(),
            .sampler = (filter == GL_LINEAR ? m_blitResources.linearSampler.get()
                                            : m_blitResources.nearestSampler.get()),
        };
        const Bool bound = m_uniformDescriptorBinder->BindProgramUniformBuffers(
            frame.commandBuffer, *m_blitResources.program, m_frameContext.GetCurrentFrameIndex(),
            &samplerBindingOverride);
        MOBILEGL_ASSERT(bound, "TryBlitToDefaultFramebufferWithShader: BindProgramUniformBuffers failed");
        vkCmdDraw(frame.commandBuffer, 3, 1, 0, 0);
        return true;
    }

    void VulkanRenderer::BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                                         GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                                         GLbitfield mask, GLenum filter) {
        if ((mask & ~GL_COLOR_BUFFER_BIT) != 0) {
            MGLOG_E("BlitFramebuffer skipped: only GL_COLOR_BUFFER_BIT is supported right now (mask=0x%x)",
                    static_cast<Uint32>(mask));
            return;
        }
        if ((mask & GL_COLOR_BUFFER_BIT) == 0) {
            return;
        }
        if (filter != GL_NEAREST && filter != GL_LINEAR) {
            MGLOG_E("BlitFramebuffer skipped: unsupported filter=0x%x", static_cast<Uint32>(filter));
            return;
        }

        auto readFbo = MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Read).GetBoundObject();
        auto drawFbo = MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw).GetBoundObject();
        MOBILEGL_ASSERT(readFbo != nullptr, "VulkanRenderer::BlitFramebuffer: read framebuffer is null");
        MOBILEGL_ASSERT(drawFbo != nullptr, "VulkanRenderer::BlitFramebuffer: draw framebuffer is null");

        auto& frame = m_frameContext.GetCurrent();
        if (!frame.isCommandRecording) {
            m_frameContext.BeginCommandRecording();
            m_uniformDescriptorBinder->BeginFrame(m_frameContext.GetCurrentFrameIndex());
        }

        auto* activeRenderPass = VkRenderPassManager::GetActiveRenderPass();
        if (activeRenderPass != nullptr) {
            VkRenderPassManager::EndRenderPass(frame.commandBuffer);
        }

        const Bool readIsDefaultFbo =
            (readFbo == MG_Impl::GLImpl::FramebufferImpl::pDefaultFramebufferInfo->defaultFBO);
        const Bool drawIsDefaultFbo =
            (drawFbo == MG_Impl::GLImpl::FramebufferImpl::pDefaultFramebufferInfo->defaultFBO);
        if (drawIsDefaultFbo && m_swapchainObject.GetPreTransform() != VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
            if (TryBlitToDefaultFramebufferWithShader(frame, *readFbo, *drawFbo,
                                                      srcX0, srcY0, srcX1, srcY1,
                                                      dstX0, dstY0, dstX1, dstY1, filter)) {
                return;
            }
            MGLOG_E("BlitFramebuffer skipped: rotated blit to default framebuffer requires a texture-backed source framebuffer");
            return;
        }

        BlitImageBinding srcBinding{};
        BlitImageBinding dstBinding{};
        if (!ResolveColorBlitBinding(*readFbo, true, m_imageIndexAcquired, m_swapchainObject, *m_textureManager, srcBinding) ||
            !ResolveColorBlitBinding(*drawFbo, false, m_imageIndexAcquired, m_swapchainObject, *m_textureManager, dstBinding)) {
            return;
        }

        if (!readIsDefaultFbo) {
            const auto& sourceAttachment = readFbo->GetAttachment(readFbo->GetReadBuffer());
            auto sourceTexture = sourceAttachment.GetTexture();
            MOBILEGL_ASSERT(sourceTexture != nullptr, "BlitFramebuffer: source texture attachment is null");
            const Bool clearReady = MaterializePendingClearForTexture(frame.commandBuffer, *sourceTexture);
            MOBILEGL_ASSERT(clearReady,
                            "BlitFramebuffer: failed to materialize pending clear for source textureId=%d",
                            sourceTexture->GetExternalIndex());
        }

        VkImageLayout srcLayout = readIsDefaultFbo
            ? m_swapchainObject.GetImageLayout(m_imageIndexAcquired)
            : *srcBinding.trackedLayout;
        VkImageLayout dstLayout = drawIsDefaultFbo
            ? m_swapchainObject.GetImageLayout(m_imageIndexAcquired)
            : *dstBinding.trackedLayout;

        if (readIsDefaultFbo && srcLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            MGLOG_E("BlitFramebuffer skipped: swapchain source image layout is undefined");
            return;
        }
        if (srcLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            MGLOG_E("BlitFramebuffer skipped: source image layout is undefined");
            return;
        }

        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkAccessFlags srcAccessMask = 0;
        GetImageTransitionSourceState(srcLayout, srcStageMask, srcAccessMask);
        if (readIsDefaultFbo) {
            Bool ok = VkTextureManager::TransitionImageLayout(
                frame.commandBuffer, srcBinding.image, srcLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                srcStageMask, VK_PIPELINE_STAGE_TRANSFER_BIT,
                srcAccessMask, VK_ACCESS_TRANSFER_READ_BIT, srcBinding.aspectMask);
            MOBILEGL_ASSERT(ok, "%s: failed to transition swapchain source image", __func__);
            m_swapchainObject.SetImageLayout(m_imageIndexAcquired, srcLayout);
        } else {
            Bool ok = VkTextureManager::TransitionImageLayout(
                frame.commandBuffer, srcBinding.image, *srcBinding.trackedLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                srcStageMask, VK_PIPELINE_STAGE_TRANSFER_BIT,
                srcAccessMask, VK_ACCESS_TRANSFER_READ_BIT, srcBinding.aspectMask, 0, srcBinding.mipLevelCount);
            MOBILEGL_ASSERT(ok, "%s: failed to transition source image", __func__);
        }

        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkAccessFlags dstAccessMask = 0;
        GetImageTransitionSourceState(dstLayout, dstStageMask, dstAccessMask);
        if (drawIsDefaultFbo) {
            Bool ok = VkTextureManager::TransitionImageLayout(
                frame.commandBuffer, dstBinding.image, dstLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                dstStageMask, VK_PIPELINE_STAGE_TRANSFER_BIT,
                dstAccessMask, VK_ACCESS_TRANSFER_WRITE_BIT, dstBinding.aspectMask);
            MOBILEGL_ASSERT(ok, "%s: failed to transition swapchain destination image", __func__);
            m_swapchainObject.SetImageLayout(m_imageIndexAcquired, dstLayout);
        } else {
            Bool ok = VkTextureManager::TransitionImageLayout(
                frame.commandBuffer, dstBinding.image, *dstBinding.trackedLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                dstStageMask, VK_PIPELINE_STAGE_TRANSFER_BIT,
                dstAccessMask, VK_ACCESS_TRANSFER_WRITE_BIT, dstBinding.aspectMask, 0, dstBinding.mipLevelCount);
            MOBILEGL_ASSERT(ok, "%s: failed to transition destination image", __func__);
        }

        VkImageBlit blitRegion{};
        blitRegion.srcSubresource.aspectMask = srcBinding.aspectMask;
        blitRegion.srcSubresource.mipLevel = srcBinding.mipLevel;
        blitRegion.srcSubresource.baseArrayLayer = 0;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.srcOffsets[0] = {srcX0, srcY0, 0};
        blitRegion.srcOffsets[1] = {srcX1, srcY1, 1};
        blitRegion.dstSubresource.aspectMask = dstBinding.aspectMask;
        blitRegion.dstSubresource.mipLevel = dstBinding.mipLevel;
        blitRegion.dstSubresource.baseArrayLayer = 0;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.dstOffsets[0] = {dstX0, dstY0, 0};
        blitRegion.dstOffsets[1] = {dstX1, dstY1, 1};

        vkCmdBlitImage(frame.commandBuffer,
                       srcBinding.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       dstBinding.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blitRegion, filter == GL_LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);
    }

    void VulkanRenderer::DrawArrays(const DrawCmd& payload) {
        auto& frame = m_frameContext.GetCurrent();

        SetupDraw(frame, payload.mode, 0);

        MOBILEGL_ASSERT(frame.isCommandRecording, "%s: frame recording was not started", __func__);

        VkCommandBuffer& commandBuffer = frame.commandBuffer;

        vkCmdDraw(commandBuffer,
            payload.params.vertexCount,
            payload.params.instanceCount,
            payload.params.firstVertex,
            payload.params.firstInstance);
    }

    void VulkanRenderer::DrawElements(const DrawIndexedCmd& payload) {
        auto& frame = m_frameContext.GetCurrent();

        SetupDraw(frame, payload.mode, DrawSetupAspect::IndexBuffer,
                       &payload.indexBufferView);

        MOBILEGL_ASSERT(frame.isCommandRecording, "%s: frame recording was not started", __func__);

        VkCommandBuffer& commandBuffer = frame.commandBuffer;

        vkCmdDrawIndexed(commandBuffer,
            payload.params.indexCount,
            payload.params.instanceCount,
            payload.params.firstIndex,
            payload.params.vertexOffset,
            payload.params.firstInstance);
    }

    void VulkanRenderer::MultiDrawElements(const MultiDrawIndexedCmd& payload) {
        auto& frame = m_frameContext.GetCurrent();

        SetupDraw(frame, payload.mode, DrawSetupAspect::IndexBuffer,
                  &payload.indexBufferView);

        MOBILEGL_ASSERT(frame.isCommandRecording, "%s: frame recording was not started", __func__);

        VkCommandBuffer& commandBuffer = frame.commandBuffer;

        for (Uint32 idraw = 0; idraw < payload.drawCount; ++idraw) {
            vkCmdDrawIndexed(commandBuffer,
                             payload.pParams[idraw].indexCount,
                             payload.pParams[idraw].instanceCount,
                             payload.pParams[idraw].firstIndex,
                             payload.pParams[idraw].vertexOffset,
                             payload.pParams[idraw].firstInstance);
        }
    }

    void VulkanRenderer::Present() {
        MOBILEGL_ASSERT(m_imageIndexAcquired < m_swapchainObject.GetImageCount(),
                        "Present, acquired image index out of range");
        auto& frame = m_frameContext.GetCurrent();
        auto* activeRenderPass = VkRenderPassManager::GetActiveRenderPass();
        if (activeRenderPass)
            VkRenderPassManager::EndRenderPass(frame.commandBuffer);
        if (frame.isCommandRecording) {
            m_frameContext.EndCommandRecording();
            frame.hasCommandBufferRecorded = true;
        }

        const auto acquiredImageLayout = m_swapchainObject.GetImageLayout(m_imageIndexAcquired);
        const Bool needsLayoutTransitionForPresent =
            m_frameContext.TransitionToPresent(m_swapchainObject.GetImage(m_imageIndexAcquired), acquiredImageLayout);
        const Bool shouldSubmitCommandBuffer = frame.hasCommandBufferRecorded || needsLayoutTransitionForPresent;

        // 1) Submit current frame work.
        auto submitPacket = m_frameContext.GetSubmitInfo(shouldSubmitCommandBuffer, m_imageIndexAcquired);
        VK_VERIFY(vkQueueSubmit(m_graphicsQueue, 1, &submitPacket.submitInfo, frame.imageInFlightFence));
        frame.isCommandRecording = false;
        frame.hasCommandBufferRecorded = false;
        m_swapchainObject.SetImageLayout(m_imageIndexAcquired, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        // 2) Present current frame.
        auto presentPacket = m_frameContext.GetPresentInfo(m_swapchainObject.GetHandle(), m_imageIndexAcquired);
        auto result = vkQueuePresentKHR(m_presentQueue, &presentPacket.presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            MGLOG_D("Present, vkQueuePresentKHR got %d, recreating swapchain", result);
            RecreateSwapchain();
            result = VK_SUCCESS;
        }
        VK_VERIFY(result, "Present, vkQueuePresentKHR");

        // 3) Advance frame slot.
        m_frameContext.AdvanceToNext();

        // 4) Wait/reset/acquire for next frame.
        result = m_frameContext.WaitAndAcquireNextImage(m_device, m_swapchainObject.GetHandle(), m_imageIndexAcquired);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            MGLOG_D("Present, vkAcquireNextImageKHR got %d, recreating swapchain", result);
            RecreateSwapchain();
            result = VK_SUCCESS;
        }
        VK_VERIFY(result, "Present, vkAcquireNextImageKHR");
        m_bufferManager.BeginFrame(m_frameContext.GetCurrentFrameIndex());
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
            MGLOG_I("Validation layers not available! Disabling validation layers.");
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
#elif defined VK_USE_PLATFORM_METAL_EXT
                                    VK_EXT_METAL_SURFACE_EXTENSION_NAME
#else
#warning "VulkanContext::CreateInstance: VK_KHR_*_surface extension not defined on this platform"
#endif
        }; // TODO: support more platforms

#if defined(VK_USE_PLATFORM_METAL_EXT)
        exts.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

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

        if (m_validationLayersEnabled) VK_VERIFY(SetupDebugMessenger());
    }

    VkResult VulkanRenderer::SetupDebugMessenger() {
        auto createInfo = PopulateDebugMessengerCreateInfo();
        auto vkCreateDebugUtilsMessengerEXT =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
        if (!vkCreateDebugUtilsMessengerEXT) return VK_ERROR_EXTENSION_NOT_PRESENT;
        VK_VERIFY(vkCreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger));
        return VK_SUCCESS;
    }

    VkResult VulkanRenderer::DestroyDebugMessenger() {
        if (m_debugMessenger != VK_NULL_HANDLE) {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance,
                                                                                   "vkDestroyDebugUtilsMessengerEXT");
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
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
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

    Bool VulkanRenderer::GetMoreCapablePhysicalDevice(VkPhysicalDevice newVkDevice, VkSurfaceKHR surface,
                                                      const PhysicalDevice& otherDevice,
                                                      PhysicalDevice& outBetterDevice) {
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
        MGLOG_I("    %s (Vulkan %d.%d.%d, %s)", deviceProperties.deviceName, VK_VERSION_MAJOR(apiVersion),
                VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion),
                deviceTypeToStr(deviceProperties.deviceType));

        // Check device extensions (including swapchain extension)
        Bool deviceExtSupported = IsNecessaryDeviceExtensionSupported(newVkDevice);
        if (!deviceExtSupported) {
            outBetterDevice = otherDevice;
            MGLOG_I("    Ignored physical device. (Reason: Some of the required device extension not supported on this "
                    "device)");
            return false;
        }

        // Check swapchain capabilities
        auto swapchainCapabilities = SwapchainObject::GetSwapchainCapabilities(newVkDevice, surface);
        if (!swapchainCapabilities.IsComplete()) {
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

    Bool VulkanRenderer::IsNecessaryDeviceExtensionSupported(VkPhysicalDevice device) {
        const Vector<VkExtensionProperties> availableExtensions = EnumerateDeviceExtensions(device);

        MGLOG_I("Got %u Vulkan device extensions: ", static_cast<Uint32>(availableExtensions.size()));
        for (auto& extension : availableExtensions) {
            MGLOG_I("    %s (r.%u)", extension.extensionName, extension.specVersion);
        }

        for (SizeT i = 0; i < std::size(s_deviceExtensionNames); ++i) {
            if (!IsExtensionSupported(availableExtensions, s_deviceExtensionNames[i])) {
                MGLOG_I("Required extension not found: %s", s_deviceExtensionNames[i]);
                return false;
            }
            MGLOG_I("Required extension found: %s", s_deviceExtensionNames[i]);
        }

        return true;
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

        Vector<const char*> enabledDeviceExtensions;
        enabledDeviceExtensions.reserve(std::size(s_deviceExtensionNames) + 2);
        for (const char* extensionName : s_deviceExtensionNames) {
            enabledDeviceExtensions.push_back(extensionName);
        }

        const Vector<VkExtensionProperties> availableExtensions = EnumerateDeviceExtensions(m_physicalDevice.handle);
        ResolveOptionalDeviceExtensions(availableExtensions, enabledDeviceExtensions);
        MGLOG_I("VK_KHR_draw_indirect_count enabled: %s", m_drawIndirectCountExtensionEnabled ? "true" : "false");

        deviceCreateInfo.enabledExtensionCount = static_cast<Uint32>(enabledDeviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();
        VK_VERIFY(vkCreateDevice(m_physicalDevice.handle, &deviceCreateInfo, nullptr, &m_device), "vkCreateDevice");

        s_vkCmdDrawIndexedIndirectCount = reinterpret_cast<PFNDrawIndexedIndirectCountFunc>(
            vkGetDeviceProcAddr(m_device, "vkCmdDrawIndexedIndirectCountKHR"));
        if (s_vkCmdDrawIndexedIndirectCount == nullptr) {
            s_vkCmdDrawIndexedIndirectCount = reinterpret_cast<PFNDrawIndexedIndirectCountFunc>(
                vkGetDeviceProcAddr(m_device, "vkCmdDrawIndexedIndirectCount"));
        }
        if (m_drawIndirectCountExtensionEnabled && s_vkCmdDrawIndexedIndirectCount == nullptr) {
            MGLOG_W("VK_KHR_draw_indirect_count enabled but vkCmdDrawIndexedIndirectCount entry point is missing, will continue as if VK_KHR_draw_indirect_count is not supported!");
            m_drawIndirectCountExtensionEnabled = false;
        }
        MGLOG_I("Logical device created.");

        // Queues
        vkGetDeviceQueue(m_device, m_physicalDevice.queueFamilies.graphicsFamily, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, m_physicalDevice.queueFamilies.presentFamily, 0, &m_presentQueue);
        MGLOG_I("Queues got successfully.");
    }

    void VulkanRenderer::CreateAllocator() {
        MOBILEGL_ASSERT(m_instance != VK_NULL_HANDLE, "CreateAllocator requires valid VkInstance");
        MOBILEGL_ASSERT(m_physicalDevice.handle != VK_NULL_HANDLE, "CreateAllocator requires valid physical device");
        MOBILEGL_ASSERT(m_device != VK_NULL_HANDLE, "CreateAllocator requires valid VkDevice");

        if (m_allocator != nullptr) {
            return;
        }

        VmaAllocatorCreateInfo allocatorInfo{};
        VmaVulkanFunctions vulkanFunctions{};
        vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        allocatorInfo.instance = m_instance;
        allocatorInfo.physicalDevice = m_physicalDevice.handle;
        allocatorInfo.device = m_device;
        allocatorInfo.pVulkanFunctions = &vulkanFunctions;
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;

        VK_VERIFY(vmaCreateAllocator(&allocatorInfo, &m_allocator), "vmaCreateAllocator");
    }

    void VulkanRenderer::DestroyAllocator() {
        if (m_allocator != nullptr) {
            vmaDestroyAllocator(m_allocator);
            m_allocator = nullptr;
        }
    }

    void VulkanRenderer::CreateSwapchain() {
        m_swapchainObject.Create(m_device, m_physicalDevice.handle, m_surface,
                                 static_cast<Uint32>(m_physicalDevice.queueFamilies.graphicsFamily),
                                 static_cast<Uint32>(m_physicalDevice.queueFamilies.presentFamily),
                                 m_config.MaxFramesInFlight);
    }

    void VulkanRenderer::CreateCommandPool() {
        VkCommandPoolCreateInfo createInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = m_physicalDevice.queueFamilies.graphicsFamily;
        VK_VERIFY(vkCreateCommandPool(m_device, &createInfo, nullptr, &m_commandPool));
        MGLOG_I("Command pool created");
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
#elif defined VK_USE_PLATFORM_METAL_EXT
        MOBILEGL_ASSERT(m_window, "CAMetalLayer is null");

        VkMetalSurfaceCreateInfoEXT sci{VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT};
        sci.pLayer = reinterpret_cast<const void*>(m_window);
        VK_VERIFY(vkCreateMetalSurfaceEXT(m_instance, &sci, nullptr, &m_surface), "vkCreateMetalSurfaceEXT failed");
#else
        // #warning "VulkanRenderer::Initialize called on a platform which is not supported yet"
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

    Int VulkanRenderer::GetPresentQueueFamilyIndex(const PhysicalDevice& physicalDevice, VkSurfaceKHR surface,
                                                   const Vector<VkQueueFamilyProperties>& queueFamilies,
                                                   Int preferredFamilyIndex) {
        if (preferredFamilyIndex != -1) {
            VkBool32 supportsPresent = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.handle, preferredFamilyIndex, surface,
                                                 &supportsPresent);
            if (supportsPresent) return preferredFamilyIndex;
        }

        for (Uint32 i = 0; i < queueFamilies.size(); i++) {
            VkBool32 supportsPresent = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.handle, i, surface, &supportsPresent);
            if (supportsPresent) return i;
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

    Vector<VkExtensionProperties> VulkanRenderer::EnumerateDeviceExtensions(VkPhysicalDevice device) {
        Uint32 extensionCount = 0;
        VK_VERIFY(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));
        Vector<VkExtensionProperties> extensions(extensionCount);
        VK_VERIFY(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data()));
        return extensions;
    }

    Bool VulkanRenderer::IsExtensionSupported(const Vector<VkExtensionProperties>& availableExtensions,
                                              const char* extensionName) {
        for (const auto& extension : availableExtensions) {
            if (strcmp(extension.extensionName, extensionName) == 0) {
                return true;
            }
        }
        return false;
    }

    Bool VulkanRenderer::IsExtensionAlreadyEnabled(const Vector<const char*>& enabledExtensions,
                                                   const char* extensionName) {
        return std::any_of(enabledExtensions.begin(), enabledExtensions.end(),
                           [&extensionName](const String& name) { return name == extensionName; });
    }

    Bool VulkanRenderer::EnableOptionalDeviceExtension(const Vector<VkExtensionProperties>& availableExtensions,
                                                       Vector<const char*>& inOutEnabledExtensions,
                                                       const char* extensionName) {
        if (!IsExtensionSupported(availableExtensions, extensionName)) {
            MGLOG_I("Optional device extension not supported: %s", extensionName);
            return false;
        }

        if (!IsExtensionAlreadyEnabled(inOutEnabledExtensions, extensionName)) {
            inOutEnabledExtensions.push_back(extensionName);
        }
        MGLOG_I("Enabled optional device extension: %s", extensionName);
        return true;
    }

    void VulkanRenderer::ResolveOptionalDeviceExtensions(const Vector<VkExtensionProperties>& availableExtensions,
                                                         Vector<const char*>& inOutEnabledExtensions) {
        m_drawIndirectCountExtensionEnabled = EnableOptionalDeviceExtension(availableExtensions, inOutEnabledExtensions,
                                                                            VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
        EnableOptionalDeviceExtension(availableExtensions, inOutEnabledExtensions,
                                      VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif
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

    void VulkanRenderer::ShutdownSwapchain() {
        MOBILEGL_ASSERT(m_renderPassManager != nullptr, "ShutdownSwapchain: render pass manager is null");
        m_renderPassManager->Shutdown();

        m_swapchainObject.Shutdown(m_device);
    }

    void VulkanRenderer::RecreateSwapchain() {
        // Handle cases like minimize on Windows, where swapchain could return a 0x0 extent
        const auto swapchainCapabilities =
            SwapchainObject::GetSwapchainCapabilities(m_physicalDevice.handle, m_surface);
        if (swapchainCapabilities.capabilities.currentExtent.width == 0 ||
            swapchainCapabilities.capabilities.currentExtent.height == 0) {
            return;
        }

        vkDeviceWaitIdle(m_device);

        ShutdownSwapchain();

        CreateSwapchain();
        VK_VERIFY(m_frameContext.InitializeSwapchainSemaphores(m_device,
                                                               static_cast<Uint32>(m_swapchainObject.GetImageCount())),
                  "RecreateSwapchain, InitializeSwapchainSemaphores");
        MOBILEGL_ASSERT(m_renderPassManager != nullptr, "RecreateSwapchain: render pass manager is null");
        Bool ok = m_renderPassManager->Initialize();
        MOBILEGL_ASSERT(ok, "RecreateSwapchain: render pass manager initialization failed");
        if (m_pipelineFactory) {
            m_pipelineFactory->DestroyAll();
        }
        if (m_frameContext.GetFrameCount() > 0) {
            m_frameContext.GetCurrent().isCommandRecording = false;
            m_frameContext.GetCurrent().hasCommandBufferRecorded = false;
        }
        const Bool okArena = m_bufferManager.RecreateTransientArenas(m_frameContext.GetFrameCount());
        MOBILEGL_ASSERT(okArena, "RecreateSwapchain: buffer manager transient arena initialization failed");
        if (m_frameContext.GetFrameCount() > 0) {
            m_bufferManager.BeginFrame(m_frameContext.GetCurrentFrameIndex());
        }
    }

    const PhysicalDevice& VulkanRenderer::GetPhysicalDevice() const {
        return m_physicalDevice;
    }

    Bool VulkanRenderer::IsDrawIndirectCountExtensionEnabled() const {
        return m_drawIndirectCountExtensionEnabled;
    }

    void VulkanRenderer::ClearAttachmentsOnActiveRenderPass(VkCommandBuffer commandBuffer,
                                                            const RenderPassEntry &compatibleRenderPassEntry) {
        auto* activeRenderPass = VkRenderPassManager::GetActiveRenderPass();
        MOBILEGL_ASSERT(activeRenderPass, "No render pass active");
        VkClearRect clearRect{};
        clearRect.rect.offset = {0, 0};
        clearRect.rect.extent = {
                static_cast<Uint32>(activeRenderPass->extent.x()),
                static_cast<Uint32>(activeRenderPass->extent.y())
        };
        clearRect.baseArrayLayer = 0;
        clearRect.layerCount = 1;

        for (const auto& pending : compatibleRenderPassEntry.pendingClearAttachments) {
            if (!pending.texture) {
                continue;
            }

            ClearAttachmentPayload clearPayload{};
            if (!m_clearManager->GetPendingClear(pending.texture, clearPayload)) {
                continue;
            }

            VkClearAttachment clearAttachment{};
            clearAttachment.clearValue.depthStencil = {1.0f, 0};
            if (clearPayload.attachmentType >= FramebufferAttachmentType::Color0 &&
                clearPayload.attachmentType <= FramebufferAttachmentType::Color31) {
                clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                clearAttachment.colorAttachment = pending.attachmentIndex;
                clearAttachment.clearValue.color = {
                        clearPayload.color.x(),
                        clearPayload.color.y(),
                        clearPayload.color.z(),
                        clearPayload.color.w()
                };
            } else if (clearPayload.attachmentType == FramebufferAttachmentType::Depth) {
                clearAttachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                clearAttachment.clearValue.depthStencil.depth = clearPayload.depth;
            } else if (clearPayload.attachmentType == FramebufferAttachmentType::Stencil) {
                clearAttachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                clearAttachment.clearValue.depthStencil.stencil = clearPayload.stencil;
            } else {
                continue;
            }

            vkCmdClearAttachments(commandBuffer, 1, &clearAttachment, 1, &clearRect);
            m_clearManager->PopPendingClear(pending.texture);
        }
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
