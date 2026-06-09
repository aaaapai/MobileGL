// MobileGL - MobileGL/MG_State/GLState/BufferState/BufferObject.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "BufferObject.h"

namespace MobileGL::MG_State::GLState {
    BufferObject::BufferObject(Uint externalIndex)
        : m_externalIndex(externalIndex), m_size(0), m_usage(BufferUsage::StaticDraw), m_isMapped(false),
          m_mappingAccess(BufferMappingAccessBit::Null),
          m_change(BufferChangeBits::DirtyBit | BufferChangeBits::PreferReallocationBit), m_mappedRange({0, 0}),
          m_dataPtr(MakeShared<Data>()), m_ownsStagingData{} {
        m_change.DirtyRanges.reserve(BufferChange::DEFAULT_RESERVED_DIRTY_RANGES_COUNT);
    }

    void BufferObject::Resize(SizeT size) {
        ReleaseMemory();
        m_size = size;
        m_dataPtr->reserve(std::bit_ceil(size)); // power-of-2 reserve
        m_dataPtr->resize(size);
        m_isImmutableStorage = false;
        m_storageFlags = 0;
        m_change.Bits |= BufferChangeBits::DirtyBit;
        m_change.Bits |= BufferChangeBits::PreferReallocationBit;
        ++m_changeSerial;
    }

    void BufferObject::AllocateImmutableStorage(SizeT size, const void* data, GLbitfield storageFlags) {
        ReleaseMemory();
        m_size = size;
        m_dataPtr->reserve(std::bit_ceil(size));
        m_dataPtr->resize(size);
        if (data) {
            Memcpy(m_dataPtr->data(), data, size);
        } else if (size > 0) {
            Memset(m_dataPtr->data(), 0, size);
        }
        m_isImmutableStorage = true;
        m_storageFlags = storageFlags;
        m_change.DirtyRanges.clear();
        m_change.DirtyRanges.Add({0, size});
        m_change.Bits = BufferChangeBits::DirtyBit | BufferChangeBits::PreferReallocationBit;
        ++m_changeSerial;
    }

    void BufferObject::UploadData(DataPtr data, SizeT atOffset) {
        MOBILEGL_ASSERT(atOffset + data.size <= m_size,
                        "UploadData out of bounds: atOffset (%zu) + data.size (%zu) > m_size (%zu)", atOffset,
                        data.size, m_size);
        MOBILEGL_ASSERT(!m_isMapped || (m_mappingAccess & BufferMappingAccessBit::Persistent),
                        "Cannot upload data while buffer is non-persistently mapped.");
        Memcpy(m_dataPtr->data() + atOffset, data.data, data.size);
        m_change.DirtyRanges.Add({atOffset, atOffset + data.size});
        m_change.Bits |= BufferChangeBits::DirtyBit;
        m_change.Bits |= BufferChangeBits::ForbidInvalidationBit;
        m_change.Bits |= BufferChangeBits::ForbidUnsynchronizationBit;
        ++m_changeSerial;
        // This function may be called by `glBufferData`, but we still set the forbid bits above,
        // because when `PreferReallocationBit` is set, those bits are ignored anyway.
        // The bits can fit the `glBufferSubData` semantics
        // (though `glBufferSubData` calls `UploadSubData` instead).
    }

    void BufferObject::SetUsage(BufferUsage usage) {
        m_usage = usage;
    }

    void BufferObject::ReleaseMemory() {
        if (!m_isMapped) return;

        if (m_mappingAccess & BufferMappingAccessBit::Write) {                // if we wrote to the buffer
            if (!(m_mappingAccess & BufferMappingAccessBit::FlushExplicit)) { // if we didn't flush explicitly
                if (!(m_mappingAccess & BufferMappingAccessBit::Persistent)) {
                    Memcpy(m_dataPtr->data() + m_mappedRange.start, m_stagingData.data(),
                           m_mappedRange.end - m_mappedRange.start);
                }
                m_change.DirtyRanges.Add({m_mappedRange.start, m_mappedRange.end});
                m_change.Bits |= BufferChangeBits::DirtyBit;
                ++m_changeSerial;
            }

            m_stagingData.clear();
        }

        m_isMapped = false;
        m_mappingAccess = BufferMappingAccessBit::Null;
        m_mappedRange = {0, 0};
        m_ownsStagingData = false;
    }

    void BufferObject::FlushMemoryRange(SizeT offset, SizeT length) {
        MOBILEGL_ASSERT(m_isMapped, "Buffer must be mapped to flush memory range.");
        MOBILEGL_ASSERT((m_mappingAccess & BufferMappingAccessBit::FlushExplicit),
                        "Buffer must be mapped with FlushExplicit access to flush memory range.");
        MOBILEGL_ASSERT((m_mappingAccess & BufferMappingAccessBit::Write),
                        "Buffer must be mapped with Write access to flush memory range.");

        SizeT start = m_mappedRange.start + offset;
        SizeT end = start + length;
        MOBILEGL_ASSERT(end <= m_mappedRange.end, "Flush range out of bounds: mappedRange.end (%zu) < end (%zu)",
                        m_mappedRange.end, end);

        if (!(m_mappingAccess & BufferMappingAccessBit::Persistent)) {
            Memcpy(m_dataPtr->data() + start, m_stagingData.data() + offset, length);
        }
        m_change.DirtyRanges.Add({start, end});
        m_change.Bits |= BufferChangeBits::DirtyBit;
        ++m_changeSerial;
    }

    void BufferObject::MarkPersistentMappedRangeDirty() {
        if (!m_isMapped) return;
        if (!(m_mappingAccess & BufferMappingAccessBit::Persistent)) return;
        if (!(m_mappingAccess & BufferMappingAccessBit::Write)) return;
        if (m_mappingAccess & BufferMappingAccessBit::FlushExplicit) return;
        if (m_mappedRange.start >= m_mappedRange.end) return;

        m_change.DirtyRanges.Add(m_mappedRange);
        m_change.Bits |= BufferChangeBits::DirtyBit;
        ++m_changeSerial;
    }

    void BufferObject::UploadSubData(DataPtr data, SizeT atOffset) {
        MOBILEGL_ASSERT(!m_isMapped || (m_mappingAccess & BufferMappingAccessBit::Persistent),
                        "Cannot upload sub data while buffer is non-persistently mapped.");
        MOBILEGL_ASSERT(atOffset + data.size <= m_size,
                        "UploadSubData out of bounds: atOffset (%zu) + data.size (%zu) > m_size (%zu)", atOffset,
                        data.size, m_size);

        Memcpy(m_dataPtr->data() + atOffset, data.data, data.size);
        m_change.DirtyRanges.Add({atOffset, atOffset + data.size});
        m_change.Bits |= BufferChangeBits::DirtyBit;
        m_change.Bits |= BufferChangeBits::ForbidInvalidationBit;
        m_change.Bits |= BufferChangeBits::ForbidUnsynchronizationBit;
        ++m_changeSerial;
    }

    void BufferObject::CopyDataFrom(const SharedPtr<BufferObject>& src, SizeT srcOffset, SizeT dstOffset, SizeT size) {
        MOBILEGL_ASSERT(!m_isMapped || (m_mappingAccess & BufferMappingAccessBit::Persistent),
                        "Cannot copy data while destination buffer is non-persistently mapped.");
        MOBILEGL_ASSERT(!src->IsMapped() || (src->GetMappingAccess() & BufferMappingAccessBit::Persistent),
                        "Cannot copy data from a buffer that is non-persistently mapped.");
        MOBILEGL_ASSERT(srcOffset + size <= src->GetSize(),
                        "Source buffer copy out of bounds: srcOffset (%zu) + size (%zu) > src->GetSize() (%zu)",
                        srcOffset, size, src->GetSize());
        MOBILEGL_ASSERT(dstOffset + size <= m_size,
                        "Destination buffer copy out of bounds: dstOffset (%zu) + size (%zu) > m_size (%zu)", dstOffset,
                        size, m_size);

        const Uint8* srcData = src->m_dataPtr->data() + srcOffset;
        Memcpy(m_dataPtr->data() + dstOffset, srcData, size);
        m_change.DirtyRanges.Add({dstOffset, dstOffset + size});
        m_change.Bits |= BufferChangeBits::DirtyBit;
        ++m_changeSerial;
    }

    void* BufferObject::AcquireMemory(Bool markMapped, Bool read, Bool write) {
        if (markMapped) {
            m_isMapped = true;
            m_mappingAccess = (read ? BufferMappingAccessBit::Read : BufferMappingAccessBit::Null) |
                              (write ? BufferMappingAccessBit::Write : BufferMappingAccessBit::Null);
            m_mappedRange = {0, m_size};
            if (write) {
                m_change.Bits |= BufferChangeBits::ForbidInvalidationBit;
                m_change.Bits |= BufferChangeBits::ForbidUnsynchronizationBit;
            }

            if (m_mappingAccess & BufferMappingAccessBit::Write) {
                m_stagingData.resize(m_size);
                m_ownsStagingData = true;

                if (!(m_mappingAccess &
                      (BufferMappingAccessBit::InvalidateRange | BufferMappingAccessBit::InvalidateBuffer))) {
                    Memcpy(m_stagingData.data(), m_dataPtr->data(), m_size);
                }

                return m_stagingData.data();
            }
        }

        return m_dataPtr->data();
    }

    void* BufferObject::AcquireMemoryRange(Range1D range, Flags<BufferMappingAccessBit> access) {
        MOBILEGL_ASSERT(range.end <= m_size && range.start <= range.end,
                        "AcquireMemoryRange out of bounds: range (%zu, %zu) exceeds m_size (%zu)", range.start,
                        range.end, m_size);
        m_isMapped = true;
        m_mappingAccess = access;
        m_mappedRange = range;
        m_change.Bits |=
            !(access & BufferMappingAccessBit::InvalidateBuffer || access & BufferMappingAccessBit::InvalidateRange)
                ? BufferChangeBits::ForbidInvalidationBit
                : BufferChangeBits::None;
        m_change.Bits |= !(access & BufferMappingAccessBit::Unsynchronized)
                             ? BufferChangeBits::ForbidUnsynchronizationBit
                             : BufferChangeBits::None;

        m_change.Bits |=
            !(access & BufferMappingAccessBit::InvalidateBuffer || access & BufferMappingAccessBit::InvalidateRange)
                ? BufferChangeBits::ForbidInvalidationBit
                : BufferChangeBits::None;
        m_change.Bits |= !(access & BufferMappingAccessBit::Unsynchronized)
                             ? BufferChangeBits::ForbidUnsynchronizationBit
                             : BufferChangeBits::None;

        if (access & BufferMappingAccessBit::Persistent) {
            m_ownsStagingData = false;
            return m_dataPtr->data() + range.start;
        }

        if (access & BufferMappingAccessBit::Write) {
            m_stagingData.resize(range.end - range.start);
            m_ownsStagingData = true;

            if (!(access & (BufferMappingAccessBit::InvalidateRange | BufferMappingAccessBit::InvalidateBuffer))) {
                Memcpy(m_stagingData.data(), m_dataPtr->data() + range.start, m_stagingData.size());
            }

            return m_stagingData.data();
        } else {
            m_ownsStagingData = false;
            return m_dataPtr->data() + range.start;
        }
    }

    const SharedPtr<Data>& BufferObject::GetDataReadOnly() const {
        return m_dataPtr;
    }

    void BufferObject::ClearDirty() {
        m_change.DirtyRanges.clear();
        m_change.Bits = BufferChangeBits::None;
    }

    SizeT BufferObject::GetSize() const {
        return m_size;
    }

    Bool BufferObject::IsImmutableStorage() const {
        return m_isImmutableStorage;
    }

    BufferUsage BufferObject::GetUsage() const {
        return m_usage;
    }

    const VecRange1D& BufferObject::GetDirtyRanges() const {
        return m_change.DirtyRanges;
    }

    Flags<BufferChangeBits> BufferObject::GetChangeBits() const {
        return m_change.Bits;
    }

    Uint64 BufferObject::GetChangeSerial() const {
        return m_changeSerial;
    }

    Bool BufferObject::IsMapped() const {
        return m_isMapped;
    }

    Range1D BufferObject::GetMappedRange() const {
        return m_isMapped ? m_mappedRange : Range1D{0, 0};
    }

    void* BufferObject::GetMappedPointer() const {
        if (!m_isMapped) return nullptr;
        if (m_mappingAccess & BufferMappingAccessBit::Persistent) {
            return const_cast<Uint8*>(m_dataPtr->data()) + m_mappedRange.start;
        }
        if (m_ownsStagingData) {
            return const_cast<Uint8*>(m_stagingData.data());
        }
        return const_cast<Uint8*>(m_dataPtr->data()) + m_mappedRange.start;
    }

    Flags<BufferMappingAccessBit> BufferObject::GetMappingAccess() const {
        return m_isMapped ? m_mappingAccess : BufferMappingAccessBit::Null;
    }

    GLbitfield BufferObject::GetStorageFlags() const {
        return m_storageFlags;
    }

    Uint BufferObject::GetExternalIndex() const {
        return m_externalIndex;
    }
} // namespace MobileGL::MG_State::GLState
