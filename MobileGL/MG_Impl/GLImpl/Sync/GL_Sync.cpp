// MobileGL - MobileGL/MG_Impl/GLImpl/Sync/GL_Sync.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "GL_Sync.h"

namespace MobileGL::MG_Impl::GLImpl {
    namespace {
        int g_stubSyncObject = 0;
    }

    // glSync semantics not really needed right now, stubbing them out

    GLsync FenceSync(GLenum condition, GLbitfield flags) {
        (void)condition;
        (void)flags;
        return reinterpret_cast<GLsync>(&g_stubSyncObject);
    }

    GLboolean IsSync(GLsync sync) {
        return sync != nullptr ? GL_TRUE : GL_FALSE;
    }

    GLenum ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
        (void)sync;
        (void)flags;
        (void)timeout;
        return GL_ALREADY_SIGNALED;
    }

    void WaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
        (void)sync;
        (void)flags;
        (void)timeout;
    }

    void DeleteSync(GLsync sync) {
        (void)sync;
    }

    void GetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values) {
        (void)sync;
        GLint value = 0;
        switch (pname) {
        case GL_OBJECT_TYPE:
            value = GL_SYNC_FENCE;
            break;
        case GL_SYNC_STATUS:
            value = GL_SIGNALED;
            break;
        case GL_SYNC_CONDITION:
            value = GL_SYNC_GPU_COMMANDS_COMPLETE;
            break;
        case GL_SYNC_FLAGS:
            value = 0;
            break;
        default:
            break;
        }

        if (length) {
            *length = bufSize > 0 && values ? 1 : 0;
        }
        if (bufSize > 0 && values) {
            values[0] = value;
        }
    }
} // namespace MobileGL::MG_Impl::GLImpl
