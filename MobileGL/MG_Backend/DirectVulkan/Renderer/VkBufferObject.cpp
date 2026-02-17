// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkBufferObject.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VkBufferObject.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    VkBufferObject::VkBufferObject(VkBufferObject&& other) noexcept {
        m_allocator = other.m_allocator;
        m_buffer = other.m_buffer;
        m_allocation = other.m_allocation;
        m_size = other.m_size;

        other.m_allocator = nullptr;
        other.m_buffer = VK_NULL_HANDLE;
        other.m_allocation = nullptr;
        other.m_size = 0;
    }

    VkBufferObject& VkBufferObject::operator=(VkBufferObject&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        Destroy();

        m_allocator = other.m_allocator;
        m_buffer = other.m_buffer;
        m_allocation = other.m_allocation;
        m_size = other.m_size;

        other.m_allocator = nullptr;
        other.m_buffer = VK_NULL_HANDLE;
        other.m_allocation = nullptr;
        other.m_size = 0;
        return *this;
    }

    VkBufferObject::~VkBufferObject() {
        Destroy();
    }

    Bool VkBufferObject::Create(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
                                VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocationFlags) {
        MOBILEGL_ASSERT(allocator != nullptr, "VkBufferObject::Create requires valid VMA allocator");
        MOBILEGL_ASSERT(size > 0, "VkBufferObject::Create requires non-zero size");

        Destroy();
        m_allocator = allocator;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.usage = memoryUsage;
        allocationInfo.flags = allocationFlags;

        const VkResult result =
            vmaCreateBuffer(m_allocator, &bufferInfo, &allocationInfo, &m_buffer, &m_allocation, nullptr);
        if (result != VK_SUCCESS) {
            MGLOG_E("VkBufferObject::Create failed: vmaCreateBuffer returned %d", result);
            m_allocator = nullptr;
            m_buffer = VK_NULL_HANDLE;
            m_allocation = nullptr;
            m_size = 0;
            return false;
        }

        m_size = size;
        return true;
    }

    void VkBufferObject::Destroy() {
        if (m_allocator != nullptr && m_buffer != VK_NULL_HANDLE && m_allocation != nullptr) {
            vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
        }
        m_buffer = VK_NULL_HANDLE;
        m_allocation = nullptr;
        m_allocator = nullptr;
        m_size = 0;
    }

    Bool VkBufferObject::Upload(const void* data, VkDeviceSize size, VkDeviceSize offset) {
        MOBILEGL_ASSERT(IsValid(), "VkBufferObject::Upload called on invalid buffer");
        MOBILEGL_ASSERT(data != nullptr || size == 0, "VkBufferObject::Upload data pointer is null");
        MOBILEGL_ASSERT(offset + size <= m_size, "VkBufferObject::Upload out of range");

        if (size == 0) {
            return true;
        }

        void* mapped = nullptr;
        const VkResult mapResult = vmaMapMemory(m_allocator, m_allocation, &mapped);
        if (mapResult != VK_SUCCESS || mapped == nullptr) {
            MGLOG_E("VkBufferObject::Upload failed: vmaMapMemory returned %d", mapResult);
            return false;
        }

        Memcpy(static_cast<Uint8*>(mapped) + offset, data, static_cast<SizeT>(size));
        vmaUnmapMemory(m_allocator, m_allocation);
        return true;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
