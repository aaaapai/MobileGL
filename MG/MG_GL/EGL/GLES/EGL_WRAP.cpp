//
// Created by BZLZHH on 2025/4/26.
//

#include "EGL_WRAP.h"

#if BACKEND_TYPE == BACKEND_GLES

MG_EXPORT EGLint eglGetError(void) {
    LOAD_EGL(eglGetError)
    return egl_eglGetError();
}

MG_EXPORT EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id) {
    LOAD_EGL(eglGetDisplay)
    return egl_eglGetDisplay(display_id);
}

MG_EXPORT EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor) {
    LOAD_EGL(eglInitialize)
    return egl_eglInitialize(dpy, major, minor);
}

MG_EXPORT EGLBoolean eglTerminate(EGLDisplay dpy) {
    LOAD_EGL(eglTerminate)
    return egl_eglTerminate(dpy);
}

MG_EXPORT const char *eglQueryString(EGLDisplay dpy, EGLint name) {
    LOAD_EGL(eglQueryString)
    return egl_eglQueryString(dpy, name);
}

MG_EXPORT EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config) {
    LOAD_EGL(eglGetConfigs)
    return egl_eglGetConfigs(dpy, configs, config_size, num_config);
}

MG_EXPORT EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config) {
    LOAD_EGL(eglChooseConfig)
    return egl_eglChooseConfig(dpy, attrib_list, configs, config_size, num_config);
}

MG_EXPORT EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value) {
    LOAD_EGL(eglGetConfigAttrib)
    return egl_eglGetConfigAttrib(dpy, config, attribute, value);
}

MG_EXPORT EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list) {
    LOAD_EGL(eglCreateWindowSurface)
    return egl_eglCreateWindowSurface(dpy, config, win, attrib_list);
}

MG_EXPORT EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) {
    LOAD_EGL(eglCreatePbufferSurface)
    return egl_eglCreatePbufferSurface(dpy, config, attrib_list);
}

MG_EXPORT EGLSurface eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list) {
    LOAD_EGL(eglCreatePixmapSurface)
    return egl_eglCreatePixmapSurface(dpy, config, pixmap, attrib_list);
}

MG_EXPORT EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface) {
    LOAD_EGL(eglDestroySurface)
    return egl_eglDestroySurface(dpy, surface);
}

MG_EXPORT EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value) {
    LOAD_EGL(eglQuerySurface)
    return egl_eglQuerySurface(dpy, surface, attribute, value);
}

MG_EXPORT EGLBoolean eglBindAPI(EGLenum api) {
    LOAD_EGL(eglBindAPI)
    return egl_eglBindAPI(api);
}

MG_EXPORT EGLenum eglQueryAPI(void) {
    LOAD_EGL(eglQueryAPI)
    return egl_eglQueryAPI();
}

MG_EXPORT EGLBoolean eglWaitClient(void) {
    LOAD_EGL(eglWaitClient)
    return egl_eglWaitClient();
}

MG_EXPORT EGLBoolean eglReleaseThread(void) {
    LOAD_EGL(eglReleaseThread)
    return egl_eglReleaseThread();
}

MG_EXPORT EGLSurface eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list) {
    LOAD_EGL(eglCreatePbufferFromClientBuffer)
    return egl_eglCreatePbufferFromClientBuffer(dpy, buftype, buffer, config, attrib_list);
}

MG_EXPORT EGLBoolean eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value) {
    LOAD_EGL(eglSurfaceAttrib)
    return egl_eglSurfaceAttrib(dpy, surface, attribute, value);
}

MG_EXPORT EGLBoolean eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    LOAD_EGL(eglBindTexImage)
    return egl_eglBindTexImage(dpy, surface, buffer);
}

MG_EXPORT EGLBoolean eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    LOAD_EGL(eglReleaseTexImage)
    return egl_eglReleaseTexImage(dpy, surface, buffer);
}

MG_EXPORT EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval) {
    LOAD_EGL(eglSwapInterval)
    return egl_eglSwapInterval(dpy, interval);
}

MG_EXPORT EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) {
    LOAD_EGL(eglCreateContext)
    return egl_eglCreateContext(dpy, config, share_context, attrib_list);
}

MG_EXPORT EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx) {
    LOAD_EGL(eglDestroyContext)
    return egl_eglDestroyContext(dpy, ctx);
}

MG_EXPORT EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
    LOAD_EGL(eglMakeCurrent)
    return egl_eglMakeCurrent(dpy, draw, read, ctx);
}

MG_EXPORT EGLContext eglGetCurrentContext(void) {
    LOAD_EGL(eglGetCurrentContext)
    return egl_eglGetCurrentContext();
}

MG_EXPORT EGLSurface eglGetCurrentSurface(EGLint readdraw) {
    LOAD_EGL(eglGetCurrentSurface)
    return egl_eglGetCurrentSurface(readdraw);
}

MG_EXPORT EGLDisplay eglGetCurrentDisplay(void) {
    LOAD_EGL(eglGetCurrentDisplay)
    return egl_eglGetCurrentDisplay();
}

MG_EXPORT EGLBoolean eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value) {
    LOAD_EGL(eglQueryContext)
    return egl_eglQueryContext(dpy, ctx, attribute, value);
}

MG_EXPORT EGLBoolean eglWaitGL(void) {
    LOAD_EGL(eglWaitGL)
    return egl_eglWaitGL();
}

MG_EXPORT EGLBoolean eglWaitNative(EGLint engine) {
    LOAD_EGL(eglWaitNative)
    return egl_eglWaitNative(engine);
}

MG_EXPORT EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    LOAD_EGL(eglSwapBuffers)
    return egl_eglSwapBuffers(dpy, surface);
}

MG_EXPORT EGLBoolean eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target) {
    LOAD_EGL(eglCopyBuffers)
    return egl_eglCopyBuffers(dpy, surface, target);
}

MG_EXPORT EGLDisplay eglGetPlatformDisplay(EGLenum platform, void *native_display, const EGLAttrib *attrib_list) {
    LOAD_EGL(eglGetPlatformDisplay)
    return egl_eglGetPlatformDisplay(platform, native_display, (const EGLint *)attrib_list);
}

#endif