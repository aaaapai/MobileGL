// MobileGL - MobileGL/MG_Impl/GLImpl/VertexArray/GL_VertexArray.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "GL_VertexArray.h"
#include "Validators.h"
#include <MG_Impl/GLImpl/Buffer/Validators.h>
#include <MG_State/GLState/Core.h>
#include <MG_State/GLState/ErrorState/Error.h>
#include <MG_Util/Converters/GLToMG/DataTypeConverter.h>

namespace MobileGL::MG_Impl::GLImpl {
    SharedPtr<MG_State::GLState::VertexArrayObject> GetNamedVertexArrayObject_State(GLuint vaobj,
                                                                                   const char* caller) {
        if (!VertexArrayImpl::ValidateVertexArrayName(vaobj)) return nullptr;
        if (!VertexArrayImpl::ValidateVertexArrayObject(vaobj)) return nullptr;
        return MG_State::pGLContext->GetVertexArrayObject(vaobj);
    }

    SharedPtr<MG_State::GLState::BufferObject> GetVertexArrayBufferObject_State(GLuint buffer, const char* caller) {
        if (!BufferImpl::ValidateBufferName(buffer, true)) return nullptr;
        if (buffer == 0) return nullptr;

        auto& bufferObject = MG_State::pGLContext->GetBufferObject(buffer);
        if (!bufferObject) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", caller,
                                             std::format("Buffer object {} does not exist.", buffer)));
            return nullptr;
        }
        return bufferObject;
    }

    void DisableVertexAttribArray_State(GLuint index) {
        if (!VertexArrayImpl::ValidateVertexAttributeIndex(index)) return;

        auto& vao = MG_State::pGLContext->GetBoundVertexArray();
        if (!vao) {
            MG_State::pGLContext->RecordError(ErrorCode::InvalidOperation,
                                              MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl",
                                                                           "EnableVertexAttribArray_State",
                                                                           "No vertex array object is bound."));
            return;
        }

        vao->DisableAttribute(index);
    }

    void EnableVertexAttribArray_State(GLuint index) {
        if (!VertexArrayImpl::ValidateVertexAttributeIndex(index)) return;

        auto& vao = MG_State::pGLContext->GetBoundVertexArray();
        if (!vao) {
            MG_State::pGLContext->RecordError(ErrorCode::InvalidOperation,
                                              MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl",
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

        auto& vao = MG_State::pGLContext->GetBoundVertexArray();
        if (!vao) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation, MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "VertexAttribPointer_State",
                                                                          "No vertex array object is bound."));
            return;
        }

        auto& vboSlot = MG_State::pGLContext->GetBufferBindingSlot(BufferTarget::Vertex);
        auto& vbo = vboSlot.GetBoundObject();

        auto offset = reinterpret_cast<SizeT>(pointer);

        vao->SetAttributeFormat(index, size, dataType, false, stride, offset, true);
        vao->BindAttributeBuffer(index, vbo);
    }

    // TODO: implement GL_BGRA support
    void VertexAttribPointer_State(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
                                   const void* pointer) {
        if (!VertexArrayImpl::ValidateVertexAttributeIndex(index)) return;

        DataType dataType = MG_Util::ConvertGLEnumToDataType(type);
        if (!VertexArrayImpl::ValidateVertexAttribPointerParams(index, size, dataType, stride)) return;

        auto& vao = MG_State::pGLContext->GetBoundVertexArray();
        if (!vao) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation, MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "VertexAttribPointer_State",
                                                                          "No vertex array object is bound."));
            return;
        }

        auto& vboSlot = MG_State::pGLContext->GetBufferBindingSlot(BufferTarget::Vertex);
        auto& vbo = vboSlot.GetBoundObject();

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
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "DeleteVertexArrays_State", "n must be non-negative."));
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
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "GenVertexArrays_State", "n must be non-negative."));
            return;
        }

        Vector<Uint> vaos;
        MG_State::pGLContext->GenVertexArrayNames(n, vaos);
        Memcpy(arrays, vaos.data(), n * sizeof(GLuint));
    }

    void CreateVertexArrays_State(GLsizei n, GLuint* arrays) {
        if (n < 0) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "CreateVertexArrays_State", "n must be non-negative."));
            return;
        }

        Vector<Uint> vaos;
        MG_State::pGLContext->GenVertexArrayNames(n, vaos);
        for (GLsizei i = 0; i < n; ++i) {
            MG_State::pGLContext->CreateVertexArrayObject(vaos[i]);
            arrays[i] = vaos[i];
        }
    }

    void DisableVertexArrayAttrib_State(GLuint vaobj, GLuint index) {
        auto vao = GetNamedVertexArrayObject_State(vaobj, "DisableVertexArrayAttrib_State");
        if (!vao) return;
        if (!VertexArrayImpl::ValidateVertexAttributeIndex(index)) return;
        vao->DisableAttribute(index);
    }

    void EnableVertexArrayAttrib_State(GLuint vaobj, GLuint index) {
        auto vao = GetNamedVertexArrayObject_State(vaobj, "EnableVertexArrayAttrib_State");
        if (!vao) return;
        if (!VertexArrayImpl::ValidateVertexAttributeIndex(index)) return;
        vao->EnableAttribute(index);
    }

    void VertexArrayElementBuffer_State(GLuint vaobj, GLuint buffer) {
        auto vao = GetNamedVertexArrayObject_State(vaobj, "VertexArrayElementBuffer_State");
        if (!vao) return;
        auto bufferObject = GetVertexArrayBufferObject_State(buffer, "VertexArrayElementBuffer_State");
        if (buffer != 0 && !bufferObject) return;
        vao->GetIndexBufferBindingSlot().Bind(bufferObject);
    }

    void VertexArrayVertexBuffer_State(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset,
                                       GLsizei stride) {
        auto vao = GetNamedVertexArrayObject_State(vaobj, "VertexArrayVertexBuffer_State");
        if (!vao) return;
        if (!VertexArrayImpl::ValidateVertexAttributeIndex(bindingindex)) return;
        if (offset < 0 || stride < 0) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "VertexArrayVertexBuffer_State",
                                             "offset and stride must be non-negative."));
            return;
        }
        auto bufferObject = GetVertexArrayBufferObject_State(buffer, "VertexArrayVertexBuffer_State");
        if (buffer != 0 && !bufferObject) return;

        const auto& attr = vao->GetAttribute(bindingindex);
        vao->SetAttributeFormat(bindingindex, attr.Size, attr.Type, attr.Normalized, stride, static_cast<SizeT>(offset),
                                attr.IsInteger);
        vao->BindAttributeBuffer(bindingindex, bufferObject);
    }

    void VertexArrayAttribFormat_State(GLuint vaobj, GLuint attribindex, GLint size, GLenum type,
                                       GLboolean normalized, GLuint relativeoffset) {
        auto vao = GetNamedVertexArrayObject_State(vaobj, "VertexArrayAttribFormat_State");
        if (!vao) return;
        if (!VertexArrayImpl::ValidateVertexAttributeIndex(attribindex)) return;

        DataType dataType = MG_Util::ConvertGLEnumToDataType(type);
        const auto& attr = vao->GetAttribute(attribindex);
        if (!VertexArrayImpl::ValidateVertexAttribPointerParams(attribindex, size, dataType, attr.Stride)) return;

        vao->SetAttributeFormat(attribindex, size, dataType, normalized, attr.Stride, relativeoffset, false);
    }

    void VertexArrayAttribIFormat_State(GLuint vaobj, GLuint attribindex, GLint size, GLenum type,
                                        GLuint relativeoffset) {
        auto vao = GetNamedVertexArrayObject_State(vaobj, "VertexArrayAttribIFormat_State");
        if (!vao) return;
        if (!VertexArrayImpl::ValidateVertexAttributeIndex(attribindex)) return;

        DataType dataType = MG_Util::ConvertGLEnumToDataType(type);
        const auto& attr = vao->GetAttribute(attribindex);
        if (!VertexArrayImpl::ValidateVertexAttribPointerParams(attribindex, size, dataType, attr.Stride)) return;

        vao->SetAttributeFormat(attribindex, size, dataType, false, attr.Stride, relativeoffset, true);
    }

    GLboolean IsVertexArray_State(GLuint array) {
        if (array == 0) return GL_FALSE;
        return MG_State::pGLContext->ValidateVertexArrayObject(array) ? GL_TRUE : GL_FALSE;
    }

    void VertexAttribDivisor_State(GLuint index, GLuint divisor) {
        if (!VertexArrayImpl::ValidateVertexAttributeIndex(index)) return;

        auto& vao = MG_State::pGLContext->GetBoundVertexArray();
        if (!vao) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation, MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "VertexAttribDivisor_State",
                                                                          "No vertex array object is bound."));
            return;
        }

        vao->SetAttributeDivisor(index, divisor);
    }

    /* @INSERTION_POINT:FUNCTION_IMPLEMENTATION@ */
    void CreateVertexArrays(GLsizei n, GLuint* arrays) {
        CreateVertexArrays_State(n, arrays);
    }

    void DisableVertexArrayAttrib(GLuint vaobj, GLuint index) {
        DisableVertexArrayAttrib_State(vaobj, index);
    }

    void EnableVertexArrayAttrib(GLuint vaobj, GLuint index) {
        EnableVertexArrayAttrib_State(vaobj, index);
    }

    void VertexArrayElementBuffer(GLuint vaobj, GLuint buffer) {
        VertexArrayElementBuffer_State(vaobj, buffer);
    }

    void VertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) {
        VertexArrayVertexBuffer_State(vaobj, bindingindex, buffer, offset, stride);
    }

    void VertexArrayAttribFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized,
                                 GLuint relativeoffset) {
        VertexArrayAttribFormat_State(vaobj, attribindex, size, type, normalized, relativeoffset);
    }

    void VertexArrayAttribIFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) {
        VertexArrayAttribIFormat_State(vaobj, attribindex, size, type, relativeoffset);
    }

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
} // namespace MobileGL::MG_Impl::GLImpl
