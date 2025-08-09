#pragma once
#include <Includes.h>
#include "BufferState/BufferState.h"
#include "VertexArrayState/VertexArrayState.h"
#include "ErrorState/Error.h"
#include "ProgramState/ProgramState.h"

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

                // VertexArray
                Vector<Uint> GenVertexArrayNames(Uint number);
                SharedPtr<VertexArrayObject> GetVertexArrayObject(Uint index);
                void BindVertexArray(Uint index);
                SharedPtr<VertexArrayObject> CreateVertexArrayObject(Uint index);
                void MarkVertexArrayForDeletion(Uint index);
                bool ValidateVertexArrayName(Uint index) const;
                bool ValidateVertexArrayObject(Uint index) const;
                SharedPtr<VertexArrayObject> GetBoundVertexArray();

            private:
                // Error
                ErrorState m_errorState;

                // Buffer
                BufferState m_bufferState;

                // VertexArray
                VertexArrayState m_vertexArrayState;
            };
        } // namespace GLState

        extern GLState::GLContext* pGLContext;
    } // namespace MG_State
} // namespace MobileGL