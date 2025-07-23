#pragma once

namespace MobileGL {
	namespace MG_State {
		namespace GLState {
			class BufferState {
			public:
				BufferState();

				SharedPtr<BufferObject> GetBufferObject(Uint index);
				Vector<Uint> GenerateNames(Uint number);
				SharedPtr<BufferObject> CreateBufferObject(Uint index);
				BindingSlot<BufferObject>& GetBindingSlot(BufferTarget target);
				void MarkBufferObjectForDeletion(Uint index);
				bool ValidateName(Uint index) const;
				bool ValidateBufferObject(Uint index) const;

			private:
				UnorderedMap<Uint, SharedPtr<BufferObject>> m_bufferObjects;
				IndexGenerator<Uint> m_indexGenerator;
				Array<BindingSlot<BufferObject>, (SizeT)BufferTarget::BufferTargetCount> m_bindingSlots;
			};
		}
	}
}