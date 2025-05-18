//
// Created by Swung 0x48 on 2025-05-18.
//

#ifndef MOBILEGLUES_EGL_IMPL_H
#define MOBILEGLUES_EGL_IMPL_H


#include "../../../../../Includes.h"

#if BACKEND_TYPE == BACKEND_DILIGENT

namespace MG_EGL::Diligent {
    struct DeviceContext {
    public:
        static inline DeviceContext& GetInstance() {
            static DeviceContext devctx;
            return devctx;
        }

        ::Diligent::RefCntAutoPtr<::Diligent::IRenderDevice> pDevice;
        ::Diligent::RefCntAutoPtr<::Diligent::IDeviceContext> pContext;
        ::Diligent::RefCntAutoPtr<::Diligent::ISwapChain> pSwapChain;
        ::Diligent::RefCntAutoPtr<::Diligent::IPipelineState> pPSO;
        bool initialized = false;
        int majorVer = 1;
        int minorVer = 5;

    };

    EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, NativeWindowType window, const EGLint* attrib_list);
    EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs, EGLint config_size, EGLint* num_config);
    EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext shareCtx, const EGLint* attrib_list);
    EGLBoolean eglInitialize(EGLDisplay dpy, EGLint* major, EGLint* minor);
    EGLDisplay eglGetDisplay(NativeDisplayType display);
    EGLint eglGetError();
    EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
    EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx);
    EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface);
    EGLBoolean eglTerminate(EGLDisplay dpy);
    EGLBoolean eglReleaseThread(void);
    EGLContext eglGetCurrentContext(void);
    EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
    EGLBoolean eglBindAPI(EGLenum api);
    EGLSurface eglGetCurrentSurface(EGLint readdraw);
    EGLBoolean eglQuerySurface(EGLDisplay display, EGLSurface surface, EGLint attribute, EGLint *value);
    EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval);
    EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface draw);
    EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
}

#define MG_EGL_PROC_EXPORT(name) \
    if (strncmp(procname, #name, strlen(#name)) == 0) { \
        return (__eglMustCastToProperFunctionPointerType)(&MG_EGL::Diligent::name); \
    }

MG_EXPORT __eglMustCastToProperFunctionPointerType eglGetProcAddress(const char *procname);

#endif


#endif //MOBILEGLUES_EGL_IMPL_H
