// MobileGL - MobileGL/MG_Impl/EGLImpl/Temporary/TemporaryEGLImpl.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MobileGL {
    namespace MG_Impl::EGLImpl {
        EGLSurface CreateWindowSurface(EGLDisplay dpy, EGLConfig config, NativeWindowType window,
                                       const EGLint* attrib_list);
        EGLBoolean ChooseConfig(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs, EGLint config_size,
                                EGLint* num_config);
        EGLContext CreateContext(EGLDisplay dpy, EGLConfig config, EGLContext shareCtx, const EGLint* attrib_list);
        EGLBoolean Initialize(EGLDisplay dpy, EGLint* major, EGLint* minor);
        EGLDisplay GetDisplay(NativeDisplayType display);
        EGLint GetError();
        EGLBoolean MakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
        EGLBoolean DestroyContext(EGLDisplay dpy, EGLContext ctx);
        EGLBoolean DestroySurface(EGLDisplay dpy, EGLSurface surface);
        EGLBoolean Terminate(EGLDisplay dpy);
        EGLBoolean ReleaseThread(void);
        EGLContext GetCurrentContext(void);
        EGLBoolean GetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint* value);
        EGLBoolean BindAPI(EGLenum api);
        EGLSurface GetCurrentSurface(EGLint readdraw);
        EGLBoolean QuerySurface(EGLDisplay display, EGLSurface surface, EGLint attribute, EGLint* value);
        EGLBoolean SwapInterval(EGLDisplay dpy, EGLint interval);
        EGLBoolean SwapBuffers(EGLDisplay dpy, EGLSurface draw);
        EGLSurface CreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint* attrib_list);
        __eglMustCastToProperFunctionPointerType GetProcAddress(const char* name);
    } // namespace MG_Impl::EGLImpl
} // namespace MobileGL