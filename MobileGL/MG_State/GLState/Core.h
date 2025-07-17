#pragma once

namespace MobileGL {
	namespace MG_State {
		namespace GLState {
			class GLContext {
			public:
				GLContext() = default;

				// Buffer
				Vector<Uint> GenBufferNames(Uint number);
				SharedPtr<BufferObject> GetBufferObject(Uint index);
				BindingSlot<BufferObject>& GetBufferBindingSort(BufferTarget target);
				SharedPtr<BufferObject> CreateBufferObject(Uint index);	

			private:
				// Buffer
				BufferState bufferState_;
			};
		}

		extern GLState::GLContext* pGLContext;
	}
}