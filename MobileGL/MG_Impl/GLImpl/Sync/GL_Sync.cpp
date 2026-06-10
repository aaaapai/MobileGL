// MobileGL - MobileGL/MG_Impl/GLImpl/Sync/GL_Sync.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "GL_Sync.h"

#include <MG_State/GLState/Core.h>
#include <MG_Backend/BackendObjects.h>
#include <MG_Backend/DirectGLES/BackendObject_DirectGLES.h>
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>
#include <MG_State/GLState/ErrorState/Error.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace {
        struct SyncObject {
            GLenum Condition = GL_SYNC_GPU_COMMANDS_COMPLETE;
            GLbitfield Flags = 0;
            GLsync BackendHandle = nullptr;
        };

        UnorderedMap<GLsync, UniquePtr<SyncObject>> g_syncObjects;

        void RecordSyncError(const char* funcName, ErrorCode code, String message) {
            MG_State::pGLContext->RecordError(code,
                                              MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", funcName, Move(message)));
        }

        const MG_External::GLESFunctionsTable* TryGetDirectGLESFunctions() {
            auto* activeBackend = MG_Backend::pActiveBackendObject.get();
            if (!activeBackend) return nullptr;

            auto* directGLESBackend =
                dynamic_cast<MG_Backend::DirectGLES::BackendObject_DirectGLES*>(activeBackend);
            return directGLESBackend ? &directGLESBackend->GetGLESFunctions() : nullptr;
        }

        SyncObject* GetSyncObject(GLsync sync, const char* funcName) {
            auto it = g_syncObjects.find(sync);
            if (sync == nullptr || it == g_syncObjects.end()) {
                RecordSyncError(funcName, ErrorCode::InvalidValue, "Sync object is not valid.");
                return nullptr;
            }
            return it->second.get();
        }
    } // namespace

    GLsync FenceSync(GLenum condition, GLbitfield flags) {
        if (condition != GL_SYNC_GPU_COMMANDS_COMPLETE) {
            RecordSyncError("FenceSync", ErrorCode::InvalidEnum, "Condition must be GL_SYNC_GPU_COMMANDS_COMPLETE.");
            return nullptr;
        }
        if (flags != 0) {
            RecordSyncError("FenceSync", ErrorCode::InvalidValue, "Flags must be zero.");
            return nullptr;
        }

        auto syncObject = MakeUnique<SyncObject>();
        syncObject->Condition = condition;
        syncObject->Flags = flags;
        if (const auto* glesFuncs = TryGetDirectGLESFunctions(); glesFuncs && glesFuncs->glFenceSync) {
            syncObject->BackendHandle = glesFuncs->glFenceSync(condition, flags);
            if (syncObject->BackendHandle == nullptr) {
                return nullptr;
            }
        }
        GLsync handle = reinterpret_cast<GLsync>(syncObject.get());
        g_syncObjects[handle] = Move(syncObject);
        return handle;
    }

    GLboolean IsSync(GLsync sync) {
        return g_syncObjects.find(sync) != g_syncObjects.end() ? GL_TRUE : GL_FALSE;
    }

    GLenum ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
        (void)timeout;
        if ((flags & ~GL_SYNC_FLUSH_COMMANDS_BIT) != 0) {
            RecordSyncError("ClientWaitSync", ErrorCode::InvalidValue,
                            "Flags can only contain GL_SYNC_FLUSH_COMMANDS_BIT.");
            return GL_WAIT_FAILED;
        }
        auto* syncObject = GetSyncObject(sync, "ClientWaitSync");
        if (!syncObject) return GL_WAIT_FAILED;
        if (syncObject->BackendHandle != nullptr) {
            if (const auto* glesFuncs = TryGetDirectGLESFunctions(); glesFuncs && glesFuncs->glClientWaitSync) {
                return glesFuncs->glClientWaitSync(syncObject->BackendHandle, flags, timeout);
            }
            RecordSyncError("ClientWaitSync", ErrorCode::InvalidOperation, "Backend sync wait is unavailable.");
            return GL_WAIT_FAILED;
        }
        return GL_ALREADY_SIGNALED;
    }

    void WaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
        if (flags != 0) {
            RecordSyncError("WaitSync", ErrorCode::InvalidValue, "Flags must be zero.");
            return;
        }
        if (timeout != GL_TIMEOUT_IGNORED) {
            RecordSyncError("WaitSync", ErrorCode::InvalidValue, "Timeout must be GL_TIMEOUT_IGNORED.");
            return;
        }
        auto* syncObject = GetSyncObject(sync, "WaitSync");
        if (!syncObject) return;
        if (syncObject->BackendHandle != nullptr) {
            if (const auto* glesFuncs = TryGetDirectGLESFunctions(); glesFuncs && glesFuncs->glWaitSync) {
                glesFuncs->glWaitSync(syncObject->BackendHandle, flags, timeout);
                return;
            }
            RecordSyncError("WaitSync", ErrorCode::InvalidOperation, "Backend sync wait is unavailable.");
        }
    }

    void DeleteSync(GLsync sync) {
        if (sync == nullptr) return;
        auto it = g_syncObjects.find(sync);
        if (it == g_syncObjects.end()) {
            RecordSyncError("DeleteSync", ErrorCode::InvalidValue, "Sync object is not valid.");
            return;
        }
        if (it->second->BackendHandle != nullptr) {
            if (const auto* glesFuncs = TryGetDirectGLESFunctions(); glesFuncs && glesFuncs->glDeleteSync) {
                glesFuncs->glDeleteSync(it->second->BackendHandle);
            }
        }
        g_syncObjects.erase(it);
    }

    void GetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values) {
        if (bufSize < 0) {
            RecordSyncError("GetSynciv", ErrorCode::InvalidValue, "bufSize must be non-negative.");
            return;
        }
        if (bufSize > 0 && values == nullptr) {
            RecordSyncError("GetSynciv", ErrorCode::InvalidValue,
                            "values must not be null when bufSize is positive.");
            return;
        }

        auto* syncObject = GetSyncObject(sync, "GetSynciv");
        if (!syncObject) return;
        if (syncObject->BackendHandle != nullptr) {
            if (const auto* glesFuncs = TryGetDirectGLESFunctions(); glesFuncs && glesFuncs->glGetSynciv) {
                glesFuncs->glGetSynciv(syncObject->BackendHandle, pname, bufSize, length, values);
                return;
            }
        }

        GLint value = 0;
        switch (pname) {
        case GL_OBJECT_TYPE:
            value = GL_SYNC_FENCE;
            break;
        case GL_SYNC_STATUS:
            value = GL_SIGNALED;
            break;
        case GL_SYNC_CONDITION:
            value = static_cast<GLint>(syncObject->Condition);
            break;
        case GL_SYNC_FLAGS:
            value = static_cast<GLint>(syncObject->Flags);
            break;
        default:
            RecordSyncError("GetSynciv", ErrorCode::InvalidEnum, std::format("Invalid pname enum: 0x{:X}", pname));
            return;
        }

        if (length) {
            *length = bufSize > 0 && values ? 1 : 0;
        }
        if (bufSize > 0 && values) {
            values[0] = value;
        }
    }
} // namespace MobileGL::MG_Impl::GLImpl
