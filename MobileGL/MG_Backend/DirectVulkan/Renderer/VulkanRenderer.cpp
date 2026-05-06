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
#include "MG_Util/Converters/GLToMG/TextureEnumConverter.h"
#include "MG_Util/Converters/MGToVk/RenderStateEnumConverter.h"
#include <vulkan/vulkan_core.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    static VkPipelineColorBlendAttachmentState MakeColorBlendAttachmentState(
        Bool blendEnable,
        VkBlendFactor srcColorBlendFactor,
        VkBlendFactor dstColorBlendFactor,
        VkBlendFactor srcAlphaBlendFactor,
        VkBlendFactor dstAlphaBlendFactor,
        VkColorComponentFlags colorWriteMask) {
        VkPipelineColorBlendAttachmentState attachment{};
        attachment.blendEnable = blendEnable ? VK_TRUE : VK_FALSE;
        attachment.srcColorBlendFactor = srcColorBlendFactor;
        attachment.dstColorBlendFactor = dstColorBlendFactor;
        attachment.colorBlendOp = VK_BLEND_OP_ADD;
        attachment.srcAlphaBlendFactor = srcAlphaBlendFactor;
        attachment.dstAlphaBlendFactor = dstAlphaBlendFactor;
        attachment.alphaBlendOp = VK_BLEND_OP_ADD;
        attachment.colorWriteMask = colorWriteMask;
        return attachment;
    }

    static Bool ShouldUseTransientVertexIndexBuffer(const MG_State::GLState::BufferObject& bufferObject) {
        switch (bufferObject.GetUsage()) {
        case BufferUsage::StreamDraw:
        case BufferUsage::StreamRead:
        case BufferUsage::StreamCopy:
        case BufferUsage::DynamicDraw:
        case BufferUsage::DynamicRead:
        case BufferUsage::DynamicCopy:
            return true;
        case BufferUsage::StaticDraw:
        case BufferUsage::StaticRead:
        case BufferUsage::StaticCopy:
        default:
            return false;
        }
    }

    static Bool HasTransientVertexIndexBufferThisFrame(
        const Vector<const MG_State::GLState::BufferObject*>& buffers,
        const MG_State::GLState::BufferObject* buffer) {
        return std::find(buffers.begin(), buffers.end(), buffer) != buffers.end();
    }

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

    static void RecordClearBufferError(const char* func, ErrorCode code, const char* message) {
        MG_State::pGLContext->RecordError(code, MakeUnique<GenericErrorInfo>("DirectVulkan", func, message));
    }

    static void RecordTextureCopyError(const char* func, ErrorCode code, const char* message) {
        MG_State::pGLContext->RecordError(code, MakeUnique<GenericErrorInfo>("DirectVulkan", func, message));
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
        static constexpr Uint32 kDescriptorSetsPerFrame = 64;
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

        static Uint32 ComputeMaxProgramBindings(const VkPhysicalDeviceProperties& properties) {
            const auto& limits = properties.limits;
            static constexpr Uint32 kMinProgramBindings = 16;
            static constexpr Uint32 kMaxProgramBindingsCap = 256;
            const Uint32 maxCombinedImageSamplers =
                std::min(limits.maxPerStageDescriptorSamplers, limits.maxDescriptorSetSamplers);
            const Uint32 maxSampledImages =
                std::min(limits.maxPerStageDescriptorSampledImages, limits.maxDescriptorSetSampledImages);
            const Uint32 maxDynamicUniformBuffers =
                std::min(limits.maxPerStageDescriptorUniformBuffers, limits.maxDescriptorSetUniformBuffersDynamic);

            Uint32 maxBindings = limits.maxPerStageResources;
            maxBindings = std::min(maxBindings, maxCombinedImageSamplers);
            maxBindings = std::min(maxBindings, maxSampledImages + maxDynamicUniformBuffers);

            maxBindings = std::max(kMinProgramBindings, maxBindings);
            maxBindings = std::min(kMaxProgramBindingsCap, maxBindings);
            return maxBindings;
        }

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
                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                    outSrcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                      VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                    outSrcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    break;
                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
                case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                    outSrcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                    outSrcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
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

        static void GetImageTransitionDestinationState(VkImageLayout newLayout, VkPipelineStageFlags& outDstStageMask,
                                                       VkAccessFlags& outDstAccessMask) {
            switch (newLayout) {
                case VK_IMAGE_LAYOUT_UNDEFINED:
                    outDstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    outDstAccessMask = 0;
                    break;
                case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                    outDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    outDstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    break;
                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                    outDstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                      VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                    outDstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    break;
                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
                case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                    outDstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                    outDstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    break;
                case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                    outDstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    outDstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    break;
                case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                    outDstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    outDstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    break;
                case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                    outDstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                    outDstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                    break;
                default:
                    outDstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                    outDstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
                    break;
            }
        }

        static VkImageAspectFlags GetSwapchainDepthStencilAspectMask(const SwapchainObject& swapchainObject) {
            VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            switch (swapchainObject.GetDepthStencilFormat()) {
                case VK_FORMAT_D24_UNORM_S8_UINT:
                case VK_FORMAT_D32_SFLOAT_S8_UINT:
                    aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                    break;
                default:
                    break;
            }
            return aspectMask;
        }

        static FramebufferAttachmentType ResolveFramebufferCopyAttachmentType(
            const MG_State::GLState::FramebufferObject& fbo, Bool isReadFramebuffer,
            VkImageAspectFlags aspectMask) {
            if ((aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) != 0) {
                return isReadFramebuffer ? fbo.GetReadBuffer() : fbo.GetDrawBuffers()[0];
            }
            if ((aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0) {
                return FramebufferAttachmentType::Depth;
            }
            if ((aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
                return FramebufferAttachmentType::Stencil;
            }
            return FramebufferAttachmentType::None;
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

        static Bool ResolveTextureCopyDestinationBinding(MG_State::GLState::ITextureObject& texture, Uint32 mipLevel,
                                                         VkTextureManager& textureManager, BlitImageBinding& outBinding) {
            auto* resource = textureManager.SyncTextureAndGetDescriptor(texture);
            if (resource == nullptr) {
                MGLOG_E("CopyTexSubImage2D skipped: failed to sync destination textureId=%d",
                        texture.GetExternalIndex());
                return false;
            }
            const VkImageAspectFlags copyAspectMask =
                resource->aspect & (VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
            if (copyAspectMask == 0) {
                MGLOG_E("CopyTexSubImage2D skipped: destination textureId=%d uses unsupported aspect mask=0x%x",
                        texture.GetExternalIndex());
                return false;
            }
            if (mipLevel >= resource->mipLevels) {
                MGLOG_E("CopyTexSubImage2D skipped: destination textureId=%d mip=%u out of range (mips=%u)",
                        texture.GetExternalIndex(), mipLevel, resource->mipLevels);
                return false;
            }

            outBinding.image = resource->image;
            outBinding.trackedLayout = &resource->layout;
            outBinding.aspectMask = copyAspectMask;
            outBinding.extent = {
                static_cast<Int>(std::max(1u, resource->extent.width >> mipLevel)),
                static_cast<Int>(std::max(1u, resource->extent.height >> mipLevel))};
            outBinding.mipLevel = mipLevel;
            outBinding.mipLevelCount = 1;
            outBinding.label = "destination texture";
            return true;
        }

        static Bool ResolveTextureCopySourceBinding(MG_State::GLState::FramebufferObject& fbo, Uint32 swapchainImageIndex,
                                                    SwapchainObject& swapchainObject,
                                                    VkTextureManager& textureManager,
                                                    VkImageAspectFlags requiredAspectMask,
                                                    BlitImageBinding& outBinding) {
            const Bool isDefaultFbo =
                (&fbo == MG_Impl::GLImpl::FramebufferImpl::pDefaultFramebufferInfo->defaultFBO.get());
            const auto attachmentType = ResolveFramebufferCopyAttachmentType(fbo, true, requiredAspectMask);
            if (attachmentType == FramebufferAttachmentType::None) {
                MGLOG_E("CopyTexSubImage2D skipped: unsupported source aspect mask=0x%x",
                        static_cast<Uint32>(requiredAspectMask));
                return false;
            }

            outBinding.label = "read";
            if (isDefaultFbo) {
                const auto extent = swapchainObject.GetExtent();
                outBinding.extent = {static_cast<Int>(extent.width), static_cast<Int>(extent.height)};
                outBinding.mipLevel = 0;
                outBinding.mipLevelCount = 1;
                outBinding.trackedLayout = nullptr;
                if ((requiredAspectMask & VK_IMAGE_ASPECT_COLOR_BIT) != 0) {
                    outBinding.image = swapchainObject.GetImage(swapchainImageIndex);
                    outBinding.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    return true;
                }

                const VkImageAspectFlags swapchainAspectMask = GetSwapchainDepthStencilAspectMask(swapchainObject);
                if ((swapchainAspectMask & requiredAspectMask) != requiredAspectMask) {
                    MGLOG_E("CopyTexSubImage2D skipped: swapchain depth image missing required aspect mask=0x%x",
                            static_cast<Uint32>(requiredAspectMask));
                    return false;
                }

                outBinding.image = swapchainObject.GetDepthStencilImage(swapchainImageIndex);
                outBinding.aspectMask = requiredAspectMask;
                return true;
            }

            const auto& attachment = fbo.GetAttachment(attachmentType);
            if (!attachment.IsComplete()) {
                MGLOG_E("CopyTexSubImage2D skipped: read framebuffer attachment %d is incomplete",
                        static_cast<Int>(attachmentType));
                return false;
            }
            if (attachment.IsRenderbuffer()) {
                MGLOG_E("CopyTexSubImage2D skipped: renderbuffer read attachments are not supported yet");
                return false;
            }
            if (!attachment.IsTexture()) {
                MGLOG_E("CopyTexSubImage2D skipped: unsupported read framebuffer attachment type");
                return false;
            }

            auto* texture = attachment.GetTexture().get();
            MOBILEGL_ASSERT(texture != nullptr, "ResolveTextureCopySourceBinding: source texture attachment is null");
            auto* resource = textureManager.SyncTextureAndGetDescriptor(*texture);
            if (resource == nullptr) {
                MGLOG_E("CopyTexSubImage2D skipped: failed to sync read framebuffer textureId=%d",
                        texture->GetExternalIndex());
                return false;
            }
            if ((resource->aspect & requiredAspectMask) != requiredAspectMask) {
                MGLOG_E("CopyTexSubImage2D skipped: read framebuffer textureId=%d aspect mask=0x%x does not satisfy requested mask=0x%x",
                        texture->GetExternalIndex(), static_cast<Uint32>(resource->aspect),
                        static_cast<Uint32>(requiredAspectMask));
                return false;
            }

            outBinding.image = resource->image;
            outBinding.trackedLayout = &resource->layout;
            outBinding.aspectMask = requiredAspectMask;
            const auto attachmentExtent = attachment.GetSize();
            outBinding.extent = {attachmentExtent.x(), attachmentExtent.y()};
            outBinding.mipLevel = static_cast<Uint32>(std::max(attachment.GetTextureLevel(), 0));
            outBinding.mipLevelCount = 1;
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
            .minUploadBytes = 4 * 1024 * 1024,
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

        const Uint32 maxProgramBindings = ComputeMaxProgramBindings(m_physicalDevice.properties);
        MGLOG_I("DirectVulkan: using %u program descriptor bindings", maxProgramBindings);

        RecreateSwapchain();

        m_pipelineFactory = MakeUnique<PipelineFactory>(m_device, m_config);
        MOBILEGL_ASSERT(m_pipelineFactory != nullptr, "PipelineFactory creation failed.");
        m_programFactory = MakeUnique<ProgramFactory>(m_device, m_config, maxProgramBindings);
        MOBILEGL_ASSERT(m_programFactory != nullptr, "ProgramFactory creation failed.");

        m_samplerManager = MakeUnique<VkSamplerManager>();
        MOBILEGL_ASSERT(m_samplerManager != nullptr, "VkSamplerManager creation failed.");
        succeeded = m_samplerManager->Initialize({m_device, &m_config});
        MOBILEGL_ASSERT(succeeded, "VkSamplerManager initialization failed.");
        succeeded = InitializeBlitResources();
        MOBILEGL_ASSERT(succeeded, "Blit pipeline resource initialization failed.");

        m_uniformManager = MakeUnique<UniformManager>();
        MOBILEGL_ASSERT(m_uniformManager != nullptr, "UniformDescriptorBinder creation failed.");
        succeeded = m_uniformManager->Initialize(
            m_device, &m_bufferManager, m_programFactory.get(),
            m_physicalDevice.properties.limits.minUniformBufferOffsetAlignment, m_config.MaxFramesInFlight,
            maxProgramBindings, kDescriptorSetsPerFrame, m_textureManager.get(), m_samplerManager.get());
        MOBILEGL_ASSERT(succeeded, "UniformDescriptorBinder initialization failed.");
        m_vertexInputStateFactory = MakeUnique<VertexInputStateFactory>(m_config);
        MOBILEGL_ASSERT(m_vertexInputStateFactory != nullptr, "VertexInputStateFactory creation failed.");

        // Prime the first frame so Render() always targets an acquired swapchain image.
        VK_VERIFY(m_frameContext.WaitAndAcquireNextImage(m_device, m_swapchainObject.GetHandle(), m_imageIndexAcquired),
                  "Initialize, WaitAndAcquireNextImage");
        m_bufferManager.BeginFrame(m_frameContext.GetCurrentFrameIndex());
        m_transientVertexIndexBuffersThisFrame.clear();

        MGLOG_D("VulkanRenderer initialized");
    }

    void VulkanRenderer::Shutdown() {
        VK_VERIFY(vkDeviceWaitIdle(m_device));

        m_pipelineFactory.reset();
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
        m_transientVertexIndexBuffersThisFrame.clear();

        m_frameContext.Destroy(m_device, m_commandPool);

        if (m_uniformManager) {
            m_uniformManager->Shutdown();
            m_uniformManager.reset();
        }
        m_programFactory.reset();

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

    Bool VulkanRenderer::UploadAndBindVertexBuffers(
        VkCommandBuffer commandBuffer, const MG_State::GLState::VertexArrayObject& vao) {
        auto& vertexInputState = m_vertexInputStateFactory->GetOrCreateVertexInputState(vao);

        const auto bindingCount = vertexInputState.bindings.size();

        Vector<VkBuffer> vkBuffers(bindingCount, VK_NULL_HANDLE);
        Vector<VkDeviceSize> vkOffsets(bindingCount, 0);

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
            auto sourceBufferShared = MG_State::pGLContext->GetBufferObject(sourceBuffer->GetExternalIndex());
            MOBILEGL_ASSERT(sourceBufferShared != nullptr,
                            "UploadAndBindVertexStreams failed to resolve shared source buffer");
            BufferSlice slice{};
            const Bool transientThisFrame =
                HasTransientVertexIndexBufferThisFrame(m_transientVertexIndexBuffersThisFrame, sourceBufferShared.get());
            const Bool isDirty = (sourceBufferShared->GetChangeBits() & BufferChangeBits::DirtyBit);
            if (ShouldUseTransientVertexIndexBuffer(*sourceBufferShared) || transientThisFrame || isDirty) {
                const auto sourceData = sourceBufferShared->GetDataReadOnly();
                const SizeT sourceSize = sourceBufferShared->GetSize();
                if (!m_bufferManager.UploadTransient(BufferKind::Vertex, m_frameContext.GetCurrentFrameIndex(),
                                                     sourceData->data(), static_cast<VkDeviceSize>(sourceSize), 16,
                                                     slice)) {
                    MOBILEGL_ASSERT(false, "UploadAndBindVertexStreams skipped: failed to upload transient binding %zu", binding);
                    return false;
                }
                if (!transientThisFrame) {
                    m_transientVertexIndexBuffersThisFrame.push_back(sourceBufferShared.get());
                }
                m_bufferManager.DowngradeResidentBufferToTransient(sourceBufferShared);
                sourceBufferShared->ClearDirty();
            } else {
                if (!m_bufferManager.SyncResidentBuffer(BufferKind::Vertex, sourceBufferShared, slice)) {
                    MGLOG_E("UploadAndBindVertexStreams skipped: failed to sync resident binding %zu", binding);
                    return false;
                }
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
        case GL_UNSIGNED_BYTE:
            MOBILEGL_ASSERT(m_indexTypeUint8ExtensionEnabled,
                            "DrawElements with GL_UNSIGNED_BYTE requires VK_KHR_index_type_uint8 or VK_EXT_index_type_uint8");
            vkIndexType = VK_INDEX_TYPE_UINT8;
            break;
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
        const SizeT indexSize = MG_Util::GetGLTypeSize(pIndexBufferView->indexType);
        const SizeT indexDataSizeBytes = pIndexBufferView->indexByteSize;
        MOBILEGL_ASSERT(pIndexBufferView->indexByteOffset + indexDataSizeBytes <= indexBuffer->GetSize(),
                        "DrawElements index range out of bounds");

        BufferSlice slice{};
        auto indexBufferShared = MG_State::pGLContext->GetBufferObject(indexBuffer->GetExternalIndex());
        MOBILEGL_ASSERT(indexBufferShared != nullptr, "UploadAndBindIndexBuffer failed to resolve shared EBO");
        const Bool transientThisFrame =
            HasTransientVertexIndexBufferThisFrame(m_transientVertexIndexBuffersThisFrame, indexBufferShared.get());
        const Bool isDirty = (indexBufferShared->GetChangeBits() & BufferChangeBits::DirtyBit);
        if (ShouldUseTransientVertexIndexBuffer(*indexBufferShared) || transientThisFrame || isDirty) {
            const auto indexData = indexBufferShared->GetDataReadOnly();
            MOBILEGL_ASSERT(indexData != nullptr && !indexData->empty(), "DrawElements requires non-empty EBO data");
            if (!m_bufferManager.UploadTransient(BufferKind::Index, m_frameContext.GetCurrentFrameIndex(),
                                                 indexData->data() + pIndexBufferView->indexByteOffset,
                                                 static_cast<VkDeviceSize>(indexDataSizeBytes), indexSize, slice)) {
                MOBILEGL_ASSERT(false, "DrawElements skipped: failed to prepare transient index buffer");
                return false;
            }
            if (!transientThisFrame) {
                m_transientVertexIndexBuffersThisFrame.push_back(indexBufferShared.get());
            }
            m_bufferManager.DowngradeResidentBufferToTransient(indexBufferShared);
            indexBufferShared->ClearDirty();
            vkCmdBindIndexBuffer(frame.commandBuffer, slice.buffer, slice.offset, vkIndexType);
            return true;
        }
        if (!m_bufferManager.SyncResidentBuffer(BufferKind::Index, indexBufferShared, slice)) {
            MGLOG_E("DrawElements skipped: failed to sync resident index buffer");
            return false;
        }
        vkCmdBindIndexBuffer(frame.commandBuffer, slice.buffer,
                             slice.offset + static_cast<VkDeviceSize>(pIndexBufferView->indexByteOffset), vkIndexType);
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
        MOBILEGL_ASSERT(m_programFactory != nullptr, "InitializeBlitResources: program factory is null");

        ProgramFactory::CompileOptionFlags blitTransformFlags = 0;
        const auto& blitProgramObj = m_programFactory->GetOrCreateProgram(*m_blitResources.program, blitTransformFlags);
        Bool foundBlitSamplerBinding = false;
        for (Uint32 binding = 0; binding < blitProgramObj.samplerNameByBinding.size(); ++binding) {
            if (blitProgramObj.bindingKinds[binding] != ProgramFactory::DescriptorBindingKind::CombinedImageSampler) {
                continue;
            }
            if (blitProgramObj.samplerNameByBinding[binding] == "uSource") {
                m_blitResources.samplerBinding = binding;
                foundBlitSamplerBinding = true;
                break;
            }
        }
        MOBILEGL_ASSERT(foundBlitSamplerBinding,
                        "InitializeBlitResources: failed to resolve reflected binding for uSource");

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
        return true;
    }

    void VulkanRenderer::ShutdownBlitResources() {
        m_blitResources = {};
    }

    VkPipeline VulkanRenderer::GetOrCreateBlitPipeline(const RenderPassEntry& renderPassEntry) {
        MOBILEGL_ASSERT(m_blitResources.program != nullptr, "GetOrCreateBlitPipeline: blit program is null");
        MOBILEGL_ASSERT(m_programFactory != nullptr, "GetOrCreateBlitPipeline: program factory is null");
        MOBILEGL_ASSERT(m_uniformManager != nullptr, "GetOrCreateBlitPipeline: descriptor binder is null");

        static const VkPipelineVertexInputStateCreateInfo kEmptyVertexInputState {
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
        };
        ProgramFactory::CompileOptionFlags transformFlags = 0;
        const auto& programObj = m_programFactory->GetOrCreateProgram(*m_blitResources.program, transformFlags);
        PipelineFactory::PipelineCreatePayload payload{
            .programHash = programObj.hash,
            .vertexInputHash = 0,
            .pipelineLayout = programObj.pipelineLayout,
            .renderPass = renderPassEntry.renderPass,
            .colorAttachmentCount = renderPassEntry.colorAttachmentCount,
            .subpass = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthTestEnable = false,
            .depthWriteEnable = false,
            .depthCompareOp = VK_COMPARE_OP_ALWAYS,
            .stages = &programObj.stages,
            .vertexInputState = &kEmptyVertexInputState
        };
        static constexpr VkColorComponentFlags kColorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        MOBILEGL_ASSERT(payload.colorAttachmentCount <= PipelineFactory::PipelineCreatePayload::kMaxColorAttachments,
                        "GetOrCreateBlitPipeline: colorAttachmentCount=%u exceeds payload capacity",
                        payload.colorAttachmentCount);
        for (Uint32 i = 0; i < payload.colorAttachmentCount; ++i) {
            payload.colorBlendAttachments[i] = MakeColorBlendAttachmentState(
                false,
                VK_BLEND_FACTOR_ONE,
                VK_BLEND_FACTOR_ZERO,
                VK_BLEND_FACTOR_ONE,
                VK_BLEND_FACTOR_ZERO,
                kColorWriteMask);
        }
        return m_pipelineFactory->GetOrCreatePipeline(payload);
    }

    VkPipeline VulkanRenderer::GetOrCreatePipeline(
            GLenum mode,
            const MG_State::GLState::ProgramObject& program,
            const MG_State::GLState::VertexArrayObject& vao,
            const RenderPassEntry& renderPassEntry) {
        ProgramFactory::CompileOptionFlags transformFlags = GetShaderTransformFlags(m_swapchainObject.GetPreTransform());
        Bool invertClockwise = transformFlags & ProgramFactory::CompileOptionBit::PositionYFlip;
        const auto& programObj = m_programFactory->GetOrCreateProgram(program, transformFlags);
        if (programObj.stages.empty()) {
            MGLOG_D("GetOrCreatePipeline skipped: program has no shader stages");
            return VK_NULL_HANDLE;
        }

        auto vertexInputHash = m_vertexInputStateFactory->ComputeHash(vao);
        auto& vis = m_vertexInputStateFactory->GetOrCreateVertexInputState(vao);
        auto cullFaceEnabled = MG_State::pGLContext->IsCapabilityEnabled(CapabilityInput::CullFace);
        auto depthTestEnabled = MG_State::pGLContext->IsCapabilityEnabled(CapabilityInput::DepthTest);
        auto mask = MG_State::pGLContext->GetColorMask();
        const auto colorWriteMask = static_cast<VkColorComponentFlags>(
            (mask.r() ? VK_COLOR_COMPONENT_R_BIT : 0u) |
            (mask.g() ? VK_COLOR_COMPONENT_G_BIT : 0u) |
            (mask.b() ? VK_COLOR_COMPONENT_B_BIT : 0u) |
            (mask.a() ? VK_COLOR_COMPONENT_A_BIT : 0u));

        PipelineFactory::PipelineCreatePayload payload {
            .programHash = programObj.hash,
            .vertexInputHash = vertexInputHash,
            .pipelineLayout = programObj.pipelineLayout,
            .renderPass = renderPassEntry.renderPass,
            .colorAttachmentCount = renderPassEntry.colorAttachmentCount,
            .subpass = 0,
            .topology = MG_Util::ConvertPrimitiveModeToVkEnum(mode),
            .cullMode = cullFaceEnabled
                ? MG_Util::ConvertCullFaceModeToVkEnum(MG_State::pGLContext->GetCullFaceMode(), invertClockwise)
                : VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthTestEnable = depthTestEnabled,
            .depthWriteEnable = depthTestEnabled && MG_State::pGLContext->GetDepthMask(),
            .depthCompareOp = MG_Util::ConvertDepthTestFuncToVkEnum(MG_State::pGLContext->GetDepthFunc()),
            .stages = &programObj.stages,
            .vertexInputState = &vis.state
        };
        MOBILEGL_ASSERT(payload.colorAttachmentCount <= PipelineFactory::PipelineCreatePayload::kMaxColorAttachments,
                        "GetOrCreatePipeline: colorAttachmentCount=%u exceeds payload capacity",
                        payload.colorAttachmentCount);
        for (Uint32 i = 0; i < payload.colorAttachmentCount; ++i) {
            BlendFactor srcRGB = BlendFactor::One;
            BlendFactor dstRGB = BlendFactor::Zero;
            BlendFactor srcAlpha = BlendFactor::One;
            BlendFactor dstAlpha = BlendFactor::Zero;
            MG_State::pGLContext->GetBlendFuncIndexed(i, srcRGB, dstRGB, srcAlpha, dstAlpha);
            payload.colorBlendAttachments[i] = MakeColorBlendAttachmentState(
                MG_State::pGLContext->IsCapabilityEnabledIndexed(CapabilityInput::Blend, i),
                MG_Util::ConvertBlendFactorToVkEnum(srcRGB),
                MG_Util::ConvertBlendFactorToVkEnum(dstRGB),
                MG_Util::ConvertBlendFactorToVkEnum(srcAlpha),
                MG_Util::ConvertBlendFactorToVkEnum(dstAlpha),
                colorWriteMask);
        }
        return m_pipelineFactory->GetOrCreatePipeline(payload);
    }

    Bool VulkanRenderer::SetupDraw(FrameContext::FrameData& frame, GLenum mode, Flags<DrawSetupAspect> aspects,
                                   const IndexBufferView* pIndexBufferView) {
        m_textureManager->CollectGarbage();
        const auto& drawFbo =
                MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw).GetBoundObject();
        const auto& vao = *MG_State::pGLContext->GetBoundVertexArray();
        const auto& program = *MG_State::pGLContext->GetCurrentProgram();
        ProgramFactory::CompileOptionFlags transformFlags = GetShaderTransformFlags(m_swapchainObject.GetPreTransform());
        const auto& programObj = m_programFactory->GetOrCreateProgram(program, transformFlags);

        // Begin command recording if not yet
        if (!frame.isCommandRecording) {
            m_frameContext.BeginCommandRecording();
            m_uniformManager->BeginFrame(m_frameContext.GetCurrentFrameIndex());
        }

        auto* activeRenderPass = VkRenderPassManager::GetActiveRenderPass();

        // Check if any of the textures to sample have pending clears,
        // which probably indicates it's been gone through codepath like `fbo attach` -> `clear` -> `fbo detach`, and
        // without draws in between to give it a chance to materialize such clear.
        // Deal with this situation here.
        Vector<MG_State::GLState::ITextureObject*> sampledTextures;
        Bool hasSampledTextures = m_uniformManager->CollectSampledTextures(program, programObj, sampledTextures);
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

        const Bool boundUniforms = m_uniformManager->BindProgramUniformBuffers(
            frame.commandBuffer, program, programObj, m_frameContext.GetCurrentFrameIndex());
        if (!boundUniforms) {
            MGLOG_E("SetupDraw skipped: BindProgramUniformBuffers failed");
            return false;
        }

        auto vtxUploadOk = UploadAndBindVertexBuffers(frame.commandBuffer, vao);
        MOBILEGL_ASSERT(vtxUploadOk, "SetupDraw skipped: failed to upload vertex buffers");

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
        return true;
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

    void VulkanRenderer::QueueClearBufferPayload(GLenum buffer, GLint drawbuffer,
                                                 const ClearAttachmentPayload& clearPayload) {
        m_clearManager->CollectGarbage();
        auto* fbo = MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw).GetBoundObject().get();
        MOBILEGL_ASSERT(fbo, "VulkanRenderer::QueueClearBufferPayload: draw framebuffer not found");

        auto queueAttachmentClear = [&](FramebufferAttachmentType attachmentType) {
            if (attachmentType == FramebufferAttachmentType::None) {
                return;
            }
            const auto& attachment = fbo->GetAttachment(attachmentType);
            if (!attachment.IsTexture() || attachment.IsRenderbuffer()) {
                return;
            }
            auto texture = attachment.GetTexture();
            if (!texture) {
                return;
            }
            m_clearManager->QueueClear(clearPayload, texture);
        };

        switch (buffer) {
            case GL_COLOR: {
                if (drawbuffer < 0 ||
                    drawbuffer >= static_cast<GLint>(MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS)) {
                    RecordClearBufferError(__func__, ErrorCode::InvalidValue, "color drawbuffer index is out of range");
                    return;
                }
                queueAttachmentClear(fbo->GetDrawBuffers()[drawbuffer]);
                return;
            }
            case GL_DEPTH:
                if (drawbuffer != 0) {
                    RecordClearBufferError(__func__, ErrorCode::InvalidValue, "depth clear requires drawbuffer 0");
                    return;
                }
                queueAttachmentClear(FramebufferAttachmentType::Depth);
                return;
            case GL_STENCIL:
                if (drawbuffer != 0) {
                    RecordClearBufferError(__func__, ErrorCode::InvalidValue, "stencil clear requires drawbuffer 0");
                    return;
                }
                queueAttachmentClear(FramebufferAttachmentType::Stencil);
                return;
            case GL_DEPTH_STENCIL:
                if (drawbuffer != 0) {
                    RecordClearBufferError(__func__, ErrorCode::InvalidValue, "depth/stencil clear requires drawbuffer 0");
                    return;
                }
                queueAttachmentClear(FramebufferAttachmentType::Depth);
                queueAttachmentClear(FramebufferAttachmentType::Stencil);
                return;
            default:
                RecordClearBufferError(__func__, ErrorCode::InvalidEnum, "unsupported clear buffer target");
                return;
        }
    }

    void VulkanRenderer::ClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
        ClearAttachmentPayload payload{};
        payload.mask = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
        payload.depth = depth;
        payload.stencil = static_cast<Uint32>(stencil);
        QueueClearBufferPayload(buffer, drawbuffer, payload);
    }

    void VulkanRenderer::ClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value) {
        if (value == nullptr) {
            return;
        }
        ClearAttachmentPayload payload{};
        switch (buffer) {
            case GL_COLOR:
                payload.mask = GL_COLOR_BUFFER_BIT;
                payload.color = FloatVec4(value[0], value[1], value[2], value[3]);
                break;
            case GL_DEPTH:
                payload.mask = GL_DEPTH_BUFFER_BIT;
                payload.depth = value[0];
                break;
            default:
                break;
        }
        QueueClearBufferPayload(buffer, drawbuffer, payload);
    }

    void VulkanRenderer::ClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value) {
        if (value == nullptr) {
            return;
        }
        ClearAttachmentPayload payload{};
        switch (buffer) {
            case GL_COLOR:
                payload.mask = GL_COLOR_BUFFER_BIT;
                payload.color = FloatVec4(static_cast<Float>(value[0]), static_cast<Float>(value[1]),
                                          static_cast<Float>(value[2]), static_cast<Float>(value[3]));
                break;
            case GL_STENCIL:
                payload.mask = GL_STENCIL_BUFFER_BIT;
                payload.stencil = value[0];
                break;
            default:
                break;
        }
        QueueClearBufferPayload(buffer, drawbuffer, payload);
    }

    void VulkanRenderer::ClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value) {
        if (value == nullptr) {
            return;
        }
        ClearAttachmentPayload payload{};
        switch (buffer) {
            case GL_COLOR:
                payload.mask = GL_COLOR_BUFFER_BIT;
                payload.color = FloatVec4(static_cast<Float>(value[0]), static_cast<Float>(value[1]),
                                          static_cast<Float>(value[2]), static_cast<Float>(value[3]));
                break;
            case GL_STENCIL:
                payload.mask = GL_STENCIL_BUFFER_BIT;
                payload.stencil = static_cast<Uint32>(std::max(value[0], 0));
                break;
            default:
                break;
        }
        QueueClearBufferPayload(buffer, drawbuffer, payload);
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
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;

        VkImageLayout sampledLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        if ((resource->aspect & VK_IMAGE_ASPECT_COLOR_BIT) != 0) {
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            VkClearColorValue clearValue{};
            clearValue.float32[0] = clearPayload.color.x();
            clearValue.float32[1] = clearPayload.color.y();
            clearValue.float32[2] = clearPayload.color.z();
            clearValue.float32[3] = clearPayload.color.w();
            vkCmdClearColorImage(commandBuffer, resource->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 &clearValue, 1, &subresourceRange);
        } else {
            VkImageAspectFlags clearAspectMask = 0;
            if ((resource->aspect & VK_IMAGE_ASPECT_DEPTH_BIT) != 0 &&
                (clearPayload.mask & GL_DEPTH_BUFFER_BIT) != 0) {
                clearAspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            if ((resource->aspect & VK_IMAGE_ASPECT_STENCIL_BIT) != 0 &&
                (clearPayload.mask & GL_STENCIL_BUFFER_BIT) != 0) {
                clearAspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            MOBILEGL_ASSERT(clearAspectMask != 0,
                            "MaterializePendingClearForTexture: textureId=%d has no matching depth/stencil clear mask",
                            texture.GetExternalIndex());
            subresourceRange.aspectMask = clearAspectMask;
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
        const VkImageView sourceImageView =
            m_textureManager->GetOrCreateSampledViewAtMipLevel(*sourceTexture, srcBinding.mipLevel);
        MOBILEGL_ASSERT(sourceImageView != VK_NULL_HANDLE,
                        "TryBlitToDefaultFramebufferWithShader: failed to create sampled view for textureId=%d mip=%u",
                        sourceTexture->GetExternalIndex(), srcBinding.mipLevel);

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

        const auto samplerBindingOverride = UniformManager::SamplerBindingOverride{
            .binding = m_blitResources.samplerBinding,
            .texture = sourceTexture.get(),
            .sampler = (filter == GL_LINEAR ? m_blitResources.linearSampler.get()
                                            : m_blitResources.nearestSampler.get()),
            .imageView = sourceImageView,
        };
        ProgramFactory::CompileOptionFlags blitTransformFlags = 0;
        const auto& blitProgramObj = m_programFactory->GetOrCreateProgram(*m_blitResources.program, blitTransformFlags);
        const Bool bound = m_uniformManager->BindProgramUniformBuffers(
            frame.commandBuffer, *m_blitResources.program, blitProgramObj, m_frameContext.GetCurrentFrameIndex(),
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
            m_uniformManager->BeginFrame(m_frameContext.GetCurrentFrameIndex());
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
            m_swapchainObject.SetImageLayout(m_imageIndexAcquired, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
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
            m_swapchainObject.SetImageLayout(m_imageIndexAcquired, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
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

    void VulkanRenderer::CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
                                           GLint x, GLint y, GLsizei width, GLsizei height) {
        if (width <= 0 || height <= 0) {
            return;
        }

        const auto textureTarget = MG_Util::ConvertGLEnumToTextureTarget(target);
        if (textureTarget != TextureTarget::Texture2D) {
            RecordTextureCopyError(__func__, ErrorCode::InvalidOperation,
                                   "CopyTexSubImage2D currently only supports GL_TEXTURE_2D destinations.");
            return;
        }
        if (level < 0) {
            RecordTextureCopyError(__func__, ErrorCode::InvalidValue,
                                   "CopyTexSubImage2D level must be non-negative.");
            return;
        }

        auto& textureUnit = MG_State::pGLContext->GetTextureUnitObject(MG_State::pGLContext->GetActiveTextureUnit());
        auto destinationTexture = textureUnit.GetBindingSlot(textureTarget).GetBoundObject();
        if (destinationTexture == nullptr) {
            RecordTextureCopyError(__func__, ErrorCode::InvalidOperation,
                                   "CopyTexSubImage2D requires a bound destination texture.");
            return;
        }

        auto readFbo = MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Read).GetBoundObject();
        if (readFbo == nullptr) {
            RecordTextureCopyError(__func__, ErrorCode::InvalidOperation,
                                   "CopyTexSubImage2D requires a framebuffer bound to GL_READ_FRAMEBUFFER.");
            return;
        }

        auto& frame = m_frameContext.GetCurrent();
        if (!frame.isCommandRecording) {
            m_frameContext.BeginCommandRecording();
            m_uniformManager->BeginFrame(m_frameContext.GetCurrentFrameIndex());
        }

        if (VkRenderPassManager::GetActiveRenderPass() != nullptr) {
            VkRenderPassManager::EndRenderPass(frame.commandBuffer);
        }

        const Bool readIsDefaultFbo =
            (readFbo == MG_Impl::GLImpl::FramebufferImpl::pDefaultFramebufferInfo->defaultFBO);

        BlitImageBinding dstBinding{};
        if (!ResolveTextureCopyDestinationBinding(*destinationTexture, static_cast<Uint32>(level), *m_textureManager,
                                                  dstBinding)) {
            RecordTextureCopyError(__func__, ErrorCode::InvalidOperation,
                                   "CopyTexSubImage2D failed to resolve the destination texture.");
            return;
        }

        BlitImageBinding srcBinding{};
        if (!ResolveTextureCopySourceBinding(*readFbo, m_imageIndexAcquired, m_swapchainObject, *m_textureManager,
                                             dstBinding.aspectMask, srcBinding)) {
            RecordTextureCopyError(__func__, ErrorCode::InvalidOperation,
                                   "CopyTexSubImage2D requires a complete read attachment compatible with the destination texture.");
            return;
        }

        if (!readIsDefaultFbo) {
            const auto sourceAttachmentType = ResolveFramebufferCopyAttachmentType(*readFbo, true, srcBinding.aspectMask);
            const auto& sourceAttachment = readFbo->GetAttachment(sourceAttachmentType);
            auto sourceTexture = sourceAttachment.GetTexture();
            MOBILEGL_ASSERT(sourceTexture != nullptr, "CopyTexSubImage2D: source texture attachment is null");
            const Bool clearReady = MaterializePendingClearForTexture(frame.commandBuffer, *sourceTexture);
            MOBILEGL_ASSERT(clearReady,
                            "CopyTexSubImage2D: failed to materialize pending clear for source textureId=%d",
                            sourceTexture->GetExternalIndex());
        }

        const Bool srcUsesSwapchainDepth = readIsDefaultFbo && (srcBinding.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) == 0;
        const VkImageLayout srcOriginalLayout = readIsDefaultFbo
            ? (srcUsesSwapchainDepth
                ? m_swapchainObject.GetDepthStencilImageLayout(m_imageIndexAcquired)
                : m_swapchainObject.GetImageLayout(m_imageIndexAcquired))
            : *srcBinding.trackedLayout;
        if (srcOriginalLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            RecordTextureCopyError(__func__, ErrorCode::InvalidOperation,
                                   "CopyTexSubImage2D source image has undefined layout.");
            return;
        }

        const VkImageLayout dstOriginalLayout = *dstBinding.trackedLayout;
        const VkImageLayout dstRestoreLayout = dstOriginalLayout == VK_IMAGE_LAYOUT_UNDEFINED
            ? ((dstBinding.aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0
                ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            : dstOriginalLayout;

        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkAccessFlags srcAccessMask = 0;
        GetImageTransitionSourceState(srcOriginalLayout, srcStageMask, srcAccessMask);
        if (readIsDefaultFbo) {
            VkImageLayout srcTrackedLayout = srcOriginalLayout;
            Bool ok = VkTextureManager::TransitionImageLayout(
                frame.commandBuffer, srcBinding.image, srcTrackedLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                srcStageMask, VK_PIPELINE_STAGE_TRANSFER_BIT,
                srcAccessMask, VK_ACCESS_TRANSFER_READ_BIT, srcBinding.aspectMask,
                srcBinding.mipLevel, srcBinding.mipLevelCount);
            MOBILEGL_ASSERT(ok, "%s: failed to transition swapchain source image", __func__);
            if (srcUsesSwapchainDepth) {
                m_swapchainObject.SetDepthStencilImageLayout(m_imageIndexAcquired, srcTrackedLayout);
            } else {
                m_swapchainObject.SetImageLayout(m_imageIndexAcquired, srcTrackedLayout);
            }
        } else {
            Bool ok = VkTextureManager::TransitionImageLayout(
                frame.commandBuffer, srcBinding.image, *srcBinding.trackedLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                srcStageMask, VK_PIPELINE_STAGE_TRANSFER_BIT,
                srcAccessMask, VK_ACCESS_TRANSFER_READ_BIT, srcBinding.aspectMask,
                srcBinding.mipLevel, srcBinding.mipLevelCount);
            MOBILEGL_ASSERT(ok, "%s: failed to transition source image", __func__);
        }

        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkAccessFlags dstAccessMask = 0;
        GetImageTransitionSourceState(dstOriginalLayout, dstStageMask, dstAccessMask);
        Bool dstReady = VkTextureManager::TransitionImageLayout(
            frame.commandBuffer, dstBinding.image, *dstBinding.trackedLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            dstStageMask, VK_PIPELINE_STAGE_TRANSFER_BIT,
            dstAccessMask, VK_ACCESS_TRANSFER_WRITE_BIT, dstBinding.aspectMask,
            dstBinding.mipLevel, dstBinding.mipLevelCount);
        MOBILEGL_ASSERT(dstReady, "%s: failed to transition destination image", __func__);

        VkImageCopy copyRegion{};
        copyRegion.srcSubresource.aspectMask = srcBinding.aspectMask;
        copyRegion.srcSubresource.mipLevel = srcBinding.mipLevel;
        copyRegion.srcSubresource.baseArrayLayer = 0;
        copyRegion.srcSubresource.layerCount = 1;
        copyRegion.srcOffset = {x, y, 0};
        copyRegion.dstSubresource.aspectMask = dstBinding.aspectMask;
        copyRegion.dstSubresource.mipLevel = dstBinding.mipLevel;
        copyRegion.dstSubresource.baseArrayLayer = 0;
        copyRegion.dstSubresource.layerCount = 1;
        copyRegion.dstOffset = {xoffset, yoffset, 0};
        copyRegion.extent = {static_cast<Uint32>(width), static_cast<Uint32>(height), 1};
        vkCmdCopyImage(frame.commandBuffer,
                       srcBinding.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       dstBinding.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &copyRegion);

        VkPipelineStageFlags srcRestoreStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkAccessFlags srcRestoreAccessMask = 0;
        GetImageTransitionDestinationState(srcOriginalLayout, srcRestoreStageMask, srcRestoreAccessMask);
        if (readIsDefaultFbo) {
            VkImageLayout srcTrackedLayout = srcUsesSwapchainDepth
                ? m_swapchainObject.GetDepthStencilImageLayout(m_imageIndexAcquired)
                : m_swapchainObject.GetImageLayout(m_imageIndexAcquired);
            Bool ok = VkTextureManager::TransitionImageLayout(
                frame.commandBuffer, srcBinding.image, srcTrackedLayout, srcOriginalLayout,
                VK_PIPELINE_STAGE_TRANSFER_BIT, srcRestoreStageMask,
                VK_ACCESS_TRANSFER_READ_BIT, srcRestoreAccessMask, srcBinding.aspectMask,
                srcBinding.mipLevel, srcBinding.mipLevelCount);
            MOBILEGL_ASSERT(ok, "%s: failed to restore swapchain source image layout", __func__);
            if (srcUsesSwapchainDepth) {
                m_swapchainObject.SetDepthStencilImageLayout(m_imageIndexAcquired, srcTrackedLayout);
            } else {
                m_swapchainObject.SetImageLayout(m_imageIndexAcquired, srcTrackedLayout);
            }
        } else {
            Bool ok = VkTextureManager::TransitionImageLayout(
                frame.commandBuffer, srcBinding.image, *srcBinding.trackedLayout, srcOriginalLayout,
                VK_PIPELINE_STAGE_TRANSFER_BIT, srcRestoreStageMask,
                VK_ACCESS_TRANSFER_READ_BIT, srcRestoreAccessMask, srcBinding.aspectMask,
                srcBinding.mipLevel, srcBinding.mipLevelCount);
            MOBILEGL_ASSERT(ok, "%s: failed to restore source image layout", __func__);
        }

        VkPipelineStageFlags dstRestoreStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkAccessFlags dstRestoreAccessMask = 0;
        GetImageTransitionDestinationState(dstRestoreLayout, dstRestoreStageMask, dstRestoreAccessMask);
        Bool dstRestored = VkTextureManager::TransitionImageLayout(
            frame.commandBuffer, dstBinding.image, *dstBinding.trackedLayout, dstRestoreLayout,
            VK_PIPELINE_STAGE_TRANSFER_BIT, dstRestoreStageMask,
            VK_ACCESS_TRANSFER_WRITE_BIT, dstRestoreAccessMask, dstBinding.aspectMask,
            dstBinding.mipLevel, dstBinding.mipLevelCount);
        MOBILEGL_ASSERT(dstRestored, "%s: failed to restore destination image layout", __func__);
    }

    void VulkanRenderer::DrawArrays(const DrawCmd& payload) {
        auto& frame = m_frameContext.GetCurrent();

        if (!SetupDraw(frame, payload.mode, 0)) {
            return;
        }

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

        if (!SetupDraw(frame, payload.mode, DrawSetupAspect::IndexBuffer,
                       &payload.indexBufferView)) {
            return;
        }

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

        if (!SetupDraw(frame, payload.mode, DrawSetupAspect::IndexBuffer,
                  &payload.indexBufferView)) {
            return;
        }

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
        m_transientVertexIndexBuffersThisFrame.clear();
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

        m_indexTypeUint8ExtensionEnabled = false;
        const char* indexTypeUint8ExtensionName = nullptr;
        if (IsExtensionSupported(availableExtensions, VK_KHR_INDEX_TYPE_UINT8_EXTENSION_NAME)) {
            indexTypeUint8ExtensionName = VK_KHR_INDEX_TYPE_UINT8_EXTENSION_NAME;
        } else if (IsExtensionSupported(availableExtensions, VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME)) {
            indexTypeUint8ExtensionName = VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME;
        }

        VkPhysicalDeviceIndexTypeUint8Features indexTypeUint8Features{};
        indexTypeUint8Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES;
        if (indexTypeUint8ExtensionName != nullptr) {
            VkPhysicalDeviceFeatures2 featureQuery{};
            featureQuery.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            featureQuery.pNext = &indexTypeUint8Features;
            auto getPhysicalDeviceFeatures2 = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2>(
                vkGetInstanceProcAddr(m_instance, "vkGetPhysicalDeviceFeatures2"));
            if (getPhysicalDeviceFeatures2 == nullptr) {
                getPhysicalDeviceFeatures2 = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2>(
                    vkGetInstanceProcAddr(m_instance, "vkGetPhysicalDeviceFeatures2KHR"));
            }
            MOBILEGL_ASSERT(getPhysicalDeviceFeatures2 != nullptr,
                            "CreateLogicalDeviceAndQueues: vkGetPhysicalDeviceFeatures2 is unavailable");
            getPhysicalDeviceFeatures2(m_physicalDevice.handle, &featureQuery);
            if (indexTypeUint8Features.indexTypeUint8 == VK_TRUE) {
                if (!IsExtensionAlreadyEnabled(enabledDeviceExtensions, indexTypeUint8ExtensionName)) {
                    enabledDeviceExtensions.push_back(indexTypeUint8ExtensionName);
                }
                m_indexTypeUint8ExtensionEnabled = true;
                indexTypeUint8Features.pNext = const_cast<void*>(deviceCreateInfo.pNext);
                deviceCreateInfo.pNext = &indexTypeUint8Features;
                MGLOG_I("Enabled optional device extension: %s", indexTypeUint8ExtensionName);
            } else {
                MGLOG_W("%s is advertised, but indexTypeUint8 feature is unavailable; uint8 index buffers will stay disabled",
                        indexTypeUint8ExtensionName);
            }
        } else {
            MGLOG_W("VK_KHR_index_type_uint8 / VK_EXT_index_type_uint8 not supported; uint8 index buffers will stay disabled");
        }

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
        MGLOG_I("index type uint8 enabled: %s", m_indexTypeUint8ExtensionEnabled ? "true" : "false");
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
            m_transientVertexIndexBuffersThisFrame.clear();
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
            if ((clearPayload.mask & GL_COLOR_BUFFER_BIT) != 0) {
                clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                clearAttachment.colorAttachment = pending.attachmentIndex;
                clearAttachment.clearValue.color = {
                        clearPayload.color.x(),
                        clearPayload.color.y(),
                        clearPayload.color.z(),
                        clearPayload.color.w()
                };
            } else {
                if ((clearPayload.mask & GL_DEPTH_BUFFER_BIT) != 0) {
                    clearAttachment.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
                    clearAttachment.clearValue.depthStencil.depth = clearPayload.depth;
                }
                if ((clearPayload.mask & GL_STENCIL_BUFFER_BIT) != 0) {
                    clearAttachment.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                    clearAttachment.clearValue.depthStencil.stencil = clearPayload.stencil;
                }
                if (clearAttachment.aspectMask == 0) {
                    continue;
                }
            }

            vkCmdClearAttachments(commandBuffer, 1, &clearAttachment, 1, &clearRect);
            m_clearManager->PopPendingClear(pending.texture);
        }
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
