#include "../../Includes.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            // Error
            void GLContext::RecordError(ErrorCode code, SharedPtr<ErrorInfo> info) {
                m_errorState.RecordError(code, info);
            }

            bool GLContext::HasGLError() const {
                return m_errorState.HasGLError();
            }

            Optional<const Error> GLContext::PeekGLError() const {
                return m_errorState.PeekGLError();
            }

            Optional<Error> GLContext::PopGLError() {
                return m_errorState.PopGLError();
            }

            bool GLContext::HasNonGLError() const {
                return m_errorState.HasNonGLError();
            }

            Optional<const Error> GLContext::PeekNonGLError() const {
                return m_errorState.PeekNonGLError();
            }

            Optional<Error> GLContext::PopNonGLError() {
                return m_errorState.PopNonGLError();
            }

            void GLContext::ClearErrors() {
                m_errorState.Clear();
            }

			// Buffer
            Vector<Uint> GLContext::GenBufferNames(Uint number) {
                return m_bufferState.GenerateNames(number);
            }

            SharedPtr<BufferObject> GLContext::GetBufferObject(Uint index) {
                return m_bufferState.GetBufferObject(index);
            }

            BindingSlot<BufferObject>& GLContext::GetBufferBindingSlot(BufferTarget target) {
                return m_bufferState.GetBindingSlot(target);
            }

            SharedPtr<BufferObject> GLContext::CreateBufferObject(Uint index) {
                return m_bufferState.CreateBufferObject(index);
            }

            void GLContext::MarkBufferObjectForDeletion(Uint index) {
                m_bufferState.MarkBufferObjectForDeletion(index);
            }

            bool GLContext::ValidateBufferName(Uint index) const {
				return m_bufferState.ValidateName(index);
            }

            bool GLContext::ValidateBufferObject(Uint index) const {
				return m_bufferState.ValidateBufferObject(index);
            }

        }

        GLState::GLContext* pGLContext;
    }
}