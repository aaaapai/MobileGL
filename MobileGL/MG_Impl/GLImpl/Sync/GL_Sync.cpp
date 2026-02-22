// MobileGL - MobileGL/MG_Impl/GLImpl/Sync/GL_Sync.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "GL_Sync.h"

#include "MG_State/GLState/Core.h"

#include <MG_Util/BackendLoaders/OpenGL/Loader.h>
#include <MG_Backend/DirectGLES/DirectGLES.h>

#include <Defines.h>
#include <Config.h>

#include <MG_Util/Config/EnvChecker.h>

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
        if(MG_Util::CheckEnv("MOBILEGL_BACKEND_TYPE", "DirectGLES", false))
            return MG_Backend::DirectGLES::g_GLESFuncs.glFenceSync(condition, flags);
        else
            return FenceSync_State(condition, flags);
        }

        GLenum ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
        if(MG_Util::CheckEnv("MOBILEGL_BACKEND_TYPE", "DirectGLES", false))
            return MG_Backend::DirectGLES::g_GLESFuncs.glClientWaitSync(sync, flags, timeout);
        else
            return ClientWaitSync_State(sync, flags, timeout);

        }

        void DeleteSync(GLsync sync) {
        if (MG_Util::CheckEnv("MOBILEGL_BACKEND_TYPE", "DirectGLES", false))
            MG_Backend::DirectGLES::g_GLESFuncs.glDeleteSync(sync);
        else
            DeleteSync_State(sync);
        }

    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
