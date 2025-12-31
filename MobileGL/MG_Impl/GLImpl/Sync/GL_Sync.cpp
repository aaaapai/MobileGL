// MobileGL - MobileGL/MG_Impl/GLImpl/Sync/GL_Sync.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "GL_Sync.h"

#include "MG_State/GLState/Core.h"

namespace MobileGL {
    namespace MG_Impl::GLImpl {
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
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL