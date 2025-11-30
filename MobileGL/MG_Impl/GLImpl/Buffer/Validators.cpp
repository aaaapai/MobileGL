#include "Validators.h"
#include <MG_State/GLState/Core.h>
#include <MG_State/GLState/ErrorState/Error.h>
#include <MG_Util/Converters/GLToStr/GLEnumConverter.h>
#include <MG_Util/Converters/MGToGL/BufferEnumConverter.h>
#include <MG_Util/Converters/MGToStr/BufferEnumConverter.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace BufferImpl {
        Bool ValidateBufferTarget(BufferTarget target) {
            if (target == BufferTarget::Unknown) {
                using namespace MG_Util;
                String bufferTargetStr = ConvertBufferTargetToString(target);
                String glTargetStr = ConvertGLEnumToString(ConvertBufferTargetToGLEnum(target));
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>(
                        "MG_Impl/GLImpl/BufferImpl", "ValidateBufferTarget",
                        std::format("Target {} ({}) is not valid.", bufferTargetStr, glTargetStr)));
                return false;
            }

            if (target == BufferTarget::Index && MG_State::pGLContext->GetBoundVertexArray() == nullptr) {
                MG_State::pGLContext->RecordError(ErrorCode::InvalidOperation,
                                                  MakeShared<GenericErrorInfo>("MG_Impl/GLImpl/BufferImpl",
                                                                               "ValidateBufferTarget",
                                                                               "No vertex array object is bound."));
                return false;
            }

            return true;
        }

        Bool ValidateBufferBindingPointTarget(BufferTarget target) {
            if (target != BufferTarget::Uniform && target != BufferTarget::AtomicCounter &&
                target != BufferTarget::TransformFeedback && target != BufferTarget::ShaderStorage) {
                using namespace MG_Util;
                String bufferTargetStr = ConvertBufferTargetToString(target);
                String glTargetStr = ConvertGLEnumToString(ConvertBufferTargetToGLEnum(target));
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>(
                        "MG_Impl/GLImpl/BufferImpl", "ValidateBufferTarget",
                        std::format("Target {} ({}) is not valid.", bufferTargetStr, glTargetStr)));
                return false;
            }
            return true;
        }

        Bool ValidateBufferName(Uint index, Bool allowZero) {
            if (index == 0) {
                if (allowZero) return true;

                MG_State::pGLContext->RecordError(ErrorCode::InvalidValue,
                                                  MakeShared<GenericErrorInfo>("MG_Impl/GLImpl/BufferImpl",
                                                                               "ValidateBufferName",
                                                                               "Buffer name 0 is not valid."));
                return false;
            }
            Bool isValid = MG_State::pGLContext->ValidateBufferName(index);
            if (isValid) return true;
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeShared<GenericErrorInfo>("MG_Impl/GLImpl/BufferImpl", "ValidateBufferName",
                                             std::format("Buffer name {} is not valid.", index)));
            return false;
        }

        Bool ValidateBufferUsage(BufferUsage usage) {
            if (usage != BufferUsage::Unknown) {
                return true;
            }
            using namespace MG_Util;
            String bufferUsageStr = ConvertBufferUsageToString(usage);
            String glUsageStr = ConvertGLEnumToString(ConvertBufferUsageToGLEnum(usage));
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidEnum,
                MakeShared<GenericErrorInfo>(
                    "MG_Impl/GLImpl/BufferImpl", "ValidateBufferUsage",
                    std::format("Usage {} ({}) is not one of the allowable values.", bufferUsageStr, glUsageStr)));
            return false;
        }

        Bool ValidateBufferMappingAccess(Flags<BufferMappingAccessBit> accessBits) {
            if (accessBits == BufferMappingAccessBit::Null) {
                MG_State::pGLContext->RecordError(ErrorCode::InvalidEnum,
                                                  MakeShared<GenericErrorInfo>("MG_Impl/GLImpl/BufferImpl",
                                                                               "ValidateBufferMappingAccess",
                                                                               "Access bits cannot be null."));
                return false;
            }

            const auto validBits = BufferMappingAccessBit::Read | BufferMappingAccessBit::Write |
                                   BufferMappingAccessBit::InvalidateRange | BufferMappingAccessBit::InvalidateBuffer |
                                   BufferMappingAccessBit::FlushExplicit | BufferMappingAccessBit::Unsynchronized |
                                   BufferMappingAccessBit::Persistent | BufferMappingAccessBit::Coherent;

            if ((accessBits & validBits) != accessBits) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl/BufferImpl", "ValidateBufferMappingAccess",
                                                 "Access bits cannot contain invalid flags."));
                return false;
            }

            return true;
        }
    } // namespace BufferImpl
} // namespace MobileGL::MG_Impl::GLImpl