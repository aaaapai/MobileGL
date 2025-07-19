#include "../../../Includes.h"

namespace MobileGL {
	namespace MG_State {
		namespace GLState {
			// BufferObject
			BufferObject::BufferObject()
				: m_id(0), m_size(0), m_usage(BufferUsage::StaticDraw),
				m_isMapped(false), m_mappingAccess(BufferMappingAccessBit::Null),
				m_dirtyRange({ 0, 0 }), m_mappedRange({ 0, 0 }) {
			}

			void BufferObject::Resize(SizeT size) {
				m_size = size;
				m_data.reserve(std::bit_ceil(size)); // power-of-2 reserve
				m_data.resize(size);
				m_dirtyRange.Update(0, 0);
			}

			void BufferObject::UploadData(DataPtr data, SizeT atOffset) {
				assert(atOffset + data.size <= m_size);
				assert(!m_isMapped);
				memcpy(m_data.data() + atOffset, data.data, data.size);
				m_dirtyRange.UnionUpdate(atOffset, atOffset + data.size);
			}

			void BufferObject::SetUsage(BufferUsage usage) {
				m_usage = usage;
			}

			void BufferObject::ReleaseMemory() {
				if (!m_isMapped) return;

				if (any(m_mappingAccess & BufferMappingAccessBit::Write)) { // if we wrote to the buffer
					if (!any(m_mappingAccess & BufferMappingAccessBit::FlushExplicit)) { // if we didn't flush explicitly
						memcpy(m_data.data() + m_mappedRange.start,
							m_stagingData.data(),
							m_mappedRange.end - m_mappedRange.start);
						m_dirtyRange.UnionUpdate(m_mappedRange.start, m_mappedRange.end);
					}

					m_stagingData.clear();
					m_stagingData.shrink_to_fit();
				}

				m_isMapped = false;
				m_mappingAccess = BufferMappingAccessBit::Null;
				m_mappedRange = { 0, 0 };
				m_ownsStagingData = false;
			}

			void BufferObject::FlushMemoryRange(SizeT offset, SizeT length) {
				assert(m_isMapped);
				assert(any(m_mappingAccess & BufferMappingAccessBit::FlushExplicit));
				assert(any(m_mappingAccess & BufferMappingAccessBit::Write));

				SizeT start = m_mappedRange.start + offset;
				SizeT end = start + length;
				assert(end <= m_mappedRange.end);

				memcpy(m_data.data() + start, m_stagingData.data() + offset, length);
				m_dirtyRange.UnionUpdate(start, end);
			}

			void BufferObject::UploadSubData(SizeT offset, SizeT size, const void* data) {
				assert(!m_isMapped);
				assert(offset + size <= m_size);

				memcpy(m_data.data() + offset, data, size);
				m_dirtyRange.UnionUpdate(offset, offset + size);
			}

			void BufferObject::CopyDataFrom(const SharedPtr<BufferObject>& src, SizeT srcOffset, SizeT dstOffset, SizeT size) {
				assert(!m_isMapped);
				assert(!src->IsMapped());
				assert(srcOffset + size <= src->GetSize());
				assert(dstOffset + size <= m_size);

				const Uint8* srcData = src->m_data.data() + srcOffset;
				memcpy(m_data.data() + dstOffset, srcData, size);
				m_dirtyRange.UnionUpdate(dstOffset, dstOffset + size);
			}

			void* BufferObject::AcquireMemory(Bool markMapped, Bool read, Bool write) {
				if (markMapped) {
					m_isMapped = true;
					m_mappingAccess =
						(read ? BufferMappingAccessBit::Read : BufferMappingAccessBit::Null) |
						(write ? BufferMappingAccessBit::Write : BufferMappingAccessBit::Null);
					m_mappedRange = { 0, m_size };

					if (any(m_mappingAccess & BufferMappingAccessBit::Write)) {
						m_stagingData.resize(m_size);
						m_ownsStagingData = true;

						if (!any(m_mappingAccess & (BufferMappingAccessBit::InvalidateRange |
							BufferMappingAccessBit::InvalidateBuffer))) {
							memcpy(m_stagingData.data(), m_data.data(), m_size);
						}

						return m_stagingData.data();
					}
				}

				return m_data.data();
			}

			void* BufferObject::AcquireMemoryRange(Range1D range, BufferMappingAccessBit access) {
				assert(range.end <= m_size && range.start <= range.end);
				m_isMapped = true;
				m_mappingAccess = access;
				m_mappedRange = range;

				if (any(access & BufferMappingAccessBit::Write)) {
					m_stagingData.resize(range.end - range.start);
					m_ownsStagingData = true;

					if (!any(access & (BufferMappingAccessBit::InvalidateRange |
						BufferMappingAccessBit::InvalidateBuffer))) {
						memcpy(m_stagingData.data(), m_data.data() + range.start, m_stagingData.size());
					}

					return m_stagingData.data();
				}
				else {
					m_ownsStagingData = false;
					return m_data.data() + range.start;
				}
			}

			void BufferObject::ClearDirty() {
				m_dirtyRange = { 0, 0 };
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

			// BufferState
			SharedPtr<BufferObject> BufferState::GetBufferObject(Uint index) {
				auto it = m_bufferObjects.find(index);
				if (it != m_bufferObjects.end()) {
					return it->second;
				}
				return nullptr;
			}

			Vector<Uint> BufferState::GenerateNames(Uint number) {
				Vector<Uint> buffers(number);
				m_indexGenerator.Generate(number, buffers.data());
				return buffers;
			}

			SharedPtr<BufferObject> BufferState::CreateBufferObject(Uint index) {
				auto bufferObject = MakeShared<BufferObject>();
				m_bufferObjects[index] = bufferObject;
				return bufferObject;
			}

			BindingSlot<BufferObject>& BufferState::GetBindingSlot(BufferTarget target) {
				return m_bindingSlots[(SizeT)target];
			}
		}
	}
}