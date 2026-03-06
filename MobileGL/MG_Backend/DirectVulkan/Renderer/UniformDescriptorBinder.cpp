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
#include <limits>

namespace MobileGL::MG_Backend::DirectVulkan {
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

    TextureTarget UniformDescriptorBinder::UniformTypeToTextureTarget(GLenum glType) {
        switch (glType) {
        case GL_SAMPLER_1D:
        case GL_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_1D:
            return TextureTarget::Texture1D;
        case GL_SAMPLER_3D:
        case GL_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
            return TextureTarget::Texture3D;
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
            return TextureTarget::TextureCubeMap;
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
            return TextureTarget::Texture2DMultisample;
        case GL_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
            return TextureTarget::TextureBuffer;
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
            return TextureTarget::Texture1DArray;
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
            return TextureTarget::Texture2DArray;
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
            return TextureTarget::Texture2DMultisampleArray;
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
            return TextureTarget::TextureRectangle;
        case GL_SAMPLER_2D:
        case GL_SAMPLER_2D_SHADOW:
        case GL_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        default:
            return TextureTarget::Texture2D;
        }
    }

    Bool UniformDescriptorBinder::Initialize(VkDevice device, VmaAllocator allocator,
                                             VkDeviceSize minUniformBufferOffsetAlignment, Uint32 frameCount,
                                             Uint32 maxBindings, Uint32 setsPerFrame, VkDeviceSize perFrameUploadBytes,
                                             VkTextureManager* textureManager, VkSamplerManager* samplerManager) {
        Shutdown();

        MOBILEGL_ASSERT(device != VK_NULL_HANDLE, "UniformDescriptorBinder::Initialize requires valid VkDevice");
        MOBILEGL_ASSERT(allocator != nullptr, "UniformDescriptorBinder::Initialize requires valid VMA allocator");
        MOBILEGL_ASSERT(frameCount > 0, "UniformDescriptorBinder::Initialize requires frameCount > 0");
        MOBILEGL_ASSERT(maxBindings > 0, "UniformDescriptorBinder::Initialize requires maxBindings > 0");
        MOBILEGL_ASSERT(setsPerFrame > 0, "UniformDescriptorBinder::Initialize requires setsPerFrame > 0");
        MOBILEGL_ASSERT(textureManager != nullptr,
                        "UniformDescriptorBinder::Initialize requires valid texture manager");
        MOBILEGL_ASSERT(samplerManager != nullptr,
                        "UniformDescriptorBinder::Initialize requires valid sampler manager");

        m_device = device;
        m_allocator = allocator;
        m_minDynamicOffsetAlignment = std::max<VkDeviceSize>(1, minUniformBufferOffsetAlignment);
        m_perFrameUploadBytes = perFrameUploadBytes;
        m_frameCount = frameCount;
        m_maxBindings = maxBindings;
        m_setsPerFrame = setsPerFrame;
        m_peakDescriptorSetsObserved = 0;
        m_textureManager = textureManager;
        m_samplerManager = samplerManager;

        m_frames.resize(m_frameCount);
        for (Uint32 frameIndex = 0; frameIndex < m_frameCount; ++frameIndex) {
            auto& frame = m_frames[frameIndex];
            frame.writeCursor = 0;
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
            MGLOG_D("UniformDescriptorBinder: frame %u descriptor pool created (maxSets=%u)", frameIndex, m_setsPerFrame);

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
        frame.writeCursor = 0;
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

    Bool UniformDescriptorBinder::ReflectSamplerBindings(const MG_State::GLState::ProgramObject& program,
                                                         ProgramLayout& layout) const {
        layout.samplerUniformLocationByBinding.assign(m_maxBindings, -1);
        layout.samplerTextureTargetByBinding.assign(m_maxBindings, TextureTarget::Texture2D);

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
            if (spvc_context_parse_spirv(context, module.data(), module.size(), &ir) != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }
            if (spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP,
                                             &compiler) != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }
            if (spvc_compiler_create_shader_resources(compiler, &resources) != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }

            const spvc_reflected_resource* list = nullptr;
            size_t count = 0;
            if (spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_SAMPLED_IMAGE, &list, &count) ==
                SPVC_SUCCESS) {
                for (size_t i = 0; i < count; ++i) {
                    const Uint32 binding =
                        spvc_compiler_get_decoration(compiler, list[i].id, SpvDecorationBinding);
                    if (binding >= m_maxBindings) {
                        continue;
                    }

                    String uniformName = list[i].name ? list[i].name : "";
                    Int location = program.GetUniformLocation(uniformName);
                    if (location < 0) {
                        const auto arraySuffix = uniformName.find("[0]");
                        if (arraySuffix != String::npos) {
                            uniformName = uniformName.substr(0, arraySuffix);
                            location = program.GetUniformLocation(uniformName);
                        }
                    }
                    if (location < 0) {
                        continue;
                    }

                    layout.samplerUniformLocationByBinding[binding] = location;
                    layout.samplerTextureTargetByBinding[binding] =
                        UniformTypeToTextureTarget(program.GetUniformType(static_cast<Uint>(location)));
                }
            }

            spvc_context_destroy(context);
        }

        return true;
    }

    Bool UniformDescriptorBinder::ReflectGlobalUboBinding(const MG_State::GLState::ProgramObject& program,
                                                          ProgramLayout& layout) const {
        layout.globalUboBinding = -1;

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
            if (spvc_context_parse_spirv(context, module.data(), module.size(), &ir) != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }
            if (spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP,
                                             &compiler) != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }
            if (spvc_compiler_create_shader_resources(compiler, &resources) != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }

            const spvc_reflected_resource* list = nullptr;
            size_t count = 0;
            if (spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, &list, &count) ==
                SPVC_SUCCESS) {
                for (size_t i = 0; i < count; ++i) {
                    const char* name = list[i].name ? list[i].name : "";
                    if (std::strstr(name, MG_Util::ShaderTranspiler::GLOBAL_UBO_NAME) == nullptr) {
                        continue;
                    }
                    const Uint32 binding =
                        spvc_compiler_get_decoration(compiler, list[i].id, SpvDecorationBinding);
                    if (binding < m_maxBindings) {
                        layout.globalUboBinding = static_cast<Int>(binding);
                    }
                    break;
                }
            }

            spvc_context_destroy(context);
            if (layout.globalUboBinding >= 0) {
                break;
            }
        }
        return true;
    }

    Bool UniformDescriptorBinder::ResolveSamplerDescriptor(VkCommandBuffer commandBuffer,
                                                           const MG_State::GLState::ProgramObject& program,
                                                           const ProgramLayout& layout, Uint32 binding,
                                                           VkDescriptorImageInfo& outImageInfo) const {
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

        const MG_State::GLState::SamplerObject* samplerToUse = samplerOverride ? samplerOverride.get() : texture->GetSamplerObject().get();
        if (!samplerToUse) {
            return false;
        }
        VkTextureManager::TextureResource* resource =
            m_textureManager->SyncTextureAndGetDescriptor(*texture);
        if (resource == nullptr) {
            return false;
        }
        MOBILEGL_ASSERT(IsValidSampledImageLayout(resource->layout),
                        "ResolveSamplerDescriptor: invalid sampled image layout=%d for textureId=%d, binding=%u",
                        static_cast<Int>(resource->layout), texture->GetExternalIndex(), binding);
        outImageInfo = {
            .sampler = m_samplerManager->GetOrCreateSampler(*samplerToUse),
            .imageView = resource->view,
            .imageLayout = resource->layout,
        };
        return outImageInfo.sampler != VK_NULL_HANDLE;
    }

    Bool UniformDescriptorBinder::ResolveSamplerTexture(const MG_State::GLState::ProgramObject& program,
                                                        const ProgramLayout& layout, Uint32 binding,
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
        ProgramLayout* layout = GetOrCreateProgramLayout(program);
        if (layout == nullptr) {
            return false;
        }

        for (Uint32 binding = 0; binding < m_maxBindings; ++binding) {
            if (layout->bindingKinds[binding] != BindingKind::CombinedImageSampler) {
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
        if (!ReflectSamplerBindings(program, layout)) {
            MGLOG_E("UniformDescriptorBinder::GetOrCreateProgramLayout failed: sampler reflection failed");
            return nullptr;
        }
        if (!ReflectGlobalUboBinding(program, layout)) {
            MGLOG_E("UniformDescriptorBinder::GetOrCreateProgramLayout failed: global UBO reflection failed");
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
            MGLOG_E("UniformDescriptorBinder::CreateDescriptorPool failed: vkCreateDescriptorPool returned %d", result);
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
        ProgramLayout* layout = GetOrCreateProgramLayout(program);

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
                frame.peakAllocatedSetsThisFrame = std::max(frame.peakAllocatedSetsThisFrame, frame.allocatedSetsThisFrame);
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
                VkDescriptorImageInfo imageInfo{};
                Bool hasImage = ResolveSamplerDescriptor(commandBuffer, program, *layout, binding, imageInfo);
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

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout->pipelineLayout, 0, 1, &descriptorSet,
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
