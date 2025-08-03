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
                if (target == BufferTarget::Index) {
					return m_vertexArrayState.GetBoundVertexArray()->GetIndexBufferBindingSlot();
                }
                
                return m_bufferState.GetBindingSlot(target);
            }

            SharedPtr<BufferObject> GLContext::CreateBufferObject(Uint index) {
                return m_bufferState.CreateBufferObject(index);
            }

            void GLContext::MarkBufferObjectForDeletion(Uint index) {
                if (ValidateBufferObject(index)) {
					auto bufferObject = m_bufferState.GetBufferObject(index);
                    for (SizeT i = 0; i < m_vertexArrayState.GetAllVertexArrays().size(); ++i) {
                        auto vao = m_vertexArrayState.GetAllVertexArrays()[i];
                        if (vao->GetIndexBufferBindingSlot().GetBoundObject() == bufferObject) {
                            vao->GetIndexBufferBindingSlot().Bind(nullptr);
                        }
                        for (SizeT j = 0; j < VertexArrayObject::MAX_VERTEX_ATTRIBS; ++j) {
                            if (vao->GetAttribute(j).Buffer == bufferObject) {
                                vao->BindAttributeBuffer(j, nullptr);
                            }
                        }
                    }
                }

                m_bufferState.MarkBufferObjectForDeletion(index);
            }

            bool GLContext::ValidateBufferName(Uint index) const {
				return m_bufferState.ValidateName(index);
            }

            bool GLContext::ValidateBufferObject(Uint index) const {
				return m_bufferState.ValidateBufferObject(index);
            }

            // VertexArray
            Vector<Uint> GLContext::GenVertexArrayNames(Uint number) {
                return m_vertexArrayState.GenerateNames(number);
            }

            SharedPtr<VertexArrayObject> GLContext::GetVertexArrayObject(Uint index) {
                return m_vertexArrayState.GetVertexArrayObject(index);
            }

            void GLContext::BindVertexArray(Uint index) {
                m_vertexArrayState.Bind(index);
            }

            SharedPtr<VertexArrayObject> GLContext::CreateVertexArrayObject(Uint index) {
                return m_vertexArrayState.CreateVertexArrayObject(index);
            }

            void GLContext::MarkVertexArrayForDeletion(Uint index) {
                m_vertexArrayState.MarkVertexArrayForDeletion(index);
            }

            bool GLContext::ValidateVertexArrayName(Uint index) const {
                return m_vertexArrayState.ValidateName(index);
            }

            bool GLContext::ValidateVertexArrayObject(Uint index) const {
                return m_vertexArrayState.ValidateVertexArrayObject(index);
            }

            SharedPtr<VertexArrayObject> GLContext::GetBoundVertexArray() {
                return m_vertexArrayState.GetBoundVertexArray();
            }
        }

        GLState::GLContext* pGLContext;
    }
}