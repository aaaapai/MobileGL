#include "../../../Includes.h"

namespace MobileGL {
	namespace MG_State {
		namespace GLState {
			void BufferObject::UploadData(DataPtr data) {
				m_size = data.size;
				memcpy(m_data.data(), data.data, data.size);
				m_dirtyRange = MakeUnique<Range1D>(0, m_size);
				m_dirtyRange->Extend(0, m_size);
			}

			SharedPtr<Data> BufferObject::AquireMemory(Bool markMapped = true) {
				if (markMapped) {
					m_isMapped = true;
					m_mappingAccess = BufferMappingAccessBit::Read | BufferMappingAccessBit::Write;
				}
				return MakeShared<Data>(m_data);
			}

			SharedPtr<Data> BufferObject::AquireMemoryRange(Range1D range, BufferMappingAccessBit access) {
				m_isMapped = true;
				m_mappingAccess = access;
				return MakeShared<Data>(m_data);
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
				return *m_dirtyRange;
			}

			Bool BufferObject::IsMapped() const {
				return m_isMapped;
			}
		}
	}
}