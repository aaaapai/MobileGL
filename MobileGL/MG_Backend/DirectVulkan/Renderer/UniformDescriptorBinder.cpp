// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/UniformDescriptorBinder.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "UniformDescriptorBinder.h"

#include "MG_State/GLState/Core.h"
#include "MG_State/GLState/ProgramState/ProgramObject.h"
#include "MG_Util/Converters/MGToStr/FramebufferEnumConverter.h"
#include <limits>

namespace MobileGL::MG_Backend::DirectVulkan {
    static Bool FindFramebufferAttachmentForTexture(const MG_State::GLState::FramebufferObject& framebuffer,
                                                    const MG_State::GLState::ITextureObject& texture,
                                                    FramebufferAttachmentType& outAttachment, Int& outLevel) {
        const auto& attachments = framebuffer.GetAllAttachmentObjects();
        for (SizeT i = 0; i < attachments.size(); ++i) {
            const auto attachmentType = static_cast<FramebufferAttachmentType>(i);
            if (attachmentType == FramebufferAttachmentType::None) {
                continue;
            }

            const auto& attachment = attachments[i];
            if (!attachment.IsTexture()) {
                continue;
            }

            auto attachedTexture = attachment.GetTexture();
            if (attachedTexture && attachedTexture.get() == &texture) {
                outAttachment = attachmentType;
                outLevel = attachment.GetTextureLevel();
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

    Bool UniformDescriptorBinder::Initialize(VkDevice device, VkBufferManager* bufferManager,
                                             ProgramFactory* programFactory,
                                             VkDeviceSize minUniformBufferOffsetAlignment, Uint32 frameCount,
                                             Uint32 maxBindings, Uint32 setsPerFrame,
                                             VkTextureManager* textureManager, VkSamplerManager* samplerManager) {
        Shutdown();

        MOBILEGL_ASSERT(device != VK_NULL_HANDLE, "UniformDescriptorBinder::Initialize requires valid VkDevice");
        MOBILEGL_ASSERT(bufferManager != nullptr, "UniformDescriptorBinder::Initialize requires valid buffer manager");
        MOBILEGL_ASSERT(programFactory != nullptr,
                        "UniformDescriptorBinder::Initialize requires valid program factory");
        MOBILEGL_ASSERT(frameCount > 0, "UniformDescriptorBinder::Initialize requires frameCount > 0");
        MOBILEGL_ASSERT(maxBindings > 0, "UniformDescriptorBinder::Initialize requires maxBindings > 0");
        MOBILEGL_ASSERT(setsPerFrame > 0, "UniformDescriptorBinder::Initialize requires setsPerFrame > 0");
        MOBILEGL_ASSERT(textureManager != nullptr,
                        "UniformDescriptorBinder::Initialize requires valid texture manager");
        MOBILEGL_ASSERT(samplerManager != nullptr,
                        "UniformDescriptorBinder::Initialize requires valid sampler manager");

        m_device = device;
        m_bufferManager = bufferManager;
        m_programFactory = programFactory;
        m_minDynamicOffsetAlignment = std::max<VkDeviceSize>(1, minUniformBufferOffsetAlignment);
        m_frameCount = frameCount;
        m_maxBindings = maxBindings;
        m_setsPerFrame = setsPerFrame;
        m_peakDescriptorSetsObserved = 0;
        m_textureManager = textureManager;
        m_samplerManager = samplerManager;

        m_frames.resize(m_frameCount);
        for (Uint32 frameIndex = 0; frameIndex < m_frameCount; ++frameIndex) {
            auto& frame = m_frames[frameIndex];
            frame.activeDescriptorPoolIndex = 0;
            frame.allocatedSetsThisFrame = 0;
            frame.peakAllocatedSetsThisFrame = 0;
            frame.descriptorPools.clear();

            VkDescriptorPool initialPool = VK_NULL_HANDLE;
            if (!CreateDescriptorPool(m_setsPerFrame, initialPool)) {
                MGLOG_E("UniformDescriptorBinder::Initialize failed: cannot create frame descriptor pool %u",
                        frameIndex);
                Shutdown();
                return false;
            }
            frame.descriptorPools.push_back({initialPool, m_setsPerFrame, 0});
            MGLOG_D("UniformDescriptorBinder: frame %u descriptor pool created (maxSets=%u)", frameIndex,
                    m_setsPerFrame);
        }

        return true;
    }

    void UniformDescriptorBinder::Shutdown() {
        for (auto& frame : m_frames) {
            if (m_device != VK_NULL_HANDLE) {
                for (auto& bucket : frame.descriptorPools) {
                    if (bucket.handle != VK_NULL_HANDLE) {
                        vkDestroyDescriptorPool(m_device, bucket.handle, nullptr);
                        bucket.handle = VK_NULL_HANDLE;
                    }
                }
            }
            frame.descriptorPools.clear();
            frame.activeDescriptorPoolIndex = 0;
            frame.allocatedSetsThisFrame = 0;
            frame.peakAllocatedSetsThisFrame = 0;
        }
        m_frames.clear();

        m_bufferManager = nullptr;
        m_programFactory = nullptr;
        m_device = VK_NULL_HANDLE;
        m_minDynamicOffsetAlignment = 1;
        m_frameCount = 0;
        m_maxBindings = 0;
        m_setsPerFrame = 0;
        m_peakDescriptorSetsObserved = 0;
        m_textureManager = nullptr;
        m_samplerManager = nullptr;
    }

    void UniformDescriptorBinder::BeginFrame(Uint32 frameIndex) {
        MOBILEGL_ASSERT(frameIndex < m_frames.size(), "UniformDescriptorBinder::BeginFrame invalid frame index");
        auto& frame = m_frames[frameIndex];
        if (frame.peakAllocatedSetsThisFrame > m_peakDescriptorSetsObserved) {
            m_peakDescriptorSetsObserved = frame.peakAllocatedSetsThisFrame;
            MGLOG_D(
                "UniformDescriptorBinder: new descriptor set peak observed=%u (base setsPerFrame=%u, frame=%u, pools=%zu)",
                m_peakDescriptorSetsObserved, m_setsPerFrame, frameIndex, frame.descriptorPools.size());
        }
        frame.activeDescriptorPoolIndex = 0;
        frame.allocatedSetsThisFrame = 0;
        frame.peakAllocatedSetsThisFrame = 0;
        for (auto& bucket : frame.descriptorPools) {
            bucket.allocatedSets = 0;
            if (bucket.handle == VK_NULL_HANDLE) {
                continue;
            }
            VK_VERIFY(vkResetDescriptorPool(m_device, bucket.handle, 0),
                      "UniformDescriptorBinder::BeginFrame, vkResetDescriptorPool");
        }
    }

    Bool UniformDescriptorBinder::ResolveSamplerDescriptor(VkCommandBuffer commandBuffer,
                                                           const MG_State::GLState::ProgramObject& program,
                                                           const ProgramFactory::VkProgramLayout& layout,
                                                           Uint32 binding, VkDescriptorImageInfo& outImageInfo) const {
        (void)commandBuffer;
        MOBILEGL_ASSERT(m_textureManager != nullptr, "ResolveSamplerDescriptor: texture manager is null");
        MOBILEGL_ASSERT(m_samplerManager != nullptr, "ResolveSamplerDescriptor: sampler manager is null");
        SharedPtr<MG_State::GLState::ITextureObject> texture;
        if (!ResolveSamplerTexture(program, layout, binding, texture)) {
            return false;
        }

        const Int location = layout.samplerUniformLocationByBinding[binding];
        const Int unit = program.GetUniformSamplerOrImageUnitIndex(static_cast<Uint>(location));
        auto& textureUnit = MG_State::pGLContext->GetTextureUnitObject(unit);
        const auto samplerOverride = textureUnit.GetSamplerObject();
        if (!texture) {
            return false;
        }

        const MG_State::GLState::SamplerObject* samplerToUse =
            samplerOverride ? samplerOverride.get() : texture->GetSamplerObject().get();
        if (!samplerToUse) {
            return false;
        }
        VkTextureManager::TextureResource* resource = m_textureManager->SyncTextureAndGetDescriptor(*texture);
        if (resource == nullptr) {
            return false;
        }
        if (!IsValidSampledImageLayout(resource->layout)) {
            auto drawFbo = MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw).GetBoundObject();
            FramebufferAttachmentType attachmentType = FramebufferAttachmentType::None;
            Int attachmentLevel = 0;
            if (drawFbo &&
                FindFramebufferAttachmentForTexture(*drawFbo, *texture, attachmentType, attachmentLevel)) {
                MOBILEGL_ASSERT(false,
                                "ResolveSamplerDescriptor: framebuffer feedback loop detected: textureId=%d is bound "
                                "for sampling at binding=%u, but is also attached to drawFbo=%u as %s (level=%d, "
                                "trackedLayout=%d)",
                                texture->GetExternalIndex(), binding, drawFbo->GetExternalIndex(),
                                MG_Util::ConvertFramebufferAttachmentTypeToString(attachmentType).c_str(),
                                attachmentLevel, static_cast<Int>(resource->layout));
            }

            MOBILEGL_ASSERT(false,
                            "ResolveSamplerDescriptor: invalid sampled image layout=%d for textureId=%d, binding=%u",
                            static_cast<Int>(resource->layout), texture->GetExternalIndex(), binding);
            return false;
        }
        outImageInfo = {
            .sampler = m_samplerManager->GetOrCreateSampler(*samplerToUse),
            .imageView = resource->fullView,
            .imageLayout = resource->layout,
        };
        return outImageInfo.sampler != VK_NULL_HANDLE;
    }

    Bool UniformDescriptorBinder::ResolveSamplerDescriptorOverride(
        const SamplerBindingOverride& samplerBindingOverride, VkDescriptorImageInfo& outImageInfo) const {
        MOBILEGL_ASSERT(m_textureManager != nullptr, "ResolveSamplerDescriptorOverride: texture manager is null");
        MOBILEGL_ASSERT(m_samplerManager != nullptr, "ResolveSamplerDescriptorOverride: sampler manager is null");
        if (samplerBindingOverride.texture == nullptr || samplerBindingOverride.sampler == nullptr) {
            return false;
        }

        auto* resource = m_textureManager->SyncTextureAndGetDescriptor(*samplerBindingOverride.texture);
        if (resource == nullptr || !IsValidSampledImageLayout(resource->layout)) {
            return false;
        }

        outImageInfo = {
            .sampler = m_samplerManager->GetOrCreateSampler(*samplerBindingOverride.sampler),
            .imageView = resource->fullView,
            .imageLayout = resource->layout,
        };
        return outImageInfo.sampler != VK_NULL_HANDLE;
    }

    Bool UniformDescriptorBinder::ResolveSamplerTexture(const MG_State::GLState::ProgramObject& program,
                                                        const ProgramFactory::VkProgramLayout& layout, Uint32 binding,
                                                        SharedPtr<MG_State::GLState::ITextureObject>& outTexture) const {
        outTexture.reset();
        if (!MG_State::pGLContext || binding >= layout.samplerUniformLocationByBinding.size()) {
            return false;
        }

        const Int location = layout.samplerUniformLocationByBinding[binding];
        if (location < 0) {
            return false;
        }

        const Int unit = program.GetUniformSamplerOrImageUnitIndex(static_cast<Uint>(location));
        if (unit < 0) {
            return false;
        }

        auto& textureUnit = MG_State::pGLContext->GetTextureUnitObject(unit);
        const TextureTarget preferredTarget = layout.samplerTextureTargetByBinding[binding];
        outTexture = textureUnit.GetBindingSlot(preferredTarget).GetBoundObject();
        return outTexture != nullptr;
    }

    Bool UniformDescriptorBinder::CollectSampledTextures(const MG_State::GLState::ProgramObject& program,
                                                         Vector<MG_State::GLState::ITextureObject*>& outTextures) {
        outTextures.clear();
        MOBILEGL_ASSERT(m_programFactory != nullptr, "CollectSampledTextures: program factory is null");
        const auto* layout = m_programFactory->GetOrCreateProgramLayout(program);
        if (layout == nullptr) {
            return false;
        }

        const Uint32 bindingCount =
            std::min<Uint32>(m_maxBindings, static_cast<Uint32>(layout->bindingKinds.size()));
        for (Uint32 binding = 0; binding < bindingCount; ++binding) {
            if (layout->bindingKinds[binding] != ProgramFactory::DescriptorBindingKind::CombinedImageSampler) {
                continue;
            }

            SharedPtr<MG_State::GLState::ITextureObject> texture;
            if (!ResolveSamplerTexture(program, *layout, binding, texture) || !texture) {
                continue;
            }

            auto found = std::find(outTextures.begin(), outTextures.end(), texture.get());
            if (found == outTextures.end()) {
                outTextures.push_back(texture.get());
            }
        }
        return true;
    }

    Bool UniformDescriptorBinder::GatherBindingPayloads(const MG_State::GLState::ProgramObject& program,
                                                        Vector<const void*>& outData,
                                                        Vector<VkDeviceSize>& outSizes) const {
        outData.assign(m_maxBindings, nullptr);
        outSizes.assign(m_maxBindings, 0);

        const Uint32 activeUniformBlockCount = static_cast<Uint32>(program.GetActiveUniformBlocksCount());
        const Uint32 uniformBindingPointCount =
            static_cast<Uint32>(MG_State::pGLContext->GetBufferBindingPointCount(BufferTarget::Uniform));

        for (Uint32 blockIndex = 0; blockIndex < activeUniformBlockCount; ++blockIndex) {
            const Uint32 binding = program.GetUniformBlockBinding(blockIndex);
            if (binding >= m_maxBindings) {
                continue;
            }

            VkDeviceSize blockSize = static_cast<VkDeviceSize>(program.GetUBOSizeAt(blockIndex));
            if (blockSize == 0) {
                continue;
            }

            if (binding >= uniformBindingPointCount) {
                continue;
            }
            auto& bindingPoint = MG_State::pGLContext->GetBufferBindingPoint(BufferTarget::Uniform, binding);
            const auto bufferObject = bindingPoint.GetBoundObject();
            if (!bufferObject) {
                continue;
            }

            const auto bufferData = bufferObject->GetDataReadOnly();
            if (!bufferData || bufferData->empty()) {
                continue;
            }

            const auto range = bindingPoint.GetRange();
            const VkDeviceSize bufferSize = static_cast<VkDeviceSize>(bufferObject->GetSize());
            VkDeviceSize rangeStart = static_cast<VkDeviceSize>(range.start);
            VkDeviceSize rangeEnd = static_cast<VkDeviceSize>(range.end);

            if (rangeStart >= bufferSize) {
                continue;
            }
            if (rangeEnd <= rangeStart || rangeEnd > bufferSize) {
                rangeEnd = bufferSize;
            }

            VkDeviceSize available = rangeEnd - rangeStart;
            if (available == 0) {
                continue;
            }

            outData[binding] = bufferData->data() + static_cast<SizeT>(rangeStart);
            outSizes[binding] = std::min(blockSize, available);
        }

        return true;
    }

    Bool UniformDescriptorBinder::CreateDescriptorPool(Uint32 maxSets, VkDescriptorPool& outPool) const {
        outPool = VK_NULL_HANDLE;
        if (m_device == VK_NULL_HANDLE || maxSets == 0 || m_maxBindings == 0) {
            return false;
        }

        const Uint64 descriptorCount64 = static_cast<Uint64>(maxSets) * static_cast<Uint64>(m_maxBindings);
        if (descriptorCount64 > static_cast<Uint64>(std::numeric_limits<Uint32>::max())) {
            MGLOG_E("UniformDescriptorBinder::CreateDescriptorPool failed: descriptorCount overflow");
            return false;
        }

        const Uint32 descriptorCount = static_cast<Uint32>(descriptorCount64);
        VkDescriptorPoolSize poolSizes[2]{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        poolSizes[0].descriptorCount = descriptorCount;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = descriptorCount;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = maxSets;
        poolInfo.poolSizeCount = static_cast<Uint32>(std::size(poolSizes));
        poolInfo.pPoolSizes = poolSizes;

        const VkResult result = vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &outPool);
        if (result != VK_SUCCESS) {
            MGLOG_E("UniformDescriptorBinder::CreateDescriptorPool failed: vkCreateDescriptorPool returned %d",
                    result);
            return false;
        }
        return true;
    }

    Bool UniformDescriptorBinder::GrowFrameDescriptorPool(FrameResources& frame, Uint32 frameIndex) {
        if (frame.descriptorPools.empty()) {
            return false;
        }

        const auto& currentBucket = frame.descriptorPools[frame.activeDescriptorPoolIndex];
        const Uint32 currentMaxSets = std::max<Uint32>(1, currentBucket.maxSets);
        const Uint32 grownMaxSets = currentMaxSets <= (std::numeric_limits<Uint32>::max() / 2) ? (currentMaxSets * 2)
                                                                                                 : currentMaxSets;

        VkDescriptorPool grownPool = VK_NULL_HANDLE;
        if (!CreateDescriptorPool(grownMaxSets, grownPool)) {
            MGLOG_E("UniformDescriptorBinder::GrowFrameDescriptorPool failed: cannot create grown pool (%u -> %u sets)",
                    currentMaxSets, grownMaxSets);
            return false;
        }

        frame.descriptorPools.push_back({grownPool, grownMaxSets, 0});
        frame.activeDescriptorPoolIndex = static_cast<Uint32>(frame.descriptorPools.size() - 1);
        MGLOG_D(
            "UniformDescriptorBinder: frame %u descriptor pool exhausted, grew pool (%u -> %u sets), poolCount=%zu",
            frameIndex, currentMaxSets, grownMaxSets, frame.descriptorPools.size());
        return true;
    }

    Bool UniformDescriptorBinder::BindProgramUniformBuffers(VkCommandBuffer commandBuffer,
                                                            const MG_State::GLState::ProgramObject& program,
                                                            Uint32 frameIndex) {
        return BindProgramUniformBuffers(commandBuffer, program, frameIndex, nullptr);
    }

    Bool UniformDescriptorBinder::BindProgramUniformBuffers(VkCommandBuffer commandBuffer,
                                                            const MG_State::GLState::ProgramObject& program,
                                                            Uint32 frameIndex,
                                                            const SamplerBindingOverride* samplerBindingOverride) {
        MOBILEGL_ASSERT(m_programFactory != nullptr, "BindProgramUniformBuffers: program factory is null");
        const auto* layout = m_programFactory->GetOrCreateProgramLayout(program);
        MOBILEGL_ASSERT(layout != nullptr,
                        "UniformDescriptorBinder::BindProgramUniformBuffers: program layout is null");

        auto& frame = m_frames[frameIndex];
        if (frame.descriptorPools.empty()) {
            MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: frame descriptor pools are invalid");
            return false;
        }
        if (frame.activeDescriptorPoolIndex >= frame.descriptorPools.size()) {
            frame.activeDescriptorPoolIndex = 0;
        }

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout->descriptorSetLayout;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

        auto allocateFromActivePool = [&](VkResult& outResult) {
            auto& bucket = frame.descriptorPools[frame.activeDescriptorPoolIndex];
            allocInfo.descriptorPool = bucket.handle;
            outResult = vkAllocateDescriptorSets(m_device, &allocInfo, &descriptorSet);
            if (outResult == VK_SUCCESS) {
                ++bucket.allocatedSets;
                ++frame.allocatedSetsThisFrame;
                frame.peakAllocatedSetsThisFrame =
                    std::max(frame.peakAllocatedSetsThisFrame, frame.allocatedSetsThisFrame);
            }
        };

        VkResult allocResult = VK_SUCCESS;
        allocateFromActivePool(allocResult);
        if (allocResult == VK_ERROR_OUT_OF_POOL_MEMORY || allocResult == VK_ERROR_FRAGMENTED_POOL) {
            if (!GrowFrameDescriptorPool(frame, frameIndex)) {
                MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: descriptor pool growth failed");
                return false;
            }
            allocateFromActivePool(allocResult);
        }
        if (allocResult != VK_SUCCESS || descriptorSet == VK_NULL_HANDLE) {
            MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: vkAllocateDescriptorSets returned %d",
                    allocResult);
            return false;
        }

        Vector<const void*> bindingData;
        Vector<VkDeviceSize> bindingSizes;
        if (!GatherBindingPayloads(program, bindingData, bindingSizes)) {
            MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: cannot gather UBO payloads");
            return false;
        }

        static const Uint8 kFallbackData[16] = {};
        MOBILEGL_ASSERT(m_textureManager != nullptr, "BindProgramUniformBuffers: texture manager is null");
        MOBILEGL_ASSERT(m_samplerManager != nullptr, "BindProgramUniformBuffers: sampler manager is null");
        MOBILEGL_ASSERT(m_bufferManager != nullptr, "BindProgramUniformBuffers: buffer manager is null");

        Vector<VkWriteDescriptorSet> writes;
        writes.reserve(m_maxBindings);
        Vector<VkDescriptorBufferInfo> bufferInfos;
        Vector<VkDescriptorImageInfo> imageInfos;
        Vector<Uint32> dynamicOffsets;
        bufferInfos.reserve(m_maxBindings);
        imageInfos.reserve(m_maxBindings);
        dynamicOffsets.reserve(layout->dynamicBindings.size());

        const Uint32 bindingCount =
            std::min<Uint32>(m_maxBindings, static_cast<Uint32>(layout->bindingKinds.size()));
        for (Uint32 binding = 0; binding < bindingCount; ++binding) {
            const auto kind = layout->bindingKinds[binding];
            if (kind == ProgramFactory::DescriptorBindingKind::None) {
                continue;
            }

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptorSet;
            write.dstBinding = binding;
            write.dstArrayElement = 0;
            write.descriptorCount = 1;

            if (kind == ProgramFactory::DescriptorBindingKind::UniformBufferDynamic) {
                const void* payload = bindingData[binding];
                VkDeviceSize payloadSize = bindingSizes[binding];
                if (payload == nullptr || payloadSize == 0) {
                    if (layout->globalUboBinding == static_cast<Int>(binding)) {
                        const void* globalUboData = program.GetUBOData();
                        const VkDeviceSize globalUboSize = static_cast<VkDeviceSize>(program.GetUBOSize());
                        if (globalUboData != nullptr && globalUboSize > 0) {
                            payload = globalUboData;
                            payloadSize = globalUboSize;
                        }
                    }
                    if (payload == nullptr || payloadSize == 0) {
                        payload = kFallbackData;
                        payloadSize = sizeof(kFallbackData);
                    }
                }

                BufferSlice slice{};
                if (!m_bufferManager->UploadTransient(BufferKind::Uniform, frameIndex, payload, payloadSize,
                                                     m_minDynamicOffsetAlignment, slice)) {
                    MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: UBO upload failed on binding %u",
                            binding);
                    return false;
                }

                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = slice.buffer;
                bufferInfo.offset = 0;
                bufferInfo.range = payloadSize;
                bufferInfos.push_back(bufferInfo);

                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                write.pBufferInfo = &bufferInfos.back();
                writes.push_back(write);
                dynamicOffsets.push_back(static_cast<Uint32>(slice.offset));
            } else {
                VkDescriptorImageInfo imageInfo{};
                Bool hasImage = false;
                if (samplerBindingOverride != nullptr &&
                    samplerBindingOverride->binding == binding &&
                    samplerBindingOverride->texture != nullptr &&
                    samplerBindingOverride->sampler != nullptr) {
                    hasImage = ResolveSamplerDescriptorOverride(*samplerBindingOverride, imageInfo);
                } else {
                    hasImage = ResolveSamplerDescriptor(commandBuffer, program, *layout, binding, imageInfo);
                }
                if (!hasImage) {
                    MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: sampler binding %u has no valid texture descriptor",
                            binding);
                    return false;
                }
                if (imageInfo.sampler == VK_NULL_HANDLE || imageInfo.imageView == VK_NULL_HANDLE) {
                    MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: sampler binding %u has null sampler or imageView",
                            binding);
                    return false;
                }
                imageInfos.push_back(imageInfo);
                write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write.pImageInfo = &imageInfos.back();
                writes.push_back(write);
            }
        }

        if (!writes.empty()) {
            vkUpdateDescriptorSets(m_device, static_cast<Uint32>(writes.size()), writes.data(), 0, nullptr);
        }

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout->pipelineLayout, 0, 1,
                                &descriptorSet, static_cast<Uint32>(dynamicOffsets.size()), dynamicOffsets.data());
        return true;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
