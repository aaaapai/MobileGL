#pragma once

namespace MobileGL {
	namespace MG_State {
		namespace GLState {
			class GLContext {
			public:
				GLContext() = default;

				// Error
				void RecordError(ErrorCode code, SharedPtr<ErrorInfo> info = nullptr);
				bool HasGLError() const;
				Optional<const Error> PeekGLError() const;
				Optional<Error> PopGLError();
				bool HasNonGLError() const;
				Optional<const Error> PeekNonGLError() const;
				Optional<Error> PopNonGLError();
				void ClearErrors();

				// Buffer
				Vector<Uint> GenBufferNames(Uint number);
				SharedPtr<BufferObject> GetBufferObject(Uint index);
				BindingSlot<BufferObject>& GetBufferBindingSlot(BufferTarget target);
				SharedPtr<BufferObject> CreateBufferObject(Uint index);
				void MarkBufferObjectForDeletion(Uint index);
				bool ValidateBufferName(Uint index) const;
				bool ValidateBufferObject(Uint index) const;

			private:
				// Error
				ErrorState m_errorState;

				// Buffer
				BufferState m_bufferState;
			};
		}

		extern GLState::GLContext* pGLContext;
	}
}