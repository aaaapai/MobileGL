// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkBufferManager.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "BufferArena.h"
#include "MG_State/GLState/BufferState/BufferObject.h"
#include "../VkIncludes.h"
#include <Includes.h>
#include <vk_mem_alloc.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    enum class BufferKind : Uint8 {
        Vertex,
        Index,
        Uniform,
        TextureBuffer,
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

        Bool UploadTransient(BufferKind kind, Uint32 frameIndex, const void* data, VkDeviceSize size,
                             VkDeviceSize alignment, BufferSlice& outSlice);
        Bool SyncResidentBuffer(BufferKind kind, const SharedPtr<MG_State::GLState::BufferObject>& bufferObject,
                                BufferSlice& outSlice);
        void DowngradeResidentBufferToTransient(const SharedPtr<MG_State::GLState::BufferObject>& bufferObject);

    private:
        struct ResidentBufferEntry {
            WeakPtr<MG_State::GLState::BufferObject> aliveRef;
            VkBufferObject buffer;
            VkDeviceSize size = 0;
            VkBufferUsageFlags usage = 0;
        };

        Bool InitializeTransientArenas();
        static VkBufferUsageFlags GetVkBufferUsage(BufferKind kind);
        void DeferResidentRelease(VkBufferObject&& buffer);
        void CollectDeferredResidentReleases(Uint32 frameIndex);
        void CollectResidentGarbageIfNeeded();
        void CollectResidentGarbageNow();
        void DestroyDeferredResidentReleases();
        void DestroyResidentBuffers();

        VkBufferManagerInitInfo m_initInfo{};
        BufferArena m_transientUploadArena;
        UnorderedMap<MG_State::GLState::BufferObject*, ResidentBufferEntry> m_residentBuffers;
        Vector<Vector<VkBufferObject>> m_deferredResidentReleases;
        Uint32 m_currentFrameIndex = 0;
        Uint32 m_residentGcTick = 0;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
