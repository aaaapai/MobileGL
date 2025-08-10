#pragma once
#include <Includes.h>
#include "BufferState/BufferState.h"
#include "MG_Util/Types.h"
#include "VertexArrayState/VertexArrayState.h"
#include "ErrorState/Error.h"
#include "ProgramState/ProgramState.h"
#include "TextureState/TextureState.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class GLContext {
            public:
                GLContext() = default;

                // Error
                void RecordError(ErrorCode code, SharedPtr<ErrorInfo> info = nullptr);
                Bool HasGLError() const;
                Optional<const Error> PeekGLError() const;
                Optional<Error> PopGLError();
                Bool HasNonGLError() const;
                Optional<const Error> PeekNonGLError() const;
                Optional<Error> PopNonGLError();
                void ClearErrors();

                // Buffer
                Vector<Uint> GenBufferNames(Uint number);
                SharedPtr<BufferObject> GetBufferObject(Uint index);
                BindingSlot<BufferObject>& GetBufferBindingSlot(BufferTarget target);
                SharedPtr<BufferObject> CreateBufferObject(Uint index);
                void MarkBufferObjectForDeletion(Uint index);
                Bool ValidateBufferName(Uint index) const;
                Bool ValidateBufferObject(Uint index) const;

                // VertexArray
                Vector<Uint> GenVertexArrayNames(Uint number);
                SharedPtr<VertexArrayObject> GetVertexArrayObject(Uint index);
                void BindVertexArray(Uint index);
                SharedPtr<VertexArrayObject> CreateVertexArrayObject(Uint index);
                void MarkVertexArrayForDeletion(Uint index);
                Bool ValidateVertexArrayName(Uint index) const;
                Bool ValidateVertexArrayObject(Uint index) const;
                SharedPtr<VertexArrayObject> GetBoundVertexArray();

                // Texture
                Vector<Uint> GenTextureNames(Uint number);
                SharedPtr<ITextureObject> GetTextureObject(Uint index);
                SharedPtr<ITextureObject> CreateTextureObject(Uint index, TextureTarget target);
                void MarkTextureObjectForDeletion(Uint index);
                TextureUnit& GetTextureUnitObject();
                Bool ValidateTextureName(Uint index) const;
                Bool ValidateTextureObject(Uint index) const;
                Int GetActiveTextureUnit() const;
                void SetActiveTextureUnit(Int unit);

            private:
                // Error
                ErrorState m_errorState;

                // Buffer
                BufferState m_bufferState;

                // VertexArray
                VertexArrayState m_vertexArrayState;

                // Texture
                TextureState m_textureState;
            };
        } // namespace GLState

        extern GLState::GLContext* pGLContext;
    } // namespace MG_State
} // namespace MobileGL