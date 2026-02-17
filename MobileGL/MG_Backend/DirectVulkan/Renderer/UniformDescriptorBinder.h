// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/UniformDescriptorBinder.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "VkBufferObject.h"
#include "VkTextureSamplerManager.h"
#include "../VkIncludes.h"
#include <Includes.h>
#include <vk_mem_alloc.h>

namespace MobileGL::MG_State::GLState {
    class ProgramObject;
}

namespace MobileGL::MG_Backend::DirectVulkan {
    class UniformDescriptorBinder {
    public:
        enum class BindingKind : Uint8 {
            None = 0,
            UniformBufferDynamic,
            CombinedImageSampler
        };

        Bool Initialize(VkDevice device, VmaAllocator allocator, VkDeviceSize minUniformBufferOffsetAlignment,
                        Uint32 frameCount, Uint32 maxBindings = 16, Uint32 setsPerFrame = 64,
                        VkDeviceSize perFrameUploadBytes = 4 * 1024 * 1024,
                        VkTextureSamplerManager* textureSamplerManager = nullptr);
        void Shutdown();

        void BeginFrame(Uint32 frameIndex);
        VkPipelineLayout GetOrCreatePipelineLayout(const MG_State::GLState::ProgramObject& program);
        Bool BindProgramUniformBuffers(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
                                       const MG_State::GLState::ProgramObject& program, Uint32 frameIndex);

    private:
        struct FrameResources {
            VkBufferObject uploadBuffer;
            VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
            VkDeviceSize writeCursor = 0;
        };

        struct ProgramLayout {
            Uint64 hash = 0;
            VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
            VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
            Vector<BindingKind> bindingKinds;
            Vector<Uint32> dynamicBindings;
        };

        static VkDeviceSize AlignUp(VkDeviceSize value, VkDeviceSize alignment);
        static Uint64 ComputeProgramHash(const MG_State::GLState::ProgramObject& program);
        static Bool IsSamplerUniformType(GLenum glType);
        Bool ReflectBindingKinds(const MG_State::GLState::ProgramObject& program, Vector<BindingKind>& outKinds) const;
        ProgramLayout* GetOrCreateProgramLayout(const MG_State::GLState::ProgramObject& program);
        Bool AllocateUploadRegion(FrameResources& frame, VkDeviceSize size, VkDeviceSize& outOffset);
        Bool GatherBindingPayloads(const MG_State::GLState::ProgramObject& program, Vector<const void*>& outData,
                                   Vector<VkDeviceSize>& outSizes) const;
        void DestroyProgramLayouts();

        VkDevice m_device = VK_NULL_HANDLE;
        VmaAllocator m_allocator = nullptr;
        Vector<FrameResources> m_frames;
        UnorderedMap<Uint64, ProgramLayout> m_programLayouts;

        VkDeviceSize m_minDynamicOffsetAlignment = 1;
        VkDeviceSize m_perFrameUploadBytes = 0;
        Uint32 m_frameCount = 0;
        Uint32 m_maxBindings = 0;
        Uint32 m_setsPerFrame = 0;
        VkTextureSamplerManager* m_textureSamplerManager = nullptr;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan

