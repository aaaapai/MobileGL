// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/UniformDescriptorBinder.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "UniformManager.h"

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

    static Int ResolveSamplerUnitIndex(const MG_State::GLState::ProgramObject& program, Int location, Uint32 binding) {
        MOBILEGL_ASSERT(location >= -1, "ResolveSamplerUnitIndex: invalid sampler location for binding %u", binding);
        if (location < 0) {
            return 0;
        }
        const Int uniformUnit = program.GetUniformSamplerOrImageUnitIndex(static_cast<Uint>(location));
        MOBILEGL_ASSERT(uniformUnit >= -1,
                        "ResolveSamplerUnitIndex: invalid texture unit for binding %u location %d (unit=%d)", binding,
                        location, uniformUnit);
        return uniformUnit >= 0 ? uniformUnit : 0;
    }

    Bool UniformManager::Initialize(VkDevice device, VkBufferManager* bufferManager,
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

    void UniformManager::Shutdown() {
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

    void UniformManager::BeginFrame(Uint32 frameIndex) {
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

    Bool UniformManager::ResolveSamplerDescriptor(VkCommandBuffer commandBuffer,
                                                            const MG_State::GLState::ProgramObject& program,
                                                            const ProgramFactory::VkProgramObject& programObj,
                                                            Uint32 binding, VkDescriptorImageInfo& outImageInfo) const {
        (void)commandBuffer;
        MOBILEGL_ASSERT(m_textureManager != nullptr, "ResolveSamplerDescriptor: texture manager is null");
        MOBILEGL_ASSERT(m_samplerManager != nullptr, "ResolveSamplerDescriptor: sampler manager is null");
        MOBILEGL_ASSERT(binding < programObj.samplerNameByBinding.size(),
                        "ResolveSamplerDescriptor: sampler binding %u name lookup out of range", binding);
        SharedPtr<MG_State::GLState::ITextureObject> texture;
        const Bool resolvedTexture = ResolveSamplerTexture(program, programObj, binding, texture);
        if (!resolvedTexture) {
            MGLOG_E("ResolveSamplerDescriptor: failed to resolve sampler texture for binding %u ('%s')", binding,
                programObj.samplerNameByBinding[binding].c_str());
            return false;
        }

        const Int location = programObj.samplerUniformLocationByBinding[binding];
        const Int unit = ResolveSamplerUnitIndex(program, location, binding);
        auto& textureUnit = MG_State::pGLContext->GetTextureUnitObject(unit);
        const auto samplerOverride = textureUnit.GetSamplerObject();
        if (texture == nullptr) {
            MGLOG_E(
                "ResolveSamplerDescriptor: sampler binding %u ('%s') resolved null texture (location=%d unit=%d target=%d)",
                binding, programObj.samplerNameByBinding[binding].c_str(), location, unit,
                static_cast<Int>(programObj.samplerTextureTargetByBinding[binding]));
            return false;
        }

        const MG_State::GLState::SamplerObject* samplerToUse =
            samplerOverride ? samplerOverride.get() : texture->GetSamplerObject().get();
        if (samplerToUse == nullptr) {
            MGLOG_E(
                "ResolveSamplerDescriptor: sampler binding %u ('%s') has no sampler object (textureId=%d location=%d unit=%d)",
                binding, programObj.samplerNameByBinding[binding].c_str(), texture->GetExternalIndex(), location,
                unit);
            return false;
        }
        VkTextureManager::TextureResource* resource = m_textureManager->SyncTextureAndGetDescriptor(*texture);
        if (resource == nullptr) {
            MGLOG_E(
                "ResolveSamplerDescriptor: sampler binding %u ('%s') failed to create/sync texture resource (textureId=%d target=%d location=%d unit=%d)",
                binding, programObj.samplerNameByBinding[binding].c_str(), texture->GetExternalIndex(),
                static_cast<Int>(texture->GetTarget()), location, unit);
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
            .sampler = m_samplerManager->GetOrCreateSampler(*samplerToUse, *texture),
            .imageView = resource->fullView,
            .imageLayout = resource->layout,
        };
        return outImageInfo.sampler != VK_NULL_HANDLE;
    }

    Bool UniformManager::ResolveSamplerDescriptorOverride(
        const SamplerBindingOverride& samplerBindingOverride, VkDescriptorImageInfo& outImageInfo) const {
        MOBILEGL_ASSERT(m_textureManager != nullptr, "ResolveSamplerDescriptorOverride: texture manager is null");
        MOBILEGL_ASSERT(m_samplerManager != nullptr, "ResolveSamplerDescriptorOverride: sampler manager is null");
        MOBILEGL_ASSERT(samplerBindingOverride.texture != nullptr,
                        "ResolveSamplerDescriptorOverride: override texture is null for binding %u",
                        samplerBindingOverride.binding);
        MOBILEGL_ASSERT(samplerBindingOverride.sampler != nullptr,
                        "ResolveSamplerDescriptorOverride: override sampler is null for binding %u",
                        samplerBindingOverride.binding);

        auto* resource = m_textureManager->SyncTextureAndGetDescriptor(*samplerBindingOverride.texture);
        MOBILEGL_ASSERT(resource != nullptr,
                        "ResolveSamplerDescriptorOverride: failed to sync override texture resource for binding %u textureId=%d",
                        samplerBindingOverride.binding, samplerBindingOverride.texture->GetExternalIndex());
        MOBILEGL_ASSERT(IsValidSampledImageLayout(resource->layout),
                        "ResolveSamplerDescriptorOverride: invalid layout %d for binding %u textureId=%d",
                        static_cast<Int>(resource->layout), samplerBindingOverride.binding,
                        samplerBindingOverride.texture->GetExternalIndex());

        outImageInfo = {
            .sampler = m_samplerManager->GetOrCreateSampler(*samplerBindingOverride.sampler,
                                                            *samplerBindingOverride.texture),
            .imageView = samplerBindingOverride.imageView != VK_NULL_HANDLE ?
                samplerBindingOverride.imageView :
                resource->fullView,
            .imageLayout = resource->layout,
        };
        return outImageInfo.sampler != VK_NULL_HANDLE;
    }

    Bool UniformManager::ResolveSamplerTexture(const MG_State::GLState::ProgramObject& program,
                                                         const ProgramFactory::VkProgramObject& programObj, Uint32 binding,
                                                         SharedPtr<MG_State::GLState::ITextureObject>& outTexture) const {
        outTexture.reset();
        MOBILEGL_ASSERT(MG_State::pGLContext != nullptr, "ResolveSamplerTexture: GL context is null");
        MOBILEGL_ASSERT(binding < programObj.samplerUniformLocationByBinding.size(),
                        "ResolveSamplerTexture: sampler location binding %u out of range", binding);
        MOBILEGL_ASSERT(binding < programObj.samplerTextureTargetByBinding.size(),
                        "ResolveSamplerTexture: sampler target binding %u out of range", binding);

        const Int location = programObj.samplerUniformLocationByBinding[binding];
        const Int unit = ResolveSamplerUnitIndex(program, location, binding);

        auto& textureUnit = MG_State::pGLContext->GetTextureUnitObject(unit);
        const TextureTarget preferredTarget = programObj.samplerTextureTargetByBinding[binding];
        outTexture = textureUnit.GetBindingSlot(preferredTarget).GetBoundObject();

        if (!outTexture) {
            MGLOG_E("ResolveSamplerTexture: no texture bound for sampler binding=%u location=%d unit=%d target=%d",
                    binding, location, unit, static_cast<Int>(preferredTarget));
            return false;
        }
        return outTexture != nullptr;
    }

    Bool UniformManager::CollectSampledTextures(const MG_State::GLState::ProgramObject& program,
                                                          const ProgramFactory::VkProgramObject& programObj,
                                                          Vector<MG_State::GLState::ITextureObject*>& outTextures) {
        outTextures.clear();

        const Uint32 bindingCount =
            std::min<Uint32>(m_maxBindings, static_cast<Uint32>(programObj.bindingKinds.size()));
        for (Uint32 binding = 0; binding < bindingCount; ++binding) {
            if (programObj.bindingKinds[binding] != ProgramFactory::DescriptorBindingKind::CombinedImageSampler) {
                continue;
            }

            SharedPtr<MG_State::GLState::ITextureObject> texture;
            if (!ResolveSamplerTexture(program, programObj, binding, texture) || !texture) {
                continue;
            }

            auto found = std::find(outTextures.begin(), outTextures.end(), texture.get());
            if (found == outTextures.end()) {
                outTextures.push_back(texture.get());
            }
        }
        return true;
    }

    Bool UniformManager::ResolveUniformBufferPayload(const MG_State::GLState::ProgramObject& program,
                                                     const ProgramFactory::VkProgramObject& programObj, Uint32 binding,
                                                     const void*& outData, VkDeviceSize& outSize) const {
        outData = nullptr;
        outSize = 0;

        MOBILEGL_ASSERT(MG_State::pGLContext != nullptr, "ResolveUniformBufferPayload: GL context is null");
        MOBILEGL_ASSERT(binding < programObj.bindingKinds.size(),
                        "ResolveUniformBufferPayload: binding %u out of range", binding);
        MOBILEGL_ASSERT(programObj.bindingKinds[binding] == ProgramFactory::DescriptorBindingKind::UniformBufferDynamic,
                        "ResolveUniformBufferPayload: binding %u is not a uniform buffer descriptor", binding);

        if (programObj.globalUboBinding == static_cast<Int>(binding)) {
            outData = program.GetUBOData();
            outSize = static_cast<VkDeviceSize>(program.GetUBOSize());
            MOBILEGL_ASSERT(outData != nullptr, "ResolveUniformBufferPayload: global UBO data is null");
            MOBILEGL_ASSERT(outSize > 0, "ResolveUniformBufferPayload: global UBO size is zero");
            return outData != nullptr && outSize > 0;
        }

        MOBILEGL_ASSERT(binding < programObj.uniformBlockIndexByBinding.size(),
                        "ResolveUniformBufferPayload: UBO mapping binding %u out of range", binding);
        const Int blockIndex = programObj.uniformBlockIndexByBinding[binding];
        MOBILEGL_ASSERT(blockIndex >= 0,
                        "ResolveUniformBufferPayload: no uniform block mapped to descriptor binding %u", binding);

        const Uint32 activeUniformBlockCount = static_cast<Uint32>(program.GetActiveUniformBlocksCount());
        MOBILEGL_ASSERT(static_cast<Uint32>(blockIndex) < activeUniformBlockCount,
                        "ResolveUniformBufferPayload: uniform block index %d out of range (count=%u)", blockIndex,
                        activeUniformBlockCount);

        const Uint32 frontendBinding = program.GetUniformBlockBinding(static_cast<Uint32>(blockIndex));
        const Uint32 uniformBindingPointCount =
            static_cast<Uint32>(MG_State::pGLContext->GetBufferBindingPointCount(BufferTarget::Uniform));
        MOBILEGL_ASSERT(frontendBinding < uniformBindingPointCount,
                        "ResolveUniformBufferPayload: frontend UBO binding %u out of range for block '%s'",
                        frontendBinding, program.GetUniformBlockName(static_cast<Uint32>(blockIndex)).c_str());

        auto& bindingPoint = MG_State::pGLContext->GetBufferBindingPoint(BufferTarget::Uniform, frontendBinding);
        const auto bufferObject = bindingPoint.GetBoundObject();
        MOBILEGL_ASSERT(bufferObject != nullptr,
                        "ResolveUniformBufferPayload: no UBO bound at frontend binding %u for block '%s'",
                        frontendBinding, program.GetUniformBlockName(static_cast<Uint32>(blockIndex)).c_str());

        const auto bufferData = bufferObject->GetDataReadOnly();
        MOBILEGL_ASSERT(bufferData != nullptr && !bufferData->empty(),
                        "ResolveUniformBufferPayload: bound UBO data is empty for block '%s'",
                        program.GetUniformBlockName(static_cast<Uint32>(blockIndex)).c_str());

        const auto range = bindingPoint.GetRange();
        const VkDeviceSize bufferSize = static_cast<VkDeviceSize>(bufferObject->GetSize());
        const VkDeviceSize rangeStart = static_cast<VkDeviceSize>(range.start);
        MOBILEGL_ASSERT(rangeStart < bufferSize,
                        "ResolveUniformBufferPayload: UBO range start %zu exceeds buffer size %zu for block '%s'",
                        static_cast<SizeT>(rangeStart), static_cast<SizeT>(bufferSize),
                        program.GetUniformBlockName(static_cast<Uint32>(blockIndex)).c_str());

        VkDeviceSize rangeEnd = static_cast<VkDeviceSize>(range.end);
        if (rangeEnd > bufferSize) {
            rangeEnd = bufferSize;
        }
        MOBILEGL_ASSERT(rangeEnd > rangeStart,
                        "ResolveUniformBufferPayload: invalid UBO range [%zu, %zu) for block '%s'",
                        static_cast<SizeT>(rangeStart), static_cast<SizeT>(rangeEnd),
                        program.GetUniformBlockName(static_cast<Uint32>(blockIndex)).c_str());

        const VkDeviceSize blockSize = static_cast<VkDeviceSize>(program.GetUBOSizeAt(static_cast<Uint32>(blockIndex)));
        MOBILEGL_ASSERT(blockSize > 0,
                        "ResolveUniformBufferPayload: reflected UBO size is zero for block '%s'",
                        program.GetUniformBlockName(static_cast<Uint32>(blockIndex)).c_str());

        const VkDeviceSize available = rangeEnd - rangeStart;
        MOBILEGL_ASSERT(available >= blockSize,
                        "ResolveUniformBufferPayload: bound range %zu is smaller than UBO size %zu for block '%s'",
                        static_cast<SizeT>(available), static_cast<SizeT>(blockSize),
                        program.GetUniformBlockName(static_cast<Uint32>(blockIndex)).c_str());

        outData = bufferData->data() + static_cast<SizeT>(rangeStart);
        outSize = blockSize;
        return true;
    }

    Bool UniformManager::CreateDescriptorPool(Uint32 maxSets, VkDescriptorPool& outPool) const {
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

    Bool UniformManager::GrowFrameDescriptorPool(FrameResources& frame, Uint32 frameIndex) {
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

    VkResult UniformManager::AllocateDescriptorSetsFromActivePool(Uint32 frameIndex, const ProgramFactory::VkProgramObject& programObj, VkDescriptorSet& outDescriptorSet) {
        auto& frame = m_frames[frameIndex];
        auto& bucket = frame.descriptorPools[frame.activeDescriptorPoolIndex];
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &programObj.descriptorSetLayout;

        allocInfo.descriptorPool = bucket.handle;
        VkResult result = vkAllocateDescriptorSets(m_device, &allocInfo, &outDescriptorSet);
        if (result == VK_SUCCESS) {
            ++bucket.allocatedSets;
            ++frame.allocatedSetsThisFrame;
            frame.peakAllocatedSetsThisFrame =
                std::max(frame.peakAllocatedSetsThisFrame, frame.allocatedSetsThisFrame);
        }
        return result;
    }

    Bool UniformManager::BindProgramUniformBuffers(VkCommandBuffer commandBuffer,
                                                             const MG_State::GLState::ProgramObject& program,
                                                             const ProgramFactory::VkProgramObject& programObj,
                                                             Uint32 frameIndex,
                                                             const SamplerBindingOverride* samplerBindingOverride) {
        auto& frame = m_frames[frameIndex];
        if (frame.descriptorPools.empty()) {
            MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: frame descriptor pools are invalid");
            return false;
        }
        if (frame.activeDescriptorPoolIndex >= frame.descriptorPools.size()) {
            frame.activeDescriptorPoolIndex = 0;
        }

        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        VkResult allocResult = AllocateDescriptorSetsFromActivePool(frameIndex, programObj, descriptorSet);
        if (allocResult == VK_ERROR_OUT_OF_POOL_MEMORY || allocResult == VK_ERROR_FRAGMENTED_POOL) {
            if (!GrowFrameDescriptorPool(frame, frameIndex)) {
                MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: descriptor pool growth failed");
                return false;
            }
            allocResult = AllocateDescriptorSetsFromActivePool(frameIndex, programObj, descriptorSet);
        }
        if (allocResult != VK_SUCCESS || descriptorSet == VK_NULL_HANDLE) {
            MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: vkAllocateDescriptorSets returned %d",
                    allocResult);
            return false;
        }

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
        dynamicOffsets.reserve(programObj.dynamicBindings.size());

        const Uint32 bindingCount =
            std::min<Uint32>(m_maxBindings, static_cast<Uint32>(programObj.bindingKinds.size()));
        for (Uint32 binding = 0; binding < bindingCount; ++binding) {
            const auto kind = programObj.bindingKinds[binding];
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
                const void* payload = nullptr;
                VkDeviceSize payloadSize = 0;
                const Bool hasPayload = ResolveUniformBufferPayload(program, programObj, binding, payload, payloadSize);
                MOBILEGL_ASSERT(hasPayload && payload != nullptr && payloadSize > 0,
                                "UniformDescriptorBinder::BindProgramUniformBuffers failed: missing UBO payload on binding %u",
                                binding);

                BufferSlice slice{};
                if (!m_bufferManager->UploadTransient(BufferKind::Uniform, frameIndex, payload, payloadSize,
                                                     m_minDynamicOffsetAlignment, slice)) {
                    MOBILEGL_ASSERT(false, "UniformDescriptorBinder::BindProgramUniformBuffers failed: UBO upload failed on binding %u",
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
                    hasImage = ResolveSamplerDescriptor(commandBuffer, program, programObj, binding, imageInfo);
                }
                if (!hasImage) {
                    MGLOG_E(
                        "UniformDescriptorBinder::BindProgramUniformBuffers failed: sampler binding %u has no valid texture descriptor",
                        binding);
                    return false;
                }
                if (imageInfo.sampler == VK_NULL_HANDLE || imageInfo.imageView == VK_NULL_HANDLE) {
                    MGLOG_E(
                        "UniformDescriptorBinder::BindProgramUniformBuffers failed: sampler binding %u has null sampler or imageView",
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

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, programObj.pipelineLayout, 0, 1,
                                &descriptorSet, static_cast<Uint32>(dynamicOffsets.size()), dynamicOffsets.data());
        return true;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
