// MobileGL - MobileGL/MG_Impl/EGLImpl/Exporting/Definitions.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include <Includes.h>
#include "../Temporary/TemporaryEGLImpl.h"

MOBILEGL_EGL_API EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, NativeWindowType window,
                                                   const EGLint* attrib_list) {
    return MobileGL::MG_Impl::EGLImpl::CreateWindowSurface(dpy, config, window, attrib_list);
}

MOBILEGL_EGL_API EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs,
                                            EGLint config_size, EGLint* num_config) {
    return MobileGL::MG_Impl::EGLImpl::ChooseConfig(dpy, attrib_list, configs, config_size, num_config);
}

MOBILEGL_EGL_API EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext shareCtx,
                                             const EGLint* attrib_list) {
#ifdef TRACY_ENABLE
    tracy::StartupProfiler();
#endif
    return MobileGL::MG_Impl::EGLImpl::CreateContext(dpy, config, shareCtx, attrib_list);
}

MOBILEGL_EGL_API EGLBoolean eglInitialize(EGLDisplay dpy, EGLint* major, EGLint* minor) {
    return MobileGL::MG_Impl::EGLImpl::Initialize(dpy, major, minor);
}

MOBILEGL_EGL_API EGLDisplay eglGetDisplay(NativeDisplayType display) {
    return MobileGL::MG_Impl::EGLImpl::GetDisplay(display);
}

MOBILEGL_EGL_API EGLint eglGetError() {
    return MobileGL::MG_Impl::EGLImpl::GetError();
}

MOBILEGL_EGL_API EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
    return MobileGL::MG_Impl::EGLImpl::MakeCurrent(dpy, draw, read, ctx);
}

MOBILEGL_EGL_API EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx) {
#ifdef TRACY_ENABLE
    tracy::ShutdownProfiler();
#endif
    return MobileGL::MG_Impl::EGLImpl::DestroyContext(dpy, ctx);
}

MOBILEGL_EGL_API EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface) {
    return MobileGL::MG_Impl::EGLImpl::DestroySurface(dpy, surface);
}

MOBILEGL_EGL_API EGLBoolean eglTerminate(EGLDisplay dpy) {
    return MobileGL::MG_Impl::EGLImpl::Terminate(dpy);
}

MOBILEGL_EGL_API EGLBoolean eglReleaseThread(void) {
    return MobileGL::MG_Impl::EGLImpl::ReleaseThread();
}

MOBILEGL_EGL_API EGLContext eglGetCurrentContext(void) {
    return MobileGL::MG_Impl::EGLImpl::GetCurrentContext();
}

MOBILEGL_EGL_API EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint* value) {
    return MobileGL::MG_Impl::EGLImpl::GetConfigAttrib(dpy, config, attribute, value);
}

MOBILEGL_EGL_API EGLBoolean eglBindAPI(EGLenum api) {
    return MobileGL::MG_Impl::EGLImpl::BindAPI(api);
}

MOBILEGL_EGL_API EGLSurface eglGetCurrentSurface(EGLint readdraw) {
    return MobileGL::MG_Impl::EGLImpl::GetCurrentSurface(readdraw);
}

MOBILEGL_EGL_API EGLBoolean eglQuerySurface(EGLDisplay display, EGLSurface surface, EGLint attribute, EGLint* value) {
    return MobileGL::MG_Impl::EGLImpl::QuerySurface(display, surface, attribute, value);
}

MOBILEGL_EGL_API EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval) {
    return MobileGL::MG_Impl::EGLImpl::SwapInterval(dpy, interval);
}

MOBILEGL_EGL_API EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface draw) {
#ifdef TRACY_ENABLE
    FrameMark;
#endif
    return MobileGL::MG_Impl::EGLImpl::SwapBuffers(dpy, draw);
}

MOBILEGL_EGL_API EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint* attrib_list) {
    return MobileGL::MG_Impl::EGLImpl::CreatePbufferSurface(dpy, config, attrib_list);
}

MOBILEGL_EGL_API __eglMustCastToProperFunctionPointerType eglGetProcAddress(const char* name) {
    return MobileGL::MG_Impl::EGLImpl::GetProcAddress(name);
}
