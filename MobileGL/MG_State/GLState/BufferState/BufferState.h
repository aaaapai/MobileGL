#pragma once

namespace MobileGL {
	namespace MG_State {
		namespace GLState {
			class BufferState {
			public:
                BufferState() {
                    for (SizeT i = 0; i < (SizeT)BufferTarget::BufferTargetCount; ++i) {
                        m_bindingSlots[i] = BindingSlot<BufferObject>((BufferTarget)i);
                    }
                }

				SharedPtr<BufferObject> GetBufferObject(Uint index);
				Vector<Uint> GenerateNames(Uint number);
				SharedPtr<BufferObject> CreateBufferObject(Uint index);
				BindingSlot<BufferObject>& GetBindingSlot(BufferTarget target);

			private:
				UnorderedMap<Uint, SharedPtr<BufferObject>> m_bufferObjects;
				IndexGenerator<Uint> m_indexGenerator;
				Array<BindingSlot<BufferObject>, (SizeT)BufferTarget::BufferTargetCount> m_bindingSlots;
			};
		}
	}
}