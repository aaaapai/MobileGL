// MobileGL - MobileGL/MG_State/GLState/BufferState/BufferObject.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "BufferObject.h"
#include "MG_Util/Types.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            BufferObject::BufferObject(Uint externalIndex)
                : m_externalIndex(externalIndex), m_size(0), m_usage(BufferUsage::StaticDraw), m_isMapped(false),
                  m_mappingAccess(BufferMappingAccessBit::Null), m_dirtyRange({0, 0}), m_mappedRange({0, 0}),
                  m_dataPtr(MakeShared<Data>()) {}

            void BufferObject::Resize(SizeT size) {
                m_size = size;
                m_dataPtr->reserve(std::bit_ceil(size)); // power-of-2 reserve
                m_dataPtr->resize(size);
                m_dirtyRange = {0, 0};
            }

            void BufferObject::UploadData(DataPtr data, SizeT atOffset) {
                MOBILEGL_ASSERT(atOffset + data.size <= m_size,
                                "UploadData out of bounds: atOffset (%zu) + data.size (%zu) > m_size (%zu)", atOffset,
                                data.size, m_size);
                MOBILEGL_ASSERT(!m_isMapped, "Cannot upload data while buffer is mapped.");
                Memcpy(m_dataPtr->data() + atOffset, data.data, data.size);
                m_dirtyRange.UnionUpdate(atOffset, atOffset + data.size);
            }

            void BufferObject::SetUsage(BufferUsage usage) {
                m_usage = usage;
            }

            void BufferObject::ReleaseMemory() {
                if (!m_isMapped) return;

                if (m_mappingAccess & BufferMappingAccessBit::Write) {                // if we wrote to the buffer
                    if (!(m_mappingAccess & BufferMappingAccessBit::FlushExplicit)) { // if we didn't flush explicitly
                        Memcpy(m_dataPtr->data() + m_mappedRange.start, m_stagingData.data(),
                               m_mappedRange.end - m_mappedRange.start);
                        m_dirtyRange.UnionUpdate(m_mappedRange.start, m_mappedRange.end);
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
                MOBILEGL_ASSERT(end <= m_mappedRange.end,
                                "Flush range out of bounds: mappedRange.end (%zu) < end (%zu)", m_mappedRange.end, end);

                Memcpy(m_dataPtr->data() + start, m_stagingData.data() + offset, length);
                m_dirtyRange.UnionUpdate(start, end);
            }

            void BufferObject::UploadSubData(DataPtr data, SizeT atOffset) {
                MOBILEGL_ASSERT(!m_isMapped, "Cannot upload sub data while buffer is mapped.");
                MOBILEGL_ASSERT(atOffset + data.size <= m_size,
                                "UploadSubData out of bounds: atOffset (%zu) + data.size (%zu) > m_size (%zu)",
                                atOffset, data.size, m_size);

                Memcpy(m_dataPtr->data() + atOffset, data.data, data.size);
                m_dirtyRange.UnionUpdate(atOffset, atOffset + data.size);
            }

            void BufferObject::CopyDataFrom(const SharedPtr<BufferObject>& src, SizeT srcOffset, SizeT dstOffset,
                                            SizeT size) {
                MOBILEGL_ASSERT(!m_isMapped, "Cannot copy data while buffer is mapped.");
                MOBILEGL_ASSERT(!src->IsMapped(), "Cannot copy data from a buffer that is mapped.");
                MOBILEGL_ASSERT(srcOffset + size <= src->GetSize(),
                                "Source buffer copy out of bounds: srcOffset (%zu) + size (%zu) > src->GetSize() (%zu)",
                                srcOffset, size, src->GetSize());
                MOBILEGL_ASSERT(dstOffset + size <= m_size,
                                "Destination buffer copy out of bounds: dstOffset (%zu) + size (%zu) > m_size (%zu)",
                                dstOffset, size, m_size);

                const Uint8* srcData = src->m_dataPtr->data() + srcOffset;
                Memcpy(m_dataPtr->data() + dstOffset, srcData, size);
                m_dirtyRange.UnionUpdate(dstOffset, dstOffset + size);
            }

            void* BufferObject::AcquireMemory(Bool markMapped, Bool read, Bool write) {
                if (markMapped) {
                    m_isMapped = true;
                    auto a = BufferMappingAccessBit::Coherent | BufferMappingAccessBit::Read;
                    m_mappingAccess = (read ? BufferMappingAccessBit::Read : BufferMappingAccessBit::Null) |
                                      (write ? BufferMappingAccessBit::Write : BufferMappingAccessBit::Null);
                    m_mappedRange = {0, m_size};

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

                if (access & BufferMappingAccessBit::Write) {
                    m_stagingData.resize(range.end - range.start);
                    m_ownsStagingData = true;

                    if (!(access &
                          (BufferMappingAccessBit::InvalidateRange | BufferMappingAccessBit::InvalidateBuffer))) {
                        Memcpy(m_stagingData.data(), m_dataPtr->data() + range.start, m_stagingData.size());
                    }

                    return m_stagingData.data();
                } else {
                    m_ownsStagingData = false;
                    return m_dataPtr->data() + range.start;
                }
            }

            const SharedPtr<Data> BufferObject::GetDataReadOnly() const {
                return m_dataPtr;
            }

            void BufferObject::ClearDirty() {
                m_dirtyRange = {0, 0};
            }

            SizeT BufferObject::GetSize() const {
                return m_size;
            }

            BufferUsage BufferObject::GetUsage() const {
                return m_usage;
            }

            Range1D BufferObject::GetDirtyRange() const {
                return m_dirtyRange;
            }

            Bool BufferObject::IsMapped() const {
                return m_isMapped;
            }

            Range1D BufferObject::GetMappedRange() const {
                return m_isMapped ? m_mappedRange : Range1D{0, 0};
            }

            Flags<BufferMappingAccessBit> BufferObject::GetMappingAccess() const {
                return m_isMapped ? m_mappingAccess : BufferMappingAccessBit::Null;
            }

            Uint BufferObject::GetExternalIndex() const {
                return m_externalIndex;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL