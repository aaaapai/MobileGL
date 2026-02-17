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

    Uint64 UniformDescriptorBinder::ComputeProgramHash(const MG_State::GLState::ProgramObject& program) {
        XXH64_state_t* state = XXH64_createState();
        XXHASH_VERIFY(XXH64_reset(state, 0xC0D3A11ULL));
        const auto& spirv = program.GetGeneratedSpirv();
        for (const auto& module : spirv) {
            XXHASH_VERIFY(XXH64_update(state, module.data(), module.size() * sizeof(Uint)));
        }
        const Uint32 blockCount = static_cast<Uint32>(program.GetActiveUniformBlocksCount());
        XXHASH_VERIFY(XXH64_update(state, &blockCount, sizeof(blockCount)));
        for (Uint32 i = 0; i < blockCount; ++i) {
            const Uint32 binding = program.GetUniformBlockBinding(i);
            XXHASH_VERIFY(XXH64_update(state, &binding, sizeof(binding)));
        }
        const Uint64 hash = XXH64_digest(state);
        XXH64_freeState(state);
        return hash;
    }

    Bool UniformDescriptorBinder::IsSamplerUniformType(GLenum glType) {
        switch (glType) {
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_BUFFER:
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
            return true;
        default:
            return false;
        }
    }

    Bool UniformDescriptorBinder::Initialize(VkDevice device, VmaAllocator allocator,
                                             VkDeviceSize minUniformBufferOffsetAlignment, Uint32 frameCount,
                                             Uint32 maxBindings, Uint32 setsPerFrame, VkDeviceSize perFrameUploadBytes,
                                             VkTextureSamplerManager* textureSamplerManager) {
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
        m_textureSamplerManager = textureSamplerManager;

        m_frames.resize(m_frameCount);
        for (Uint32 frameIndex = 0; frameIndex < m_frameCount; ++frameIndex) {
            auto& frame = m_frames[frameIndex];
            frame.writeCursor = 0;
            frame.descriptorPool = VK_NULL_HANDLE;

            VkDescriptorPoolSize poolSizes[2]{};
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            poolSizes[0].descriptorCount = m_setsPerFrame * m_maxBindings;
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[1].descriptorCount = m_setsPerFrame * m_maxBindings;

            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.maxSets = m_setsPerFrame;
            poolInfo.poolSizeCount = static_cast<Uint32>(std::size(poolSizes));
            poolInfo.pPoolSizes = poolSizes;
            VK_VERIFY(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &frame.descriptorPool),
                      "UniformDescriptorBinder::Initialize, vkCreateDescriptorPool(frame)");

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
            if (m_device != VK_NULL_HANDLE && frame.descriptorPool != VK_NULL_HANDLE) {
                vkDestroyDescriptorPool(m_device, frame.descriptorPool, nullptr);
            }
            frame.descriptorPool = VK_NULL_HANDLE;
            frame.writeCursor = 0;
        }
        m_frames.clear();
        DestroyProgramLayouts();

        m_allocator = nullptr;
        m_device = VK_NULL_HANDLE;
        m_minDynamicOffsetAlignment = 1;
        m_perFrameUploadBytes = 0;
        m_frameCount = 0;
        m_maxBindings = 0;
        m_setsPerFrame = 0;
        m_textureSamplerManager = nullptr;
    }

    void UniformDescriptorBinder::BeginFrame(Uint32 frameIndex) {
        MOBILEGL_ASSERT(frameIndex < m_frames.size(), "UniformDescriptorBinder::BeginFrame invalid frame index");
        auto& frame = m_frames[frameIndex];
        frame.writeCursor = 0;
        VK_VERIFY(vkResetDescriptorPool(m_device, frame.descriptorPool, 0),
                  "UniformDescriptorBinder::BeginFrame, vkResetDescriptorPool");
    }

    Bool UniformDescriptorBinder::ReflectBindingKinds(const MG_State::GLState::ProgramObject& program,
                                                      Vector<BindingKind>& outKinds) const {
        outKinds.assign(m_maxBindings, BindingKind::None);

        const auto& spirv = program.GetGeneratedSpirv();
        for (const auto& module : spirv) {
            if (module.empty()) {
                continue;
            }

            spvc_context context = nullptr;
            spvc_parsed_ir ir = nullptr;
            spvc_compiler compiler = nullptr;
            spvc_resources resources = nullptr;

            if (spvc_context_create(&context) != SPVC_SUCCESS) {
                return false;
            }

            const spvc_result parseResult = spvc_context_parse_spirv(context, module.data(), module.size(), &ir);
            if (parseResult != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }

            const spvc_result compilerResult =
                spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler);
            if (compilerResult != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }

            if (spvc_compiler_create_shader_resources(compiler, &resources) != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }

            const auto applyBindings = [&](spvc_resource_type resourceType, BindingKind kind) {
                const spvc_reflected_resource* list = nullptr;
                size_t count = 0;
                if (spvc_resources_get_resource_list_for_type(resources, resourceType, &list, &count) != SPVC_SUCCESS) {
                    return;
                }
                for (size_t i = 0; i < count; ++i) {
                    const Uint32 binding =
                        spvc_compiler_get_decoration(compiler, list[i].id, SpvDecorationBinding);
                    if (binding >= m_maxBindings) {
                        continue;
                    }
                    if (kind == BindingKind::CombinedImageSampler) {
                        outKinds[binding] = BindingKind::CombinedImageSampler;
                    } else if (outKinds[binding] == BindingKind::None) {
                        outKinds[binding] = kind;
                    }
                }
            };

            applyBindings(SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, BindingKind::UniformBufferDynamic);
            applyBindings(SPVC_RESOURCE_TYPE_SAMPLED_IMAGE, BindingKind::CombinedImageSampler);

            spvc_context_destroy(context);
        }

        return true;
    }

    UniformDescriptorBinder::ProgramLayout* UniformDescriptorBinder::GetOrCreateProgramLayout(
        const MG_State::GLState::ProgramObject& program) {
        const Uint64 hash = ComputeProgramHash(program);
        auto it = m_programLayouts.find(hash);
        if (it != m_programLayouts.end()) {
            return &it->second;
        }

        ProgramLayout layout{};
        layout.hash = hash;
        if (!ReflectBindingKinds(program, layout.bindingKinds)) {
            MGLOG_E("UniformDescriptorBinder::GetOrCreateProgramLayout failed: reflection failed");
            return nullptr;
        }

        Vector<VkDescriptorSetLayoutBinding> bindings;
        bindings.reserve(m_maxBindings);
        for (Uint32 binding = 0; binding < m_maxBindings; ++binding) {
            const auto kind = layout.bindingKinds[binding];
            if (kind == BindingKind::None) {
                continue;
            }

            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = binding;
            layoutBinding.descriptorCount = 1;
            layoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
            layoutBinding.pImmutableSamplers = nullptr;
            if (kind == BindingKind::UniformBufferDynamic) {
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                layout.dynamicBindings.push_back(binding);
            } else {
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            }
            bindings.push_back(layoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo setLayoutInfo{};
        setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        setLayoutInfo.bindingCount = static_cast<Uint32>(bindings.size());
        setLayoutInfo.pBindings = bindings.data();
        VK_VERIFY(vkCreateDescriptorSetLayout(m_device, &setLayoutInfo, nullptr, &layout.descriptorSetLayout),
                  "UniformDescriptorBinder::GetOrCreateProgramLayout, vkCreateDescriptorSetLayout");

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &layout.descriptorSetLayout;
        VK_VERIFY(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &layout.pipelineLayout),
                  "UniformDescriptorBinder::GetOrCreateProgramLayout, vkCreatePipelineLayout");

        auto [insertIt, _] = m_programLayouts.emplace(hash, std::move(layout));
        return &insertIt->second;
    }

    VkPipelineLayout UniformDescriptorBinder::GetOrCreatePipelineLayout(const MG_State::GLState::ProgramObject& program) {
        auto* layout = GetOrCreateProgramLayout(program);
        return layout ? layout->pipelineLayout : VK_NULL_HANDLE;
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
        if (m_frames.empty()) {
            MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: binder is not initialized");
            return false;
        }
        if (frameIndex >= m_frames.size()) {
            MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: invalid frame index %u", frameIndex);
            return false;
        }

        ProgramLayout* layout = GetOrCreateProgramLayout(program);
        if (!layout || layout->pipelineLayout == VK_NULL_HANDLE || layout->descriptorSetLayout == VK_NULL_HANDLE) {
            MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: cannot get program layout");
            return false;
        }
        if (layout->pipelineLayout != pipelineLayout) {
            MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: pipelineLayout mismatch");
            return false;
        }

        auto& frame = m_frames[frameIndex];
        if (frame.descriptorPool == VK_NULL_HANDLE) {
            MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: frame descriptor pool is invalid");
            return false;
        }

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = frame.descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout->descriptorSetLayout;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        VK_VERIFY(vkAllocateDescriptorSets(m_device, &allocInfo, &descriptorSet),
                  "UniformDescriptorBinder::BindProgramUniformBuffers, vkAllocateDescriptorSets");
        Vector<const void*> bindingData;
        Vector<VkDeviceSize> bindingSizes;
        if (!GatherBindingPayloads(program, bindingData, bindingSizes)) {
            MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: cannot gather UBO payloads");
            return false;
        }

        static const Uint8 kFallbackData[16] = {};
        VkDescriptorImageInfo fallbackImageInfo{};
        const Bool hasFallbackImage = m_textureSamplerManager && m_textureSamplerManager->GetFallbackDescriptor(fallbackImageInfo);

        Vector<VkWriteDescriptorSet> writes;
        writes.reserve(m_maxBindings);
        Vector<VkDescriptorBufferInfo> bufferInfos;
        Vector<VkDescriptorImageInfo> imageInfos;
        Vector<Uint32> dynamicOffsets;
        bufferInfos.reserve(m_maxBindings);
        imageInfos.reserve(m_maxBindings);
        dynamicOffsets.reserve(layout->dynamicBindings.size());

        for (Uint32 binding = 0; binding < m_maxBindings; ++binding) {
            const auto kind = layout->bindingKinds[binding];
            if (kind == BindingKind::None) {
                continue;
            }

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptorSet;
            write.dstBinding = binding;
            write.dstArrayElement = 0;
            write.descriptorCount = 1;

            if (kind == BindingKind::UniformBufferDynamic) {
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
                    MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: UBO upload failed on binding %u",
                            binding);
                    return false;
                }

                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = frame.uploadBuffer.GetHandle();
                bufferInfo.offset = 0;
                bufferInfo.range = payloadSize;
                bufferInfos.push_back(bufferInfo);

                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                write.pBufferInfo = &bufferInfos.back();
                writes.push_back(write);
                dynamicOffsets.push_back(static_cast<Uint32>(payloadOffset));
            } else {
                if (!hasFallbackImage) {
                    MGLOG_E("UniformDescriptorBinder::BindProgramUniformBuffers failed: fallback sampler/texture is unavailable");
                    return false;
                }
                imageInfos.push_back(fallbackImageInfo);
                write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write.pImageInfo = &imageInfos.back();
                writes.push_back(write);
            }
        }

        if (!writes.empty()) {
            vkUpdateDescriptorSets(m_device, static_cast<Uint32>(writes.size()), writes.data(), 0, nullptr);
        }

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet,
                                static_cast<Uint32>(dynamicOffsets.size()), dynamicOffsets.data());
        return true;
    }

    void UniformDescriptorBinder::DestroyProgramLayouts() {
        for (auto& [_, layout] : m_programLayouts) {
            if (layout.pipelineLayout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(m_device, layout.pipelineLayout, nullptr);
                layout.pipelineLayout = VK_NULL_HANDLE;
            }
            if (layout.descriptorSetLayout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(m_device, layout.descriptorSetLayout, nullptr);
                layout.descriptorSetLayout = VK_NULL_HANDLE;
            }
        }
        m_programLayouts.clear();
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
