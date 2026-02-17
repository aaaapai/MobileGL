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
#include "MG_Util/ShaderTranspiler/Types.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    VkDeviceSize UniformDescriptorBinder::AlignUp(VkDeviceSize value, VkDeviceSize alignment) {
        if (alignment == 0) {
            return value;
        }
        return (value + alignment - 1) / alignment * alignment;
    }

    Bool UniformDescriptorBinder::Initialize(VkDevice device, VmaAllocator allocator,
                                             VkDeviceSize minUniformBufferOffsetAlignment, Uint32 frameCount,
                                             Uint32 maxBindings, Uint32 setsPerFrame, VkDeviceSize perFrameUploadBytes) {
        Shutdown();

        MOBILEGL_ASSERT(device != VK_NULL_HANDLE, "UniformDescriptorBinder::Initialize requires valid VkDevice");
        MOBILEGL_ASSERT(allocator != nullptr, "UniformDescriptorBinder::Initialize requires valid VMA allocator");
        MOBILEGL_ASSERT(frameCount > 0, "UniformDescriptorBinder::Initialize requires frameCount > 0");
        MOBILEGL_ASSERT(maxBindings > 0, "UniformDescriptorBinder::Initialize requires maxBindings > 0");
        MOBILEGL_ASSERT(setsPerFrame > 0, "UniformDescriptorBinder::Initialize requires setsPerFrame > 0");

        m_device = device;
        m_allocator = allocator;
        m_minDynamicOffsetAlignment = std::max<VkDeviceSize>(1, minUniformBufferOffsetAlignment);
        m_perFrameUploadBytes = perFrameUploadBytes;
        m_frameCount = frameCount;
        m_maxBindings = maxBindings;
        m_setsPerFrame = setsPerFrame;

        Vector<VkDescriptorSetLayoutBinding> bindings;
        bindings.reserve(m_maxBindings);
        for (Uint32 bindingIndex = 0; bindingIndex < m_maxBindings; ++bindingIndex) {
            VkDescriptorSetLayoutBinding binding{};
            binding.binding = bindingIndex;
            binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            binding.descriptorCount = 1;
            binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
            binding.pImmutableSamplers = nullptr;
            bindings.push_back(binding);
        }

        VkDescriptorSetLayoutCreateInfo setLayoutInfo{};
        setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        setLayoutInfo.bindingCount = static_cast<Uint32>(bindings.size());
        setLayoutInfo.pBindings = bindings.data();
        VK_VERIFY(vkCreateDescriptorSetLayout(m_device, &setLayoutInfo, nullptr, &m_descriptorSetLayout),
                  "UniformDescriptorBinder::Initialize, vkCreateDescriptorSetLayout");

        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        poolSize.descriptorCount = m_frameCount * m_setsPerFrame * m_maxBindings;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = m_frameCount * m_setsPerFrame;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        VK_VERIFY(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool),
                  "UniformDescriptorBinder::Initialize, vkCreateDescriptorPool");

        m_descriptorSets.resize(m_frameCount * m_setsPerFrame, VK_NULL_HANDLE);
        Vector<VkDescriptorSetLayout> layouts(m_descriptorSets.size(), m_descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<Uint32>(layouts.size());
        allocInfo.pSetLayouts = layouts.data();
        VK_VERIFY(vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()),
                  "UniformDescriptorBinder::Initialize, vkAllocateDescriptorSets");

        m_frames.resize(m_frameCount);
        for (Uint32 frameIndex = 0; frameIndex < m_frameCount; ++frameIndex) {
            auto& frame = m_frames[frameIndex];
            frame.writeCursor = 0;
            frame.descriptorCursor = 0;
            const Bool created = frame.uploadBuffer.Create(
                m_allocator, m_perFrameUploadBytes, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO,
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
            if (!created) {
                MGLOG_E("UniformDescriptorBinder::Initialize failed: cannot create frame upload buffer %u", frameIndex);
                Shutdown();
                return false;
            }
        }

        return true;
    }

    void UniformDescriptorBinder::Shutdown() {
        for (auto& frame : m_frames) {
            frame.uploadBuffer.Destroy();
            frame.writeCursor = 0;
            frame.descriptorCursor = 0;
        }
        m_frames.clear();
        m_descriptorSets.clear();

        if (m_device != VK_NULL_HANDLE && m_descriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
        }
        m_descriptorPool = VK_NULL_HANDLE;

        if (m_device != VK_NULL_HANDLE && m_descriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
        }
        m_descriptorSetLayout = VK_NULL_HANDLE;

        m_allocator = nullptr;
        m_device = VK_NULL_HANDLE;
        m_minDynamicOffsetAlignment = 1;
        m_perFrameUploadBytes = 0;
        m_frameCount = 0;
        m_maxBindings = 0;
        m_setsPerFrame = 0;
    }

    void UniformDescriptorBinder::BeginFrame(Uint32 frameIndex) {
        MOBILEGL_ASSERT(frameIndex < m_frames.size(), "UniformDescriptorBinder::BeginFrame invalid frame index");
        auto& frame = m_frames[frameIndex];
        frame.writeCursor = 0;
        frame.descriptorCursor = 0;
    }

    Bool UniformDescriptorBinder::AllocateUploadRegion(FrameResources& frame, VkDeviceSize size, VkDeviceSize& outOffset) {
        const VkDeviceSize alignedOffset = AlignUp(frame.writeCursor, m_minDynamicOffsetAlignment);
        if (alignedOffset + size > m_perFrameUploadBytes) {
            return false;
        }
        outOffset = alignedOffset;
        frame.writeCursor = alignedOffset + size;
        return true;
    }

    Bool UniformDescriptorBinder::GatherBindingPayloads(const MG_State::GLState::ProgramObject& program,
                                                        Vector<const void*>& outData,
                                                        Vector<VkDeviceSize>& outSizes) const {
        outData.assign(m_maxBindings, nullptr);
        outSizes.assign(m_maxBindings, 0);

        if (MG_State::pGLContext == nullptr) {
            return false;
        }

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

            const auto& blockName = program.GetUniformBlockName(blockIndex);
            if (blockName == MG_Util::ShaderTranspiler::GLOBAL_UBO_NAME) {
                const void* globalUboData = program.GetUBOData();
                const VkDeviceSize globalUboSize = static_cast<VkDeviceSize>(program.GetUBOSize());
                if (globalUboData == nullptr || globalUboSize == 0) {
                    continue;
                }
                outData[binding] = globalUboData;
                outSizes[binding] = std::min(blockSize, globalUboSize);
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

    Bool UniformDescriptorBinder::BindProgramUniformBuffers(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
                                                            const MG_State::GLState::ProgramObject& program,
                                                            Uint32 frameIndex) {
        if (m_descriptorSetLayout == VK_NULL_HANDLE || m_descriptorPool == VK_NULL_HANDLE) {
            MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: binder is not initialized");
            return false;
        }
        if (frameIndex >= m_frames.size()) {
            MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: invalid frame index %u", frameIndex);
            return false;
        }

        auto& frame = m_frames[frameIndex];
        if (frame.descriptorCursor >= m_setsPerFrame) {
            MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: per-frame descriptor set budget exceeded");
            return false;
        }

        const Uint32 descriptorSetIndex = frameIndex * m_setsPerFrame + frame.descriptorCursor;
        const VkDescriptorSet descriptorSet = m_descriptorSets[descriptorSetIndex];
        ++frame.descriptorCursor;

        Vector<const void*> bindingData;
        Vector<VkDeviceSize> bindingSizes;
        if (!GatherBindingPayloads(program, bindingData, bindingSizes)) {
            MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: cannot gather UBO payloads");
            return false;
        }

        static const Uint8 kFallbackData[16] = {};

        Vector<VkDescriptorBufferInfo> bufferInfos(m_maxBindings);
        Vector<VkWriteDescriptorSet> writes;
        writes.reserve(m_maxBindings);
        Vector<Uint32> dynamicOffsets(m_maxBindings, 0);

        for (Uint32 binding = 0; binding < m_maxBindings; ++binding) {
            const void* payload = bindingData[binding];
            VkDeviceSize payloadSize = bindingSizes[binding];
            if (payload == nullptr || payloadSize == 0) {
                payload = kFallbackData;
                payloadSize = sizeof(kFallbackData);
            }

            VkDeviceSize payloadOffset = 0;
            if (!AllocateUploadRegion(frame, payloadSize, payloadOffset)) {
                MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: frame upload buffer exhausted");
                return false;
            }
            if (!frame.uploadBuffer.Upload(payload, payloadSize, payloadOffset)) {
                MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: upload failed on binding %u", binding);
                return false;
            }

            bufferInfos[binding].buffer = frame.uploadBuffer.GetHandle();
            bufferInfos[binding].offset = 0;
            bufferInfos[binding].range = payloadSize;
            dynamicOffsets[binding] = static_cast<Uint32>(payloadOffset);

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptorSet;
            write.dstBinding = binding;
            write.dstArrayElement = 0;
            write.descriptorCount = 1;
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            write.pBufferInfo = &bufferInfos[binding];
            writes.push_back(write);
        }

        vkUpdateDescriptorSets(m_device, static_cast<Uint32>(writes.size()), writes.data(), 0, nullptr);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet,
                                static_cast<Uint32>(dynamicOffsets.size()), dynamicOffsets.data());
        return true;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan

