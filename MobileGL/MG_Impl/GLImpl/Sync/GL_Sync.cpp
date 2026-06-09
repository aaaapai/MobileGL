// MobileGL - MobileGL/MG_Impl/GLImpl/Sync/GL_Sync.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "GL_Sync.h"

#include "MG_State/GLState/Core.h"
#include <MG_Backend/BackendObjects.h>
#include <MG_Backend/DirectGLES/BackendObject_DirectGLES.h>
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>
#include <MG_State/GLState/ErrorState/Error.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace {
        const MG_External::GLESFunctionsTable* TryGetDirectGLESFunctions(const char* funcName) {
            auto* activeBackend = MG_Backend::pActiveBackendObject.get();
            if (!activeBackend) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", funcName, "No active backend object."));
                return nullptr;
            }

            auto* directGLESBackend =
                dynamic_cast<MG_Backend::DirectGLES::BackendObject_DirectGLES*>(activeBackend);
            if (!directGLESBackend) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", funcName,
                                                 "Sync objects are only implemented for the DirectGLES backend."));
                return nullptr;
            }

            return &directGLESBackend->GetGLESFunctions();
        }

        Bool ValidateFenceSyncArgs(const char* funcName, GLenum condition, GLbitfield flags) {
            if (condition != GL_SYNC_GPU_COMMANDS_COMPLETE) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", funcName,
                                                 "condition must be GL_SYNC_GPU_COMMANDS_COMPLETE."));
                return false;
            }
            if (flags != 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", funcName, "flags must be zero."));
                return false;
            }
            return true;
        }

        Bool ValidateSyncHandle(const char* funcName, GLsync sync) {
            if (sync != nullptr) {
                return true;
            }
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", funcName, "sync must be a valid non-null handle."));
            return false;
        }
    }

    GLsync FenceSync_Backend(GLenum condition, GLbitfield flags) {
        const auto* glesFuncs = TryGetDirectGLESFunctions(__func__);
        if (!glesFuncs || !glesFuncs->glFenceSync) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                             "Backend does not expose glFenceSync."));
            return nullptr;
        }
        return glesFuncs->glFenceSync(condition, flags);
    }

    GLboolean IsSync_Backend(GLsync sync) {
        const auto* glesFuncs = TryGetDirectGLESFunctions(__func__);
        if (!glesFuncs || !glesFuncs->glIsSync) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                             "Backend does not expose glIsSync."));
            return GL_FALSE;
        }
        return glesFuncs->glIsSync(sync);
    }

    GLenum ClientWaitSync_Backend(GLsync sync, GLbitfield flags, GLuint64 timeout) {
        const auto* glesFuncs = TryGetDirectGLESFunctions(__func__);
        if (!glesFuncs || !glesFuncs->glClientWaitSync) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                             "Backend does not expose glClientWaitSync."));
            return GL_WAIT_FAILED;
        }
        return glesFuncs->glClientWaitSync(sync, flags, timeout);
    }

    void WaitSync_Backend(GLsync sync, GLbitfield flags, GLuint64 timeout) {
        const auto* glesFuncs = TryGetDirectGLESFunctions(__func__);
        if (!glesFuncs || !glesFuncs->glWaitSync) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                             "Backend does not expose glWaitSync."));
            return;
        }
        glesFuncs->glWaitSync(sync, flags, timeout);
    }

    void GetSynciv_Backend(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values) {
        const auto* glesFuncs = TryGetDirectGLESFunctions(__func__);
        if (!glesFuncs || !glesFuncs->glGetSynciv) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                             "Backend does not expose glGetSynciv."));
            return;
        }
        glesFuncs->glGetSynciv(sync, pname, bufSize, length, values);
    }

    void DeleteSync_Backend(GLsync sync) {
        const auto* glesFuncs = TryGetDirectGLESFunctions(__func__);
        if (!glesFuncs || !glesFuncs->glDeleteSync) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                             "Backend does not expose glDeleteSync."));
            return;
        }
        glesFuncs->glDeleteSync(sync);
    }

    GLsync FenceSync_State(GLenum condition, GLbitfield flags) {
        if (!ValidateFenceSyncArgs(__func__, condition, flags)) {
            return nullptr;
        }
        return FenceSync_Backend(condition, flags);
    }

    GLboolean IsSync_State(GLsync sync) {
        if (sync == nullptr) return GL_FALSE;
        return IsSync_Backend(sync);
    }

    GLenum ClientWaitSync_State(GLsync sync, GLbitfield flags, GLuint64 timeout) {
        (void)timeout;
        if (!ValidateSyncHandle(__func__, sync)) {
            return GL_WAIT_FAILED;
        }
        if ((flags & ~GL_SYNC_FLUSH_COMMANDS_BIT) != 0) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                             "flags contains unsupported bits."));
            return GL_WAIT_FAILED;
        }
        return ClientWaitSync_Backend(sync, flags, timeout);
    }

    void WaitSync_State(GLsync sync, GLbitfield flags, GLuint64 timeout) {
        if (!ValidateSyncHandle(__func__, sync)) {
            return;
        }
        if (flags != 0) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__, "flags must be zero."));
            return;
        }
        if (timeout != GL_TIMEOUT_IGNORED) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__, "timeout must be GL_TIMEOUT_IGNORED."));
            return;
        }
        WaitSync_Backend(sync, flags, timeout);
    }

    void GetSynciv_State(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values) {
        if (!ValidateSyncHandle(__func__, sync)) {
            return;
        }
        if (bufSize < 0) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__, "bufSize must be non-negative."));
            return;
        }
        if (bufSize > 0 && values == nullptr) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                             "values must not be null when bufSize is positive."));
            return;
        }
        switch (pname) {
        case GL_OBJECT_TYPE:
        case GL_SYNC_CONDITION:
        case GL_SYNC_STATUS:
        case GL_SYNC_FLAGS:
            break;
        default:
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidEnum,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__, "Unsupported sync pname."));
            return;
        }
        GetSynciv_Backend(sync, pname, bufSize, length, values);
    }

    void DeleteSync_State(GLsync sync) {
        if (sync == nullptr) return;
        DeleteSync_Backend(sync);
    }

    GLsync FenceSync(GLenum condition, GLbitfield flags) {
        return FenceSync_State(condition, flags);
    }

    GLboolean IsSync(GLsync sync) {
        return IsSync_State(sync);
    }

    GLenum ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
        return ClientWaitSync_State(sync, flags, timeout);
    }

    void WaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
        WaitSync_State(sync, flags, timeout);
    }

    void GetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values) {
        GetSynciv_State(sync, pname, bufSize, length, values);
    }

    void DeleteSync(GLsync sync) {
        DeleteSync_State(sync);
    }
} // namespace MobileGL::MG_Impl::GLImpl
