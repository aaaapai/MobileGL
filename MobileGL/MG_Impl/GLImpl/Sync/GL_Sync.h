// MobileGL - MobileGL/MG_Impl/GLImpl/Sync/GL_Sync.h
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        GLsync FenceSync(GLenum condition, GLbitfield flags);
        GLenum ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
        void DeleteSync(GLsync sync);
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
