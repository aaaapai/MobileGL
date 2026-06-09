// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkBufferManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VkBufferManager.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    namespace {
        constexpr Uint32 kResidentBufferGCInterval = 60;
        constexpr VmaAllocationCreateFlags kResidentBufferAllocationFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    } // namespace

    Bool VkBufferManager::Initialize(const VkBufferManagerInitInfo& initInfo) {
        Shutdown();

        MOBILEGL_ASSERT(initInfo.allocator != nullptr, "VkBufferManager::Initialize requires valid allocator");
        MOBILEGL_ASSERT(initInfo.frameCount > 0, "VkBufferManager::Initialize requires non-zero frame count");

        m_initInfo = initInfo;
        m_deferredResidentReleases.resize(initInfo.frameCount);
        m_currentFrameIndex = 0;
        return InitializeTransientArenas();
    }

    void VkBufferManager::Shutdown() {
        m_transientUploadArena.Shutdown();
        DestroyResidentBuffers();
        DestroyDeferredResidentReleases();
        m_initInfo = {};
        m_currentFrameIndex = 0;
        m_residentGcTick = 0;
    }

    Bool VkBufferManager::RecreateTransientArenas(Uint32 frameCount) {
        MOBILEGL_ASSERT(m_initInfo.allocator != nullptr, "VkBufferManager::RecreateTransientArenas requires initialized manager");
        MOBILEGL_ASSERT(frameCount > 0, "VkBufferManager::RecreateTransientArenas requires non-zero frame count");

        m_transientUploadArena.Shutdown();
        m_initInfo.frameCount = frameCount;
        DestroyDeferredResidentReleases();
        m_deferredResidentReleases.resize(frameCount);
        m_currentFrameIndex = 0;
        return InitializeTransientArenas();
    }

    void VkBufferManager::BeginFrame(Uint32 frameIndex) {
        MOBILEGL_ASSERT(frameIndex < m_deferredResidentReleases.size(),
                        "VkBufferManager::BeginFrame frame index out of range");
        m_currentFrameIndex = frameIndex;
        CollectDeferredResidentReleases(frameIndex);
        m_transientUploadArena.BeginFrame(frameIndex);
    }

    Bool VkBufferManager::UploadTransient(BufferKind kind, Uint32 frameIndex, const void* data,
                                          VkDeviceSize size, VkDeviceSize alignment, BufferSlice& outSlice) {
        (void)kind;
        return m_transientUploadArena.Upload(frameIndex, data, size, alignment, outSlice);
    }

    Bool VkBufferManager::InitializeTransientArenas() {
        return m_transientUploadArena.Initialize({
            .allocator = m_initInfo.allocator,
            .frameCount = m_initInfo.frameCount,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
            .memoryUsage = m_initInfo.transientMemoryUsage,
            .allocationFlags = m_initInfo.transientAllocationFlags,
            .minBufferSize = m_initInfo.minUploadBytes,
            .persistentlyMapped = m_initInfo.transientPersistentMapping,
        });
    }

    Bool VkBufferManager::SyncResidentBuffer(BufferKind kind,
                                             const SharedPtr<MG_State::GLState::BufferObject>& bufferObject,
                                             BufferSlice& outSlice) {
        const VkBufferUsageFlags requiredUsage = GetVkBufferUsage(kind);
        MOBILEGL_ASSERT(requiredUsage != 0,
                        "VkBufferManager::SyncResidentBuffer unsupported resident buffer kind");
        MOBILEGL_ASSERT(bufferObject != nullptr, "VkBufferManager::SyncResidentBuffer requires valid buffer object");
        CollectResidentGarbageIfNeeded();

        const auto* bufferData = bufferObject->GetDataReadOnly().get();
        MOBILEGL_ASSERT(bufferData != nullptr, "VkBufferManager::SyncResidentBuffer requires frontend buffer data");
        bufferObject->MarkPersistentMappedRangeDirty();

        const VkDeviceSize bufferSize = static_cast<VkDeviceSize>(bufferObject->GetSize());
        if (bufferSize == 0) {
            MGLOG_E("VkBufferManager::SyncResidentBuffer failed: buffer size is zero");
            return false;
        }

        auto& entry = m_residentBuffers[bufferObject.get()];
        entry.aliveRef = bufferObject;

        const auto changeBits = bufferObject->GetChangeBits();
        const Bool needsRecreate = !entry.buffer.IsValid() || entry.size != bufferSize ||
                                   ((entry.usage & requiredUsage) != requiredUsage) ||
                                   (changeBits & BufferChangeBits::PreferReallocationBit);
        if (needsRecreate) {
            const VkBufferUsageFlags recreatedUsage = entry.usage | requiredUsage;
            DeferResidentRelease(std::move(entry.buffer));
            const Bool created = entry.buffer.Create({
                .allocator = m_initInfo.allocator,
                .size = bufferSize,
                .usage = recreatedUsage,
                .memoryUsage = VMA_MEMORY_USAGE_AUTO,
                .allocationFlags = kResidentBufferAllocationFlags,
            });
            if (!created || entry.buffer.Map() == nullptr) {
                MGLOG_E("VkBufferManager::SyncResidentBuffer failed: unable to create resident buffer");
                entry.buffer.Destroy();
                entry.size = 0;
                entry.usage = 0;
                return false;
            }
            if (!entry.buffer.Upload(bufferData->data(), bufferSize, 0)) {
                MGLOG_E("VkBufferManager::SyncResidentBuffer failed: initial upload failed");
                entry.buffer.Destroy();
                entry.size = 0;
                entry.usage = 0;
                return false;
            }
            entry.size = bufferSize;
            entry.usage = recreatedUsage;
            bufferObject->ClearDirty();
            outSlice = entry.buffer.GetSlice(0, bufferSize);
            return true;
        }

        if (changeBits & BufferChangeBits::DirtyBit) {
            const auto& dirtyRanges = bufferObject->GetDirtyRanges();
            for (const auto& range : dirtyRanges) {
                const VkDeviceSize rangeOffset = static_cast<VkDeviceSize>(range.start);
                const VkDeviceSize rangeSize = static_cast<VkDeviceSize>(range.end - range.start);
                if (rangeSize == 0) {
                    continue;
                }
                if (!entry.buffer.Upload(bufferData->data() + range.start, rangeSize, rangeOffset)) {
                    MGLOG_E("VkBufferManager::SyncResidentBuffer failed: dirty range upload failed");
                    return false;
                }
            }
            bufferObject->ClearDirty();
        }

        outSlice = entry.buffer.GetSlice(0, bufferSize);
        return true;
    }

    void VkBufferManager::DowngradeResidentBufferToTransient(const SharedPtr<MG_State::GLState::BufferObject>& bufferObject) {
        if (bufferObject == nullptr) {
            return;
        }

        auto it = m_residentBuffers.find(bufferObject.get());
        if (it == m_residentBuffers.end()) {
            return;
        }

        DeferResidentRelease(std::move(it->second.buffer));
        m_residentBuffers.erase(it);
    }

    void VkBufferManager::DeferResidentRelease(VkBufferObject&& buffer) {
        if (!buffer.IsValid()) {
            return;
        }

        if (m_deferredResidentReleases.empty()) {
            buffer.Destroy();
            return;
        }

        MOBILEGL_ASSERT(m_currentFrameIndex < m_deferredResidentReleases.size(),
                        "VkBufferManager::DeferResidentRelease current frame index out of range");
        m_deferredResidentReleases[m_currentFrameIndex].push_back(std::move(buffer));
    }

    void VkBufferManager::CollectDeferredResidentReleases(Uint32 frameIndex) {
        MOBILEGL_ASSERT(frameIndex < m_deferredResidentReleases.size(),
                        "VkBufferManager::CollectDeferredResidentReleases frame index out of range");
        m_deferredResidentReleases[frameIndex].clear();
    }

    VkBufferUsageFlags VkBufferManager::GetVkBufferUsage(BufferKind kind) {
        switch (kind) {
        case BufferKind::Vertex:
        case BufferKind::Index:
            // A GL buffer can be rebound between ARRAY_BUFFER and ELEMENT_ARRAY_BUFFER,
            // and may even be used as both within the same draw setup. Keep resident
            // vertex/index buffers compatible with both roles from the start so we
            // never need to recreate a buffer after it has already been bound.
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case BufferKind::Uniform:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        case BufferKind::TextureBuffer:
            return VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
        case BufferKind::ShaderStorage:
            return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        case BufferKind::Indirect:
            return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        default:
            return 0;
        }
    }

    void VkBufferManager::CollectResidentGarbageIfNeeded() {
        ++m_residentGcTick;
        if (m_residentGcTick < kResidentBufferGCInterval) {
            return;
        }
        CollectResidentGarbageNow();
        m_residentGcTick = 0;
    }

    void VkBufferManager::CollectResidentGarbageNow() {
        Vector<MG_State::GLState::BufferObject*> staleBuffers;
        staleBuffers.reserve(m_residentBuffers.size());

        for (const auto& [rawBuffer, entry] : m_residentBuffers) {
            if (entry.aliveRef.expired()) {
                staleBuffers.push_back(rawBuffer);
            }
        }

        for (const auto* rawBuffer : staleBuffers) {
            auto it = m_residentBuffers.find(const_cast<MG_State::GLState::BufferObject*>(rawBuffer));
            if (it == m_residentBuffers.end()) {
                continue;
            }
            DeferResidentRelease(std::move(it->second.buffer));
            m_residentBuffers.erase(it);
        }
    }

    void VkBufferManager::DestroyDeferredResidentReleases() {
        for (auto& deferredReleases : m_deferredResidentReleases) {
            deferredReleases.clear();
        }
        m_deferredResidentReleases.clear();
    }

    void VkBufferManager::DestroyResidentBuffers() {
        for (auto& [_, entry] : m_residentBuffers) {
            entry.buffer.Destroy();
        }
        m_residentBuffers.clear();
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
