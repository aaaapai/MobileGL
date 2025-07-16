#pragma once

namespace MobileGL {
	namespace MG_State {
		namespace GLState {
			class GLContext {
			public:
				GLContext() = default;

				// Buffer
				Vector<Uint> GenBuffersByNum(Uint number);
				SharedPtr<BufferObject> GetBufferObjByIndex(Uint index);
				SharedPtr<BufferObject> GetBufferObjByTarget(BufferTarget target);
				SharedPtr<BufferObject> CreateBufferObject(Uint index);
				void BindBufferTarget(BufferTarget target, SharedPtr<BufferObject> bufferObject);

			private:
				// Buffer
				BufferState bufferState_;
			};
		}
	}
}