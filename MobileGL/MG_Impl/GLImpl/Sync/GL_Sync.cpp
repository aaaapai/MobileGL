// MobileGL - MobileGL/MG_Impl/GLImpl/Sync/GL_Sync.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "GL_Sync.h"

#include "MG_State/GLState/Core.h"

namespace MobileGL::MG_Impl::GLImpl {
    GLsync FenceSync_Backend(GLenum condition, GLbitfield flags) {
        return 0;
    }

    GLenum ClientWaitSync_Backend(GLsync sync, GLbitfield flags, GLuint64 timeout) {
        return 0;
    }

    void DeleteSync_Backend(GLsync sync) {}

    GLsync FenceSync_State(GLenum condition, GLbitfield flags) {
        return 0;
    }

    GLenum ClientWaitSync_State(GLsync sync, GLbitfield flags, GLuint64 timeout) {
        return 0;
    }

    void DeleteSync_State(GLsync sync) {}

    GLsync FenceSync(GLenum condition, GLbitfield flags) {
        return 0;
    }

    GLenum ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
        return 0;
    }

    void DeleteSync(GLsync sync) {}
} // namespace MobileGL::MG_Impl::GLImpl
