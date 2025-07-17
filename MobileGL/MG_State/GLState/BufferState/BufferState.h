#pragma once

namespace MobileGL {
	namespace MG_State {
		namespace GLState {
			class BufferState {
			public:
				SharedPtr<BufferObject> GetBufferObject(Uint index);
				Vector<Uint> GenerateNames(Uint number);
				SharedPtr<BufferObject> CreateBufferObject(Uint index);
				BindingSlot<BufferObject>& GetBindingSlot(BufferTarget target);

			private:
				UnorderedMap<Uint, SharedPtr<BufferObject>> m_bufferObjects;
				IndexGenerator<Uint> m_indexGenerator;
				Vector<BindingSlot<BufferObject>> m_bindingSlots;
			};
		}
	}
}