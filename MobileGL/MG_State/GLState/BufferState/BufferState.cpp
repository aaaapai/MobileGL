#include "../../../Includes.h"

namespace MobileGL {
	namespace MG_State {
		namespace GLState {
			BufferState::BufferState()
				: m_indexGenerator(1024, 1) {
				for (SizeT i = 0; i < m_bindingSlots.size(); ++i) {
					m_bindingSlots[i] = BindingSlot<BufferObject>(GlobalBufferTargets[i]);
				}
			}

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
				for (SizeT i = 0; i < m_bindingSlots.size(); ++i) {
					if (m_bindingSlots[i].GetTarget() == target) {
						return m_bindingSlots[i];
					}
				}
			}

			void BufferState::MarkBufferObjectForDeletion(Uint index) {
				if (m_indexGenerator.IsValid(index)) {
					auto it = m_bufferObjects.find(index);
					if (it != m_bufferObjects.end()) {
						for (SizeT i = 0; i < m_bindingSlots.size(); ++i) {
							if (m_bindingSlots[i].GetBoundObject() == it->second) {
								m_bindingSlots[i].Bind(nullptr);
							}
						}
						m_bufferObjects.erase(it);
					}
					m_indexGenerator.Delete(index);
				}
			}

			bool BufferState::ValidateName(Uint index) const {
				return m_indexGenerator.IsValid(index);
			}

			bool BufferState::ValidateBufferObject(Uint index) const {
				return m_bufferObjects.find(index) != m_bufferObjects.end();
			}
		}
	}
}