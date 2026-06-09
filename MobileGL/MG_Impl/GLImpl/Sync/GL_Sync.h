// MobileGL - MobileGL/MG_Impl/GLImpl/Sync/GL_Sync.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MobileGL::MG_Impl::GLImpl {
    GLsync FenceSync(GLenum condition, GLbitfield flags);
    GLboolean IsSync(GLsync sync);
    void WaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
    GLenum ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
    void GetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values);
    void DeleteSync(GLsync sync);
} // namespace MobileGL::MG_Impl::GLImpl
