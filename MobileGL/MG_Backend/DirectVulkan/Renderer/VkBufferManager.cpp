// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkBufferManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VkBufferManager.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    Bool VkBufferManager::Initialize(const VkBufferManagerInitInfo& initInfo) {
        Shutdown();

        MOBILEGL_ASSERT(initInfo.allocator != nullptr, "VkBufferManager::Initialize requires valid allocator");
        MOBILEGL_ASSERT(initInfo.frameCount > 0, "VkBufferManager::Initialize requires non-zero frame count");

        m_initInfo = initInfo;
        return InitializeTransientArenas();
    }

    void VkBufferManager::Shutdown() {
        m_transientUploadArena.Shutdown();
        m_initInfo = {};
    }

    Bool VkBufferManager::RecreateTransientArenas(Uint32 frameCount) {
        MOBILEGL_ASSERT(m_initInfo.allocator != nullptr, "VkBufferManager::RecreateTransientArenas requires initialized manager");
        MOBILEGL_ASSERT(frameCount > 0, "VkBufferManager::RecreateTransientArenas requires non-zero frame count");

        m_transientUploadArena.Shutdown();
        m_initInfo.frameCount = frameCount;
        return InitializeTransientArenas();
    }

    void VkBufferManager::BeginFrame(Uint32 frameIndex) {
        m_transientUploadArena.BeginFrame(frameIndex);
    }

    Bool VkBufferManager::UploadTransient(TransientBufferKind kind, Uint32 frameIndex, const void* data,
                                          VkDeviceSize size, VkDeviceSize alignment, BufferSlice& outSlice) {
        (void)kind;
        return m_transientUploadArena.Upload(frameIndex, data, size, alignment, outSlice);
    }

    Bool VkBufferManager::InitializeTransientArenas() {
        return m_transientUploadArena.Initialize({
            .allocator = m_initInfo.allocator,
            .frameCount = m_initInfo.frameCount,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            .memoryUsage = m_initInfo.transientMemoryUsage,
            .allocationFlags = m_initInfo.transientAllocationFlags,
            .minBufferSize = m_initInfo.minUploadBytes,
            .persistentlyMapped = m_initInfo.transientPersistentMapping,
        });
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
