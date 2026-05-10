// MobileGL - MobileGL/MG_State/GLState/BufferState/BufferObject.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_Util/Math/VectorTypes.h>

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

    enum class BufferChangeBits : Uint8 {
        None = 0,
        DirtyBit = 1 << 0, // When not set, bits below are ignored and nothing should be synced to backend
        PreferReallocationBit =
            1 << 1, // <=> `glBufferData`; When set, ForbidInvalidationBit and ForbidUnsynchronizationBit are ignored
        ForbidInvalidationBit = 1 << 2, // Indidate that invalidation flags were not used during mapping, else we're
                                        // allowed to act as `GL_MAP_INVALIDATE_*` in backend
        ForbidUnsynchronizationBit = 1 << 3, // (the same description as above, but for unsynchronization)
    };

    struct BufferChange {
        static constexpr int DEFAULT_RESERVED_DIRTY_RANGES_COUNT = 50;

        Flags<BufferChangeBits> Bits = BufferChangeBits::None;
        VecRange1D DirtyRanges;
    };

    namespace MG_State::GLState {
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
            Range1D GetMappedRange() const;
            const SharedPtr<Data>& GetDataReadOnly() const;
            Flags<BufferMappingAccessBit> GetMappingAccess() const;
            Uint GetExternalIndex() const;
            const VecRange1D& GetDirtyRanges() const;
            Flags<BufferChangeBits> GetChangeBits() const;
            Uint64 GetChangeSerial() const;

        private:
            const Uint m_externalIndex = 0;
            SizeT m_size = 0;
            BufferUsage m_usage = BufferUsage::StaticDraw;
            SharedPtr<Data> m_dataPtr;
            Bool m_isMapped;
            Flags<BufferMappingAccessBit> m_mappingAccess;
            BufferChange m_change;
            Uint64 m_changeSerial = 0;
            Range1D m_mappedRange;
            Vector<Uint8> m_stagingData;
            Bool m_ownsStagingData;
        };
    } // namespace MG_State::GLState
} // namespace MobileGL
