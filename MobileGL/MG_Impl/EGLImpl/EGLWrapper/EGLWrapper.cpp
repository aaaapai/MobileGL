// MobileGL - MobileGL/MG_Impl/EGLImpl/EGLWrapper/EGLWrapper.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "EGLWrapper.h"
#include <Config.h>
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>

#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
namespace MobileGL {
    namespace MG_Impl::EGLImpl {
        // TODO: implement complete EGL functionality

        EGLSurface CreateWindowSurface(EGLDisplay dpy, EGLConfig config, NativeWindowType window,
                                       const EGLint* attrib_list) {
            return MG_External::EGL::eglCreateWindowSurface(dpy, config, window, attrib_list);
        }

        EGLBoolean ChooseConfig(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs, EGLint config_size,
                                EGLint* num_config) {
            return MG_External::EGL::eglChooseConfig(dpy, attrib_list, configs, config_size, num_config);
        }

        EGLContext CreateContext(EGLDisplay dpy, EGLConfig config, EGLContext shareCtx, const EGLint* attrib_list) {
            return MG_External::EGL::eglCreateContext(dpy, config, shareCtx, attrib_list);
        }

        EGLBoolean Initialize(EGLDisplay dpy, EGLint* major, EGLint* minor) {
            return MG_External::EGL::eglInitialize(dpy, major, minor);
        }

        EGLDisplay GetDisplay(NativeDisplayType display) {
            return MG_External::EGL::eglGetDisplay(display);
        }

        EGLint GetError() {
            return MG_External::EGL::eglGetError();
        }

        EGLBoolean MakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
            return MG_External::EGL::eglMakeCurrent(dpy, draw, read, ctx);
        }

        EGLBoolean DestroyContext(EGLDisplay dpy, EGLContext ctx) {
            return MG_External::EGL::eglDestroyContext(dpy, ctx);
        }

        EGLBoolean DestroySurface(EGLDisplay dpy, EGLSurface surface) {
            return MG_External::EGL::eglDestroySurface(dpy, surface);
        }

        EGLBoolean Terminate(EGLDisplay dpy) {
            return MG_External::EGL::eglTerminate(dpy);
        }

        EGLBoolean ReleaseThread(void) {
            return MG_External::EGL::eglReleaseThread();
        }

        EGLContext GetCurrentContext(void) {
            return MG_External::EGL::eglGetCurrentContext();
        }

        EGLBoolean GetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint* value) {
            return MG_External::EGL::eglGetConfigAttrib(dpy, config, attribute, value);
        }

        EGLBoolean BindAPI(EGLenum api) {
            return MG_External::EGL::eglBindAPI(api);
        }

        EGLSurface GetCurrentSurface(EGLint readdraw) {
            return MG_External::EGL::eglGetCurrentSurface(readdraw);
        }

        EGLBoolean QuerySurface(EGLDisplay display, EGLSurface surface, EGLint attribute, EGLint* value) {
            return MG_External::EGL::eglQuerySurface(display, surface, attribute, value);
        }

        char const * QueryString(EGLDisplay display, EGLint name) {
            return MG_External::EGL::eglQueryString(display, name);
        }

        EGLBoolean SwapInterval(EGLDisplay dpy, EGLint interval) {
            return MG_External::EGL::eglSwapInterval(dpy, interval);
        }

        EGLBoolean SwapBuffers(EGLDisplay dpy, EGLSurface draw) {
            return MG_External::EGL::eglSwapBuffers(dpy, draw);
        }

        EGLSurface CreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint* attrib_list) {
            return MG_External::EGL::eglCreatePbufferSurface(dpy, config, attrib_list);
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