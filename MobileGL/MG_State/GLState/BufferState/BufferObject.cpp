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
                assert(atOffset + data.size <= m_size);
                assert(!m_isMapped);
                memcpy(m_dataPtr->data() + atOffset, data.data, data.size);
                m_dirtyRange.UnionUpdate(atOffset, atOffset + data.size);
            }

            void BufferObject::SetUsage(BufferUsage usage) {
                m_usage = usage;
            }

            void BufferObject::ReleaseMemory() {
                if (!m_isMapped) return;

                if (m_mappingAccess & BufferMappingAccessBit::Write) {                // if we wrote to the buffer
                    if (!(m_mappingAccess & BufferMappingAccessBit::FlushExplicit)) { // if we didn't flush explicitly
                        memcpy(m_dataPtr->data() + m_mappedRange.start, m_stagingData.data(),
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
                assert(m_isMapped);
                assert((m_mappingAccess & BufferMappingAccessBit::FlushExplicit));
                assert((m_mappingAccess & BufferMappingAccessBit::Write));

                SizeT start = m_mappedRange.start + offset;
                SizeT end = start + length;
                assert(end <= m_mappedRange.end);

                memcpy(m_dataPtr->data() + start, m_stagingData.data() + offset, length);
                m_dirtyRange.UnionUpdate(start, end);
            }

            void BufferObject::UploadSubData(DataPtr data, SizeT atOffset) {
                assert(!m_isMapped);
                assert(atOffset + data.size <= m_size);

                memcpy(m_dataPtr->data() + atOffset, data.data, data.size);
                m_dirtyRange.UnionUpdate(atOffset, atOffset + data.size);
            }

            void BufferObject::CopyDataFrom(const SharedPtr<BufferObject>& src, SizeT srcOffset, SizeT dstOffset,
                                            SizeT size) {
                assert(!m_isMapped);
                assert(!src->IsMapped());
                assert(srcOffset + size <= src->GetSize());
                assert(dstOffset + size <= m_size);

                const Uint8* srcData = src->m_dataPtr->data() + srcOffset;
                memcpy(m_dataPtr->data() + dstOffset, srcData, size);
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
                            memcpy(m_stagingData.data(), m_dataPtr->data(), m_size);
                        }

                        return m_stagingData.data();
                    }
                }

                return m_dataPtr->data();
            }

            void* BufferObject::AcquireMemoryRange(Range1D range, Flags<BufferMappingAccessBit> access) {
                assert(range.end <= m_size && range.start <= range.end);
                m_isMapped = true;
                m_mappingAccess = access;
                m_mappedRange = range;

                if (access & BufferMappingAccessBit::Write) {
                    m_stagingData.resize(range.end - range.start);
                    m_ownsStagingData = true;

                    if (!(access &
                          (BufferMappingAccessBit::InvalidateRange | BufferMappingAccessBit::InvalidateBuffer))) {
                        memcpy(m_stagingData.data(), m_dataPtr->data() + range.start, m_stagingData.size());
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