// MobileGL - MobileGL/MG_State/GLState/BufferState/BufferObject.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include "MG_Util/Types.h"
#include <Includes.h>

namespace MobileGL {
    enum class BufferTarget {
        Vertex,
        Index,
        Uniform,
        CopyRead,
        CopyWrite,
        PixelPack,
        PixelUnpack,
        Query,
        Texture,
        TransformFeedback,
        AtomicCounter,
        DispatchIndirect,
        DrawIndirect,
        ShaderStorage,
        BufferTargetCount,
        Unknown = -1
    };

    enum class BufferUsage {
        StreamDraw,
        StreamRead,
        StreamCopy,
        StaticDraw,
        StaticRead,
        StaticCopy,
        DynamicDraw,
        DynamicRead,
        DynamicCopy,
        Unknown = -1
    };

    enum class BufferMappingAccessBit : Uint {
        Null = 0x00,
        Read = 0x01,
        Write = 0x02,
        InvalidateRange = 0x04,
        InvalidateBuffer = 0x08,
        FlushExplicit = 0x10,
        Unsynchronized = 0x20,
        Persistent = 0x40,
        Coherent = 0x80
    };

    enum class BufferStorageFlags : Uint32 {
        None = 0,
        DynamicStorage = 1 << 0,   // GL_DYNAMIC_STORAGE_BIT
        MapRead = 1 << 1,          // GL_MAP_READ_BIT
        MapWrite = 1 << 2,         // GL_MAP_WRITE_BIT
        MapPersistent = 1 << 3,    // GL_MAP_PERSISTENT_BIT
        MapCoherent = 1 << 4,      // GL_MAP_COHERENT_BIT
        ClientStorage = 1 << 5     // GL_CLIENT_STORAGE_BIT
    };

    inline BufferStorageFlags operator|(BufferStorageFlags a, BufferStorageFlags b) {
        return static_cast<BufferStorageFlags>(static_cast<Uint32>(a) | static_cast<Uint32>(b));
    }

    inline BufferStorageFlags operator&(BufferStorageFlags a, BufferStorageFlags b) {
        return static_cast<BufferStorageFlags>(static_cast<Uint32>(a) & static_cast<Uint32>(b));
    }

    inline bool operator!(BufferStorageFlags a) {
        return static_cast<Uint32>(a) == 0;
    }

    namespace MG_State {
        namespace GLState {
            class BufferObject {
            public:
                using TargetEnum = BufferTarget;

                BufferObject(Uint externalIndex);

                void Resize(SizeT size);
                void UploadData(DataPtr data, SizeT atOffset);
                void SetUsage(BufferUsage usage);
                void* AcquireMemory(Bool markMapped, Bool read, Bool write);
                void* AcquireMemoryRange(Range1D range, Flags<BufferMappingAccessBit> access);
                void ReleaseMemory();
                void FlushMemoryRange(SizeT offset, SizeT length);
                void UploadSubData(DataPtr data, SizeT atOffset);
                void CopyDataFrom(const SharedPtr<BufferObject>& src, SizeT srcOffset, SizeT dstOffset, SizeT size);
                void ClearDirty();

                Bool IsMapped() const;
                SizeT GetSize() const;
                BufferUsage GetUsage() const;
                Range1D GetDirtyRange() const;
                Range1D GetMappedRange() const;
                const SharedPtr<Data> GetDataReadOnly() const;
                Flags<BufferMappingAccessBit> GetMappingAccess() const;
                Uint GetExternalIndex() const;

                void AllocateImmutableStorage(SizeT size, BufferStorageFlags flags) {
                 if (m_immutable) {
                     throw std::runtime_error("Buffer already has immutable storage");
                 }
        
                 if (m_mapped) {
                     throw std::runtime_error("Cannot allocate immutable storage while buffer is mapped");
                 }
        
                 m_size = size;
                 m_storageFlags = flags;
                 m_immutable = true;
        
                 // 根据标志设置适当的 usage 提示（用于回退到传统缓冲区）
                 if (flags & BufferStorageFlags::MapRead) {
                     m_usage = BufferUsage::StaticRead;
                 } else if (flags & BufferStorageFlags::MapWrite) {
                     m_usage = BufferUsage::DynamicDraw;
                 } else {
                     m_usage = BufferUsage::StaticDraw;
                 }
        
                 // 分配内存（实际的后端分配可能延迟到第一次使用）
                 m_data.resize(size);
        
                 MGLOG_D("BufferObject::AllocateImmutableStorage: size=%zu, flags=0x%x", 
                         size, static_cast<Uint32>(flags));
             }
    
             Bool IsImmutable() const { return m_immutable; }
             BufferStorageFlags GetStorageFlags() const { return m_storageFlags; }
             void SetImmutable(bool immutable) { m_immutable = immutable; }
    
             // 检查是否允许动态更新
             Bool AllowsDynamicUpdates() const {
                 return m_immutable && (m_storageFlags & BufferStorageFlags::DynamicStorage);
             }
    
             // 检查是否支持持久映射
             Bool SupportsPersistentMapping() const {
                 return m_immutable && (m_storageFlags & BufferStorageFlags::MapPersistent);
             }
    
             // 检查是否支持连贯映射
             Bool SupportsCoherentMapping() const {
                 return m_immutable && (m_storageFlags & BufferStorageFlags::MapCoherent);
             }
    
             // 检查是否支持读取映射
             Bool SupportsReadMapping() const {
                 return m_immutable && (m_storageFlags & BufferStorageFlags::MapRead);
             }
    
             // 检查是否支持写入映射
             Bool SupportsWriteMapping() const {
                 return m_immutable && (m_storageFlags & BufferStorageFlags::MapWrite);
             }

            private:
                const Uint m_externalIndex = 0;
                SizeT m_size = 0;
                BufferUsage m_usage = BufferUsage::StaticDraw;
                SharedPtr<Data> m_dataPtr;
                Bool m_isMapped;
                Flags<BufferMappingAccessBit> m_mappingAccess;
                Range1D m_dirtyRange;
                Range1D m_mappedRange;
                Vector<Uint8> m_stagingData;
                Bool m_ownsStagingData;

                Bool m_immutable = false;
                BufferStorageFlags m_storageFlags = BufferStorageFlags::None;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
