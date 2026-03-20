// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkBufferManager.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "BufferArena.h"
#include "../VkIncludes.h"
#include <Includes.h>
#include <vk_mem_alloc.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    enum class TransientBufferKind : Uint8 {
        Vertex,
        Index,
        Uniform,
    };

    struct VkBufferManagerInitInfo {
        VmaAllocator allocator = nullptr;
        Uint32 frameCount = 0;
        VkDeviceSize minUploadBytes = 4 * 1024 * 1024;
        VmaMemoryUsage transientMemoryUsage = VMA_MEMORY_USAGE_AUTO;
        VmaAllocationCreateFlags transientAllocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        Bool transientPersistentMapping = false;
    };

    class VkBufferManager {
    public:
        Bool Initialize(const VkBufferManagerInitInfo& initInfo);
        void Shutdown();

        // Recreate all per-frame transient arenas
        Bool RecreateTransientArenas(Uint32 frameCount);
        void BeginFrame(Uint32 frameIndex);

        Bool UploadTransient(TransientBufferKind kind, Uint32 frameIndex, const void* data, VkDeviceSize size,
                             VkDeviceSize alignment, BufferSlice& outSlice);

    private:
        Bool InitializeTransientArenas();

        VkBufferManagerInitInfo m_initInfo{};
        BufferArena m_transientUploadArena;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
