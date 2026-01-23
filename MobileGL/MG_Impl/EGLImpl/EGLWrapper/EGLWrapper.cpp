// MobileGL - MobileGL/MG_Impl/EGLImpl/EGLWrapper/EGLWrapper.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "EGLWrapper.h"

#include "MG_Impl/GetProcAddress.h"

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
            // We should accept `EGL_OPENGL_API`,
            // and bind to `EGL_OPENGL_ES_API` at the backend
            MGLOG_D("%s: api = %s", __func__, api == EGL_OPENGL_API ? "EGL_OPENGL_API" : "EGL_OPENGL_ES_API");
            MGLOG_D("%s: calling backend as EGL_OPENGL_ES_API");
            return MG_External::EGL::eglBindAPI(EGL_OPENGL_ES_API);
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

        EGLBoolean BindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
            return MG_External::EGL::eglBindTexImage(dpy, surface, buffer);
        }

        EGLBoolean ReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
            return MG_External::EGL::eglReleaseTexImage(dpy, surface, buffer);
        }

        EGLBoolean CopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target) {
            return MG_External::EGL::eglCopyBuffers(dpy, surface, target);
        }

        EGLSurface CreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer,
                                                 EGLConfig config, const EGLint* attrib_list) {
            return MG_External::EGL::eglCreatePbufferFromClientBuffer(dpy, buftype, buffer, config, attrib_list);
        }

        EGLSurface CreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap,
                                        const EGLint* attrib_list) {
            return MG_External::EGL::eglCreatePixmapSurface(dpy, config, pixmap, attrib_list);
        }

        EGLBoolean GetConfigs(EGLDisplay dpy, EGLConfig* configs, EGLint config_size, EGLint* num_config) {
            return MG_External::EGL::eglGetConfigs(dpy, configs, config_size, num_config);
        }

        EGLDisplay GetCurrentDisplay(void) {
            return MG_External::EGL::eglGetCurrentDisplay();
        }

        EGLenum QueryAPI(void) {
            return MG_External::EGL::eglQueryAPI();
        }

        EGLBoolean QueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint* value) {
            return MG_External::EGL::eglQueryContext(dpy, ctx, attribute, value);
        }

        EGLBoolean SurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value) {
            return MG_External::EGL::eglSurfaceAttrib(dpy, surface, attribute, value);
        }

        EGLBoolean WaitClient(void) {
            return MG_External::EGL::eglWaitClient();
        }

        EGLBoolean WaitGL(void) {
            return MG_External::EGL::eglWaitGL();
        }

        EGLBoolean WaitNative(EGLint engine) {
            return MG_External::EGL::eglWaitNative(engine);
        }

        __eglMustCastToProperFunctionPointerType GetProcAddress(const char* name) {
            MGLOG_D("eglGetProcAddress(%s)", name);

            void* proc = MG_Impl::GetProcAddress(name);

            if (!proc) {
                MGLOG_W("Failed to get function: %s", (const char*)name);
                return nullptr;
            }
            return (__eglMustCastToProperFunctionPointerType)proc;
        }

        EGLSync CreateSync(EGLDisplay dpy, EGLenum type, const EGLAttrib * attrib_list) {
            return MG_External::EGL::eglCreateSync(dpy, type, attrib_list);
        }


        EGLBoolean DestroySync(EGLDisplay dpy, EGLSync sync) {
            return MG_External::EGL::eglDestroySync(dpy, sync);
        }


        EGLint ClientWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout) {
            return MG_External::EGL::eglClientWaitSync(dpy, sync, flags, timeout);
        }


        EGLBoolean GetSyncAttrib(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib * value) {
            return MG_External::EGL::eglGetSyncAttrib(dpy, sync, attribute, value);
        }


        EGLImage CreateImage(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib * attrib_list) {
            return MG_External::EGL::eglCreateImage(dpy, ctx, target, buffer, attrib_list);
        }


        EGLBoolean DestroyImage(EGLDisplay dpy, EGLImage image) {
            return MG_External::EGL::eglDestroyImage(dpy, image);
        }


        EGLDisplay GetPlatformDisplay(EGLenum platform, void * native_display, const EGLAttrib * attrib_list) {
            return MG_External::EGL::eglGetPlatformDisplay(platform, native_display, attrib_list);
        }


        EGLSurface CreatePlatformWindowSurface(EGLDisplay dpy, EGLConfig config, void * native_window, const EGLAttrib * attrib_list) {
            return MG_External::EGL::eglCreatePlatformWindowSurface(dpy, config, native_window, attrib_list);
        }


        EGLSurface CreatePlatformPixmapSurface(EGLDisplay dpy, EGLConfig config, void * native_pixmap, const EGLAttrib * attrib_list) {
            return MG_External::EGL::eglCreatePlatformPixmapSurface(dpy, config, native_pixmap, attrib_list);
        }


        EGLBoolean WaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags) {
            return MG_External::EGL::eglWaitSync(dpy, sync, flags);
        }
    } // namespace MG_Impl::EGLImpl
} // namespace MobileGL

#endif
