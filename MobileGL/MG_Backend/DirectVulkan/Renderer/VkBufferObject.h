// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkBufferObject.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "../VkIncludes.h"
#include <Includes.h>
#include <vk_mem_alloc.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    class VkBufferObject {
    public:
        VkBufferObject() = default;
        ~VkBufferObject();

        VkBufferObject(const VkBufferObject&) = delete;
        VkBufferObject& operator=(const VkBufferObject&) = delete;

        Bool Create(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
                    VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocationFlags = 0);
        void Destroy();

        Bool Upload(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);

        VkBuffer GetHandle() const { return m_buffer; }
        VkDeviceSize GetSize() const { return m_size; }
        Bool IsValid() const { return m_allocator != nullptr && m_buffer != VK_NULL_HANDLE && m_allocation != nullptr; }

    private:
        VmaAllocator m_allocator = nullptr;
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VmaAllocation m_allocation = nullptr;
        VkDeviceSize m_size = 0;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan

