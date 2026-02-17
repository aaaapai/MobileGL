// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/UniformDescriptorBinder.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "VkBufferObject.h"
#include "../VkIncludes.h"
#include <Includes.h>
#include <vk_mem_alloc.h>

namespace MobileGL::MG_State::GLState {
    class ProgramObject;
}

namespace MobileGL::MG_Backend::DirectVulkan {
    class UniformDescriptorBinder {
    public:
        Bool Initialize(VkDevice device, VmaAllocator allocator, VkDeviceSize minUniformBufferOffsetAlignment,
                        Uint32 frameCount, Uint32 maxBindings = 16, Uint32 setsPerFrame = 64,
                        VkDeviceSize perFrameUploadBytes = 4 * 1024 * 1024);
        void Shutdown();

        void BeginFrame(Uint32 frameIndex);
        Bool BindProgramUniformBuffers(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
                                       const MG_State::GLState::ProgramObject& program, Uint32 frameIndex);

        VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_descriptorSetLayout; }

    private:
        struct FrameResources {
            VkBufferObject uploadBuffer;
            VkDeviceSize writeCursor = 0;
            Uint32 descriptorCursor = 0;
        };

        static VkDeviceSize AlignUp(VkDeviceSize value, VkDeviceSize alignment);
        Bool AllocateUploadRegion(FrameResources& frame, VkDeviceSize size, VkDeviceSize& outOffset);
        Bool GatherBindingPayloads(const MG_State::GLState::ProgramObject& program, Vector<const void*>& outData,
                                   Vector<VkDeviceSize>& outSizes) const;

        VkDevice m_device = VK_NULL_HANDLE;
        VmaAllocator m_allocator = nullptr;
        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

        Vector<VkDescriptorSet> m_descriptorSets;
        Vector<FrameResources> m_frames;

        VkDeviceSize m_minDynamicOffsetAlignment = 1;
        VkDeviceSize m_perFrameUploadBytes = 0;
        Uint32 m_frameCount = 0;
        Uint32 m_maxBindings = 0;
        Uint32 m_setsPerFrame = 0;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan

