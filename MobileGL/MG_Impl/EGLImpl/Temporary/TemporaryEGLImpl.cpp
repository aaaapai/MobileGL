// MobileGL - MobileGL/MG_Impl/EGLImpl/Temporary/TemporaryEGLImpl.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "TemporaryEGLImpl.h"
#include <Config.h>

#if !(MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES)
namespace MobileGL {
    namespace MG_Impl::EGLImpl {
        // TODO: implement complete EGL functionality

        EGLSurface CreateWindowSurface(EGLDisplay dpy, EGLConfig config, NativeWindowType window,
                                       const EGLint* attrib_list) {
            return (EGLSurface)1;
        }

        EGLBoolean ChooseConfig(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs, EGLint config_size,
                                EGLint* num_config) {
            *num_config = 1;
            return EGL_TRUE;
        }

        EGLContext CreateContext(EGLDisplay dpy, EGLConfig config, EGLContext shareCtx, const EGLint* attrib_list) {
            return (EGLContext)1;
        }

        EGLBoolean Initialize(EGLDisplay dpy, EGLint* major, EGLint* minor) {
            if (major) *major = 1;
            if (minor) *minor = 5;
            return EGL_TRUE;
        }

        EGLDisplay GetDisplay(NativeDisplayType display) {
            return (EGLDisplay)1;
        }

        EGLint GetError() {
            return EGL_SUCCESS;
        }

        EGLBoolean MakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
            return EGL_TRUE;
        }

        EGLBoolean DestroyContext(EGLDisplay dpy, EGLContext ctx) {
            return EGL_TRUE;
        }

        EGLBoolean DestroySurface(EGLDisplay dpy, EGLSurface surface) {
            return EGL_TRUE;
        }

        EGLBoolean Terminate(EGLDisplay dpy) {
            return EGL_TRUE;
        }

        EGLBoolean ReleaseThread(void) {
            return EGL_TRUE;
        }

        EGLContext GetCurrentContext(void) {
            return (EGLContext)1;
        }

        EGLBoolean GetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint* value) {
            if (attribute == EGL_NATIVE_VISUAL_ID) {
#if defined(ANDROID)
                *value = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
                return EGL_TRUE;
#elif defined(__linux__)
                *value = 0;
                return EGL_TRUE;
#elif defined(_WIN32)
                *value = 0;
                return EGL_TRUE;
#else
                *value = 0;
                return EGL_FALSE;
#endif
            }
            return EGL_TRUE;
        }

        EGLBoolean BindAPI(EGLenum api) {
            return EGL_TRUE;
        }

        EGLSurface GetCurrentSurface(EGLint readdraw) {
            return (EGLSurface)1;
        }

        EGLBoolean QuerySurface(EGLDisplay display, EGLSurface surface, EGLint attribute, EGLint* value) {
            return EGL_TRUE;
        }

        char const * QueryString(EGLDisplay display, EGLint name) {
            return "";
        }

        EGLBoolean SwapInterval(EGLDisplay dpy, EGLint interval) {
            return EGL_TRUE;
        }

        EGLBoolean SwapBuffers(EGLDisplay dpy, EGLSurface draw) {
            return EGL_TRUE;
        }

        EGLSurface CreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint* attrib_list) {
            return (EGLSurface)1;
        }

        __eglMustCastToProperFunctionPointerType GetProcAddress(const char* name) {
            MGLOG_D("eglGetProcAddress(%s)", name);
#if !defined(WIN32) && !defined(__APPLE__)
            void* proc = dlsym(RTLD_DEFAULT, (const char*)name);
#else
            void* proc = NULL;
#endif
            if (!proc) {
                MGLOG_W("Failed to get function: %s", (const char*)name);
                return nullptr;
            }
            return (__eglMustCastToProperFunctionPointerType)proc;
        }
    } // namespace MG_Impl::EGLImpl
} // namespace MobileGL
#endif