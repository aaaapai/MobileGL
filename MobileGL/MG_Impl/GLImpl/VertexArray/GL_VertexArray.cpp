#include "GL_VertexArray.h"
#include "Validators.h"
#include <MG_State/GLState/Core.h>
#include <MG_State/GLState/ErrorState/Error.h>
#include <MG_Util/Converters/GLToMG/DataTypeConverter.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        void DisableVertexAttribArray_State(GLuint index) {
            if (!VertexArrayImpl::ValidateVertexAttributeIndex(index)) return;

            auto vao = MG_State::pGLContext->GetBoundVertexArray();
            if (!vao) {
                MG_State::pGLContext->RecordError(ErrorCode::InvalidOperation,
                                                  MakeShared<GenericErrorInfo>("MG_Impl/GLImpl",
                                                                               "EnableVertexAttribArray_State",
                                                                               "No vertex array object is bound."));
                return;
            }

            vao->DisableAttribute(index);
        }

        void EnableVertexAttribArray_State(GLuint index) {
            if (!VertexArrayImpl::ValidateVertexAttributeIndex(index)) return;

            auto vao = MG_State::pGLContext->GetBoundVertexArray();
            if (!vao) {
                MG_State::pGLContext->RecordError(ErrorCode::InvalidOperation,
                                                  MakeShared<GenericErrorInfo>("MG_Impl/GLImpl",
                                                                               "EnableVertexAttribArray_State",
                                                                               "No vertex array object is bound."));
                return;
            }

            vao->EnableAttribute(index);
        }

        // TODO: implement GL_BGRA support
        void VertexAttribIPointer_State(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) {
            if (!VertexArrayImpl::ValidateVertexAttributeIndex(index)) return;

            DataType dataType = MG_Util::ConvertGLEnumToDataType(type);
            if (!VertexArrayImpl::ValidateVertexAttribPointerParams(index, size, dataType, stride)) return;

            auto vao = MG_State::pGLContext->GetBoundVertexArray();
            if (!vao) {
                MG_State::pGLContext->RecordError(ErrorCode::InvalidOperation,
                                                  MakeShared<GenericErrorInfo>("MG_Impl/GLImpl",
                                                                               "VertexAttribPointer_State",
                                                                               "No vertex array object is bound."));
                return;
            }

            auto& vboSlot = MG_State::pGLContext->GetBufferBindingSlot(BufferTarget::Vertex);
            auto vbo = vboSlot.GetBoundObject();
            if (!vbo) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "VertexAttribPointer_State",
                                                 "No buffer is bound to GL_ARRAY_BUFFER."));
                return;
            }

            SizeT offset = reinterpret_cast<SizeT>(pointer);

            vao->SetAttributeFormat(index, size, dataType, false, stride, offset, true);
            vao->BindAttributeBuffer(index, vbo);
        }

        // TODO: implement GL_BGRA support
        void VertexAttribPointer_State(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
                                       const void* pointer) {
            if (!VertexArrayImpl::ValidateVertexAttributeIndex(index)) return;

            DataType dataType = MG_Util::ConvertGLEnumToDataType(type);
            if (!VertexArrayImpl::ValidateVertexAttribPointerParams(index, size, dataType, stride)) return;

            auto vao = MG_State::pGLContext->GetBoundVertexArray();
            if (!vao) {
                MG_State::pGLContext->RecordError(ErrorCode::InvalidOperation,
                                                  MakeShared<GenericErrorInfo>("MG_Impl/GLImpl",
                                                                               "VertexAttribPointer_State",
                                                                               "No vertex array object is bound."));
                return;
            }

            auto& vboSlot = MG_State::pGLContext->GetBufferBindingSlot(BufferTarget::Vertex);
            auto vbo = vboSlot.GetBoundObject();
            if (!vbo) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "VertexAttribPointer_State",
                                                 "No buffer is bound to GL_ARRAY_BUFFER."));
                return;
            }

            SizeT offset = reinterpret_cast<SizeT>(pointer);

            vao->SetAttributeFormat(index, size, dataType, normalized, stride, offset, false);
            vao->BindAttributeBuffer(index, vbo);
        }

        void BindVertexArray_State(GLuint array) {
            if (array == 0) {
                MG_State::pGLContext->BindVertexArray(0);
                return;
            }

            if (!VertexArrayImpl::ValidateVertexArrayName(array)) return;

            if (!MG_State::pGLContext->ValidateVertexArrayObject(array)) {
                MG_State::pGLContext->CreateVertexArrayObject(array);
            }

            MG_State::pGLContext->BindVertexArray(array);
        }

        void DeleteVertexArrays_State(GLsizei n, const GLuint* arrays) {
            if (n < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "DeleteVertexArrays_State",
                                                                          "n must be non-negative."));
                return;
            }

            for (GLsizei i = 0; i < n; ++i) {
                GLuint vao = arrays[i];
                if (vao == 0) continue;

                if (!VertexArrayImpl::ValidateVertexArrayName(vao)) continue;

                if (MG_State::pGLContext->GetBoundVertexArray() &&
                    MG_State::pGLContext->GetBoundVertexArray() == MG_State::pGLContext->GetVertexArrayObject(vao)) {
                    MG_State::pGLContext->BindVertexArray(0);
                }

                MG_State::pGLContext->MarkVertexArrayForDeletion(vao);
            }
        }

        void GenVertexArrays_State(GLsizei n, GLuint* arrays) {
            if (n < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "GenVertexArrays_State", "n must be non-negative."));
                return;
            }

            auto vaoNames = MG_State::pGLContext->GenVertexArrayNames(n);
            Copy(vaoNames.data(), arrays, vaoNames.size());
        }

        GLboolean IsVertexArray_State(GLuint array) {
            if (array == 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "IsVertexArray_State",
                                                                          "Vertex array name 0 is not supported."));
                return GL_FALSE;
            }
            if (!VertexArrayImpl::ValidateVertexArrayName(array)) return GL_FALSE;
            if (!MG_State::pGLContext->ValidateVertexArrayObject(array)) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "IsVertexArray_State",
                                                 std::format("Vertex array object {} does not exist.", array)));
                return GL_FALSE;
            }
            return GL_TRUE;
        }

        void VertexAttribDivisor_State(GLuint index, GLuint divisor) {
            if (!VertexArrayImpl::ValidateVertexAttributeIndex(index)) return;

            auto vao = MG_State::pGLContext->GetBoundVertexArray();
            if (!vao) {
                MG_State::pGLContext->RecordError(ErrorCode::InvalidOperation,
                                                  MakeShared<GenericErrorInfo>("MG_Impl/GLImpl",
                                                                               "VertexAttribDivisor_State",
                                                                               "No vertex array object is bound."));
                return;
            }

            vao->SetAttributeDivisor(index, divisor);
        }

        /* @INSERTION_POINT:FUNCTION_IMPLEMENTATION@ */
        void VertexAttribDivisor(GLuint index, GLuint divisor) {
            VertexAttribDivisor_State(index, divisor);
        }

        GLboolean IsVertexArray(GLuint array) {
            return IsVertexArray_State(array);
        }

        void DisableVertexAttribArray(GLuint index) {
            DisableVertexAttribArray_State(index);
        }

        void EnableVertexAttribArray(GLuint index) {
            EnableVertexAttribArray_State(index);
        }

        void VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) {
            VertexAttribIPointer_State(index, size, type, stride, pointer);
        }

        void VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
                                 const void* pointer) {
            VertexAttribPointer_State(index, size, type, normalized, stride, pointer);
        }

        void BindVertexArray(GLuint array) {
            BindVertexArray_State(array);
        }

        void DeleteVertexArrays(GLsizei n, const GLuint* arrays) {
            DeleteVertexArrays_State(n, arrays);
        }

        void GenVertexArrays(GLsizei n, GLuint* arrays) {
            GenVertexArrays_State(n, arrays);
        }
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
