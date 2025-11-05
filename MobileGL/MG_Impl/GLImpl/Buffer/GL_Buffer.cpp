#include "GL_Buffer.h"
#include "Validators.h"
#include "MG_Util/Converters/GLToStr/GLEnumConverter.h"
#include <MG_State/GLState/Core.h>
#include <MG_State/GLState/ErrorState/Error.h>
#include <MG_Util/Converters/GLToMG/BufferEnumConverter.h>
#include <MG_Util/Converters/MGToGL/BufferEnumConverter.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        void GetBufferParameteriv_State(GLenum target, GLenum pname, GLint* params) {
            if (!params) {
                MG_State::pGLContext->RecordError(ErrorCode::InvalidValue,
                                                  MakeShared<GenericErrorInfo>("MG_Impl/GLImpl",
                                                                               "GetBufferParameteriv_State",
                                                                               "Params pointer cannot be null."));
                return;
            }

            BufferTarget bufferTarget = MG_Util::ConvertGLEnumToBufferTarget(target);
            if (!BufferImpl::ValidateBufferTarget(bufferTarget)) return;

            auto& bindingSlot = MG_State::pGLContext->GetBufferBindingSlot(bufferTarget);

            auto bufferObject = bindingSlot.GetBoundObject();
            if (!bufferObject) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "GetBufferParameteriv_State",
                                                 "Buffer target is bound to no buffer object."));
                return;
            }

            switch (pname) {
            case GL_BUFFER_SIZE:
                *params = static_cast<GLint>(bufferObject->GetSize());
                break;
            case GL_BUFFER_USAGE:
                *params = MG_Util::ConvertBufferUsageToGLEnum(bufferObject->GetUsage());
                break;
            case GL_BUFFER_ACCESS:
                if (bufferObject->IsMapped()) {
                    auto access = bufferObject->GetMappingAccess();
                    if (access & BufferMappingAccessBit::Read && access & BufferMappingAccessBit::Write) {
                        *params = GL_READ_WRITE;
                    } else if (access & BufferMappingAccessBit::Read) {
                        *params = GL_READ_ONLY;
                    } else if (access & BufferMappingAccessBit::Write) {
                        *params = GL_WRITE_ONLY;
                    } else {
                        *params = 0;
                    }
                } else {
                    *params = 0;
                }
                break;
            case GL_BUFFER_MAPPED:
                *params = bufferObject->IsMapped() ? GL_TRUE : GL_FALSE;
                break;
            }
        }

        void DeleteBuffers_State(GLsizei n, const GLuint* buffers) {
            if (n < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "DeleteBuffers_State", "n must be non-negative."));
                return;
            }

            if (!buffers) {
                MG_State::pGLContext->RecordError(ErrorCode::InvalidValue,
                                                  MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "DeleteBuffers_State",
                                                                               "Buffer names array cannot be null."));
                return;
            }

            for (SizeT i = 0; i < static_cast<SizeT>(n); ++i) {
                Uint bufferName = buffers[i];
                if (bufferName == 0) continue;
                if (!BufferImpl::ValidateBufferName(bufferName, true)) continue;
                MG_State::pGLContext->MarkBufferObjectForDeletion(bufferName);
            }
        }

        void FlushMappedBufferRange_State(GLenum target, GLintptr offset, GLsizeiptr length) {
            if (length < 0 || offset < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "FlushMappedBufferRange_State",
                                                 "Offset and length must be non-negative."));
                return;
            }

            BufferTarget bufferTarget = MG_Util::ConvertGLEnumToBufferTarget(target);
            if (!BufferImpl::ValidateBufferTarget(bufferTarget)) return;

            auto& bindingSlot = MG_State::pGLContext->GetBufferBindingSlot(bufferTarget);

            auto bufferObject = bindingSlot.GetBoundObject();
            if (!bufferObject) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "FlushMappedBufferRange_State",
                                                 "Buffer target is bound to no buffer object."));
                return;
            }

            if (!bufferObject->IsMapped()) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "FlushMappedBufferRange_State",
                                                 "Cannot flush a buffer object that is not mapped."));
                return;
            }

            if (offset + length > bufferObject->GetSize()) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "FlushMappedBufferRange_State",
                                                 "Offset and length exceed buffer size."));
                return;
            }

            auto mappingAccess = bufferObject->GetMappingAccess();
            if (!(mappingAccess & BufferMappingAccessBit::FlushExplicit)) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>(
                        "MG_Impl/GLImpl", "FlushMappedBufferRange_State",
                        "Cannot flush a buffer object that is not mapped with GL_MAP_FLUSH_EXPLICIT_BIT."));
                return;
            }

            bufferObject->FlushMemoryRange(static_cast<SizeT>(offset), static_cast<SizeT>(length));
        }

        GLboolean UnmapBuffer_State(GLenum target) {
            BufferTarget bufferTarget = MG_Util::ConvertGLEnumToBufferTarget(target);
            if (!BufferImpl::ValidateBufferTarget(bufferTarget)) return GL_FALSE;

            auto& bindingSlot = MG_State::pGLContext->GetBufferBindingSlot(bufferTarget);

            auto bufferObject = bindingSlot.GetBoundObject();
            if (!bufferObject) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "UnmapBuffer_State",
                                                 "Buffer target is bound to no buffer object."));
                return GL_FALSE;
            }

            if (!bufferObject->IsMapped()) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "UnmapBuffer_State",
                                                 "Cannot unmap a buffer object that is not mapped."));
                return GL_FALSE;
            }

            bufferObject->ReleaseMemory();
            return GL_TRUE;
        }

        void* MapBufferRange_State(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {
            if (length < 0 || offset < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "MapBufferRange_State",
                                                                          "Offset and length must be non-negative."));
                return nullptr;
            }

            if (length == 0) {
                MG_State::pGLContext->RecordError(ErrorCode::InvalidOperation,
                                                  MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "MapBufferRange_State",
                                                                               "Length must be greater than zero."));
                return nullptr;
            }

            BufferTarget bufferTarget = MG_Util::ConvertGLEnumToBufferTarget(target);
            if (!BufferImpl::ValidateBufferTarget(bufferTarget)) return nullptr;

            auto& bindingSlot = MG_State::pGLContext->GetBufferBindingSlot(bufferTarget);
            auto bufferObject = bindingSlot.GetBoundObject();
            if (!bufferObject) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "MapBufferRange_State",
                                                 "Buffer target is bound to no buffer object."));
                return nullptr;
            }

            if (offset + length > bufferObject->GetSize()) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "MapBufferRange_State",
                                                                          "Offset and length exceed buffer size."));
                return nullptr;
            }

            auto accessBits = MG_Util::ConvertGLEnumToBufferMappingAccess(access);
            if (!BufferImpl::ValidateBufferMappingAccess(accessBits)) return nullptr;

            if (!(accessBits & (BufferMappingAccessBit::Read | BufferMappingAccessBit::Write))) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "MapBufferRange_State",
                                                 "At least one of GL_MAP_READ_BIT or GL_MAP_WRITE_BIT must be set."));
                return nullptr;
            }

            if (accessBits & BufferMappingAccessBit::Read) {
                const auto invalidFlags = BufferMappingAccessBit::InvalidateRange |
                                          BufferMappingAccessBit::InvalidateBuffer |
                                          BufferMappingAccessBit::Unsynchronized;

                if (accessBits & invalidFlags) {
                    MG_State::pGLContext->RecordError(
                        ErrorCode::InvalidOperation,
                        MakeShared<GenericErrorInfo>(
                            "MG_Impl/GLImpl", "MapBufferRange_State",
                            "GL_MAP_READ_BIT cannot be combined with invalidation or unsynchronized flags."));
                    return nullptr;
                }
            }

            if (accessBits & BufferMappingAccessBit::FlushExplicit) {
                if (!(accessBits & BufferMappingAccessBit::Write)) {
                    MG_State::pGLContext->RecordError(
                        ErrorCode::InvalidOperation,
                        MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "MapBufferRange_State",
                                                     "GL_MAP_FLUSH_EXPLICIT_BIT requires GL_MAP_WRITE_BIT."));
                    return nullptr;
                }
            }

            const auto storageFlags = BufferMappingAccessBit::Persistent | BufferMappingAccessBit::Coherent;
            auto requiredFlags = accessBits & storageFlags;
            if (requiredFlags) {
                // TODO: check if the buffer data is created by BufferStorage and its flags after its
                // implementation
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "MapBufferRange_State",
                                                 "Access flags require matching storage flags in buffer."));
                return nullptr;
            }

            if (bufferObject->IsMapped()) {
                const auto invalidateFlags =
                    BufferMappingAccessBit::InvalidateRange | BufferMappingAccessBit::InvalidateBuffer;

                if (!(accessBits & invalidateFlags)) {
                    MG_State::pGLContext->RecordError(
                        ErrorCode::InvalidOperation,
                        MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "MapBufferRange_State",
                                                     "Cannot map a buffer object that is already mapped."));
                    return nullptr;
                }
            }

            void* result = bufferObject->AcquireMemoryRange(
                {static_cast<SizeT>(offset), static_cast<SizeT>(offset + length)}, accessBits);
            if (!result) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::OutOfMemory,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "MapBufferRange_State",
                                                 "Failed to map buffer due to insufficient memory."));
                return nullptr;
            }
            return result;
        }

        void* MapBuffer_State(GLenum target, GLenum access) {
            Bool readable = access == GL_READ_ONLY || access == GL_READ_WRITE;
            Bool writable = access == GL_WRITE_ONLY || access == GL_READ_WRITE;
            if (access != GL_READ_ONLY && access != GL_WRITE_ONLY && access != GL_READ_WRITE) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>(
                        "MG_Impl/GLImpl", "MapBuffer_State",
                        "Access must be one of GL_READ_ONLY, GL_WRITE_ONLY, or GL_READ_WRITE."));
                return nullptr;
            }

            BufferTarget bufferTarget = MG_Util::ConvertGLEnumToBufferTarget(target);
            if (!BufferImpl::ValidateBufferTarget(bufferTarget)) return nullptr;
            auto& bindingSlot = MG_State::pGLContext->GetBufferBindingSlot(bufferTarget);

            auto bufferObject = bindingSlot.GetBoundObject();
            if (!bufferObject) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "MapBuffer_State",
                                                 "Buffer target is bound to no buffer object."));
                return nullptr;
            }

            if (bufferObject->IsMapped()) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "MapBuffer_State",
                                                 "Cannot map a buffer object that is already mapped."));
                return nullptr;
            }

            void* result = bufferObject->AcquireMemory(true, readable, writable);
            if (!result) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::OutOfMemory,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "MapBuffer_State",
                                                 "Failed to map buffer due to insufficient memory."));
                return nullptr;
            }
            return result;
        }

        void CopyBufferSubData_State(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset,
                                     GLsizeiptr size) {
            if (size < 0 || readOffset < 0 || writeOffset < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "CopyBufferSubData_State",
                                                                          "Offset and size must be non-negative."));
                return;
            }

            BufferTarget readBufferTarget = MG_Util::ConvertGLEnumToBufferTarget(readTarget);
            BufferTarget writeBufferTarget = MG_Util::ConvertGLEnumToBufferTarget(writeTarget);
            if (!BufferImpl::ValidateBufferTarget(readBufferTarget) ||
                !BufferImpl::ValidateBufferTarget(writeBufferTarget))
                return;

            auto& readBindingSlot = MG_State::pGLContext->GetBufferBindingSlot(readBufferTarget);
            auto& writeBindingSlot = MG_State::pGLContext->GetBufferBindingSlot(writeBufferTarget);
            auto readBufferObject = readBindingSlot.GetBoundObject();
            auto writeBufferObject = writeBindingSlot.GetBoundObject();

            if (!readBufferObject || !writeBufferObject) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "CopyBufferSubData_State",
                                                 "One of the buffer targets is bound to no buffer object."));
                return;
            }

            if (readOffset + size > readBufferObject->GetSize() || writeOffset + size > writeBufferObject->GetSize()) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "CopyBufferSubData_State",
                                                 "Offset and size must be within the bounds of the buffer objects."));
                return;
            }

            if (readBufferObject == writeBufferObject) {
                if ((readOffset <= writeOffset && readOffset + size > writeOffset) ||
                    (writeOffset <= readOffset && writeOffset + size > readOffset)) {
                    MG_State::pGLContext->RecordError(
                        ErrorCode::InvalidValue,
                        MakeShared<GenericErrorInfo>(
                            "MG_Impl/GLImpl", "CopyBufferSubData_State",
                            "Source and destination buffers overlap in the specified ranges."));
                    return;
                }
            }

            auto isIllegallyMapped = [](const SharedPtr<MG_State::GLState::BufferObject>& buffer) {
                return buffer->IsMapped() && !(buffer->GetMappingAccess() & BufferMappingAccessBit::Persistent);
            };
            if (isIllegallyMapped(readBufferObject) || isIllegallyMapped(writeBufferObject)) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "CopyBufferSubData_State",
                                                 "Cannot copy data from/to a mapped buffer object unless it was mapped "
                                                 "with GL_MAP_PERSISTENT_BIT."));
                return;
            }

            writeBufferObject->CopyDataFrom(readBufferObject, readOffset, writeOffset, size);
        }

        void BufferSubData_State(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) {
            if (!data) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::NoError, // somehow OpenGL does not generate an error for this
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "BufferSubData_State",
                                                 "Data pointer cannot be null."));
                return;
            }

            if (size < 0 || offset < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "BufferSubData_State",
                                                                          "Offset and size must be non-negative."));
                return;
            }

            BufferTarget bufferTarget = MG_Util::ConvertGLEnumToBufferTarget(target);
            if (!BufferImpl::ValidateBufferTarget(bufferTarget)) return;
            auto& bindingSlot = MG_State::pGLContext->GetBufferBindingSlot(bufferTarget);

            auto bufferObject = bindingSlot.GetBoundObject();
            if (!bufferObject) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "BufferSubData_State",
                                                 "Buffer target is bound to no buffer object."));
                return;
            }

            SizeT bufferSize = bufferObject->GetSize();
            Range1D mappedRange = bufferObject->GetMappedRange();
            if ((offset < mappedRange.end) && (offset + size > mappedRange.start)) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>(
                        "MG_Impl/GLImpl", "BufferSubData_State",
                        "Offset and size must not overlap with the mapped range of the buffer object."));
                return;
            }

            auto mappingAccess = bufferObject->GetMappingAccess();
            if (bufferObject->IsMapped() && !(mappingAccess & BufferMappingAccessBit::Persistent)) {
                Range1D mappedRange = bufferObject->GetMappedRange();
                if (offset + size >= mappedRange.start) {
                    MG_State::pGLContext->RecordError(
                        ErrorCode::InvalidOperation,
                        MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "BufferSubData_State",
                                                     "Cannot modify a mapped buffer object unless it was "
                                                     "mapped with GL_MAP_PERSISTENT_BIT."));
                    return;
                }
            }

            bufferObject->UploadSubData({(void*)data, (SizeT)size}, offset);
        }

        void BufferData_State(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
            MGLOG_D("%s: %s, size = %d, data = %p, usage = %s",
                    __func__, MG_Util::ConvertGLEnumToString(target).c_str(), size, data, MG_Util::ConvertGLEnumToString(usage).c_str());
            if (size < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "BufferData_State", "Size must be non-negative."));
                return;
            }

            BufferUsage bufferUsage = MG_Util::ConvertGLEnumToBufferUsage(usage);
            if (!BufferImpl::ValidateBufferUsage(bufferUsage)) return;

            BufferTarget bufferTarget = MG_Util::ConvertGLEnumToBufferTarget(target);
            if (!BufferImpl::ValidateBufferTarget(bufferTarget)) return;
            auto& bindingSlot = MG_State::pGLContext->GetBufferBindingSlot(bufferTarget);

            auto bufferObject = bindingSlot.GetBoundObject();
            if (!bufferObject) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "BufferData_State",
                                                 "Buffer target is bound to no buffer object."));
                return;
            }

            bufferObject->SetUsage(bufferUsage);
            bufferObject->Resize(size);
            if (data) {
                bufferObject->UploadData({(void*)data, (SizeT)size}, 0);
            }
        }

        void BindBuffer_State(GLenum target, GLuint buffer) {
            if (!BufferImpl::ValidateBufferName(buffer, true)) return;
            BufferTarget bufferTarget = MG_Util::ConvertGLEnumToBufferTarget(target);
            if (!BufferImpl::ValidateBufferTarget(bufferTarget)) return;

            auto bufferObject = MG_State::pGLContext->GetBufferObject(buffer);
            if (!bufferObject) {
                MG_State::pGLContext->CreateBufferObject(buffer);
                bufferObject = MG_State::pGLContext->GetBufferObject(buffer);
            }

            auto& bindingSlot = MG_State::pGLContext->GetBufferBindingSlot(bufferTarget);
            bindingSlot.Bind(bufferObject);
            MGLOG_D("%s: bind buffer object %p -> %s", __func__, bufferObject.get(), MG_Util::ConvertGLEnumToString(target).c_str());
        }

        void GenBuffers_State(GLsizei n, GLuint* buffers) {
            if (n < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "GenBuffers_State", "n must be non-negative"));
                return;
            }
            auto bufferNames = MG_State::pGLContext->GenBufferNames(n);
            Copy(bufferNames.data(), buffers, bufferNames.size());
        }

        GLboolean IsBuffer_State(GLuint buffer) {
            if (!BufferImpl::ValidateBufferName(buffer)) return GL_FALSE;
            return MG_State::pGLContext->ValidateBufferObject(buffer) ? GL_TRUE : GL_FALSE;
        }

        void BindBufferBase_State(GLenum target, GLuint pointIndex, GLuint buffer) {
            BufferTarget bufferTarget = MG_Util::ConvertGLEnumToBufferTarget(target);
            if (!BufferImpl::ValidateBufferBindingPointTarget(bufferTarget)) return;

            auto bufferObject = MG_State::pGLContext->GetBufferObject(buffer);
            if (!bufferObject) {
                MG_State::pGLContext->CreateBufferObject(buffer);
                bufferObject = MG_State::pGLContext->GetBufferObject(buffer);
            }

            auto& point = MG_State::pGLContext->GetBufferBindingPoint(bufferTarget, pointIndex);
            point.Bind(bufferObject);
            point.ClearRange();
        }

        void BindBufferRange_State(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
            BufferTarget bufferTarget = MG_Util::ConvertGLEnumToBufferTarget(target);
            if (!BufferImpl::ValidateBufferBindingPointTarget(bufferTarget)) return;

            auto bufferObject = MG_State::pGLContext->GetBufferObject(buffer);
            if (!bufferObject) {
                MG_State::pGLContext->CreateBufferObject(buffer);
                bufferObject = MG_State::pGLContext->GetBufferObject(buffer);
            }

            auto& point = MG_State::pGLContext->GetBufferBindingPoint(bufferTarget, index);
            point.Bind(bufferObject);
            point.SetRange(Range1D(offset, offset + size));
        }

        /* @INSERTION_POINT:FUNCTION_IMPLEMENTATION@ */
        void GetBufferParameteriv(GLenum target, GLenum pname, GLint* params) {
            GetBufferParameteriv_State(target, pname, params);
        }

        GLboolean IsBuffer(GLuint buffer) {
            return IsBuffer_State(buffer);
        }

        void DeleteBuffers(GLsizei n, const GLuint* buffers) {
            DeleteBuffers_State(n, buffers);
        }

        void FlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length) {
            FlushMappedBufferRange_State(target, offset, length);
        }

        GLboolean UnmapBuffer(GLenum target) {
            return UnmapBuffer_State(target);
        }

        void* MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {
            return MapBufferRange_State(target, offset, length, access);
        }

        void* MapBuffer(GLenum target, GLenum access) {
            return MapBuffer_State(target, access);
        }

        void CopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset,
                               GLsizeiptr size) {
            CopyBufferSubData_State(readTarget, writeTarget, readOffset, writeOffset, size);
        }

        void BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) {
            BufferSubData_State(target, offset, size, data);
        }

        void BufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
            BufferData_State(target, size, data, usage);
        }

        void BindBuffer(GLenum target, GLuint buffer) {
            BindBuffer_State(target, buffer);
        }

        void GenBuffers(GLsizei n, GLuint* buffers) {
            GenBuffers_State(n, buffers);
        }

        void BindBufferBase(GLenum target, GLuint index, GLuint buffer) {
            BindBufferBase_State(target, index, buffer);
        }

        void BindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
            BindBufferRange_State(target, index, buffer, offset, size);
        }
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
