#include "../../../Includes.h"

namespace MobileGL {
	namespace MG_State {
		namespace GLState {
			// BufferObject
			void BufferObject::Resize(SizeT size) {
				m_size = size;
				// Always reserve to power-of-2 size to amortize reallocation cost
				m_data.reserve(std::bit_ceil(size));
				m_data.resize(size);
				// no dirty range == discard all data
				m_dirtyRange.Update(0, 0);
			}

			void BufferObject::UploadData(DataPtr data, SizeT atOffset) {
				assert(atOffset + data.size <= m_size);
				// m_size = data.size;
				memcpy(m_data.data() + atOffset, data.data, data.size);
				// m_dirtyRange = MakeUnique<Range1D>(0, m_size);
                m_dirtyRange.UnionUpdate(atOffset, atOffset + data.size - 1);
			}

			void* BufferObject::AcquireMemory(Bool markMapped, Bool read, Bool write) {
				if (markMapped) {
					m_isMapped = true;
					m_mappingAccess = (read ? BufferMappingAccessBit::Read : BufferMappingAccessBit::Null) |
						(write ? BufferMappingAccessBit::Write : BufferMappingAccessBit::Null);
				}
				return m_data.data();
			}

			void* BufferObject::AcquireMemoryRange(Range1D range, BufferMappingAccessBit access) {
				m_isMapped = true;
				m_mappingAccess = access;
				return m_data.data() + range.start;
			}

			void BufferObject::SetUsage(BufferUsage usage) {
				m_usage = usage;
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
				for (auto& slot : m_bindingSlots) {
					if (slot.GetTarget() == target) {
						return slot;
					}
				}
				m_bindingSlots.emplace_back(target);
				return m_bindingSlots.back();
			}
		}
	}
}