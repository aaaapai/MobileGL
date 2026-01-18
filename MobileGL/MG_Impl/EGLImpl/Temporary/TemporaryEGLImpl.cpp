// MobileGL - MobileGL/MG_Impl/EGLImpl/Temporary/TemporaryEGLImpl.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
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

        EGLBoolean BindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
            return EGL_TRUE;
        }

        EGLBoolean ReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
            return EGL_TRUE;
        }

        EGLBoolean CopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target) {
            return EGL_TRUE;
        }

        EGLSurface CreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer,
                                                 EGLConfig config, const EGLint* attrib_list) {
            return (EGLSurface)1;
        }

        EGLSurface CreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap,
                                        const EGLint* attrib_list) {
            return (EGLSurface)1;
        }

        EGLBoolean GetConfigs(EGLDisplay dpy, EGLConfig* configs, EGLint config_size, EGLint* num_config) {
            if (num_config) {
                *num_config = 1;
            }
            if (configs && config_size > 0) {
                configs[0] = (EGLConfig)1;
            }
            return EGL_TRUE;
        }

        EGLDisplay GetCurrentDisplay(void) {
            return (EGLDisplay)1;
        }

        EGLenum QueryAPI(void) {
            return EGL_OPENGL_API;
        }

        EGLBoolean QueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint* value) {
            if (value) {
                *value = 0;
            }
            return EGL_TRUE;
        }

        EGLBoolean SurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value) {
            return EGL_TRUE;
        }

        EGLBoolean WaitClient(void) {
            return EGL_TRUE;
        }

        EGLBoolean WaitGL(void) {
            return EGL_TRUE;
        }

        EGLBoolean WaitNative(EGLint engine) {
            return EGL_TRUE;
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
