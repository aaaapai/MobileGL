// selfcheck_egl_gl33_core_dlopen.c
// Build: gcc -O2 -Wall -Wextra selfcheck_egl_gl33_core_dlopen.c -o selfcheck -ldl
// Run:   ./selfcheck /path/to/libmobileglues.so
// Notes: This program intentionally avoids linking against system libEGL/libGL.

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// --- Minimal EGL types/constants (avoid including EGL headers) ---
typedef void* EGLDisplay;
typedef void* EGLConfig;
typedef void* EGLContext;
typedef void* EGLSurface;
typedef void* EGLClientBuffer;
typedef void (*__eglMustCastToProperFunctionPointerType)(void);

typedef int32_t EGLint;
typedef uint32_t EGLBoolean;

#define EGL_FALSE 0
#define EGL_TRUE  1

#define EGL_DEFAULT_DISPLAY ((void*)0)

#define EGL_NO_DISPLAY ((EGLDisplay)0)
#define EGL_NO_CONTEXT ((EGLContext)0)
#define EGL_NO_SURFACE ((EGLSurface)0)

#define EGL_SUCCESS 0x3000

#define EGL_EXTENSIONS 0x3055
#define EGL_VENDOR     0x3053
#define EGL_VERSION    0x3054
#define EGL_CLIENT_APIS 0x308D

#define EGL_NONE 0x3038

#define EGL_RENDERABLE_TYPE 0x3040
#define EGL_SURFACE_TYPE    0x3033
#define EGL_RED_SIZE        0x3024
#define EGL_GREEN_SIZE      0x3023
#define EGL_BLUE_SIZE       0x3022
#define EGL_ALPHA_SIZE      0x3021
#define EGL_DEPTH_SIZE      0x3025
#define EGL_STENCIL_SIZE    0x3026

#define EGL_OPENGL_BIT      0x0008
#define EGL_PBUFFER_BIT     0x0001

#define EGL_WIDTH  0x3057
#define EGL_HEIGHT 0x3056

#define EGL_OPENGL_API 0x30A2

// EGL_KHR_create_context attributes
#define EGL_CONTEXT_MAJOR_VERSION_KHR 0x3098
#define EGL_CONTEXT_MINOR_VERSION_KHR 0x30FB
#define EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR 0x30FD
#define EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR 0x00000001
#define EGL_CONTEXT_FLAGS_KHR 0x30FC
#define EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR 0x00000001
#define EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR 0x00000002

// --- Minimal GL constants (avoid including GL headers) ---
#define GL_VERSION                  0x1F02
#define GL_VENDOR                   0x1F00
#define GL_RENDERER                 0x1F01
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C

// --- Function pointer typedefs (EGL) ---
typedef EGLDisplay (*PFNEGLGETDISPLAY)(void* display_id);
typedef EGLBoolean (*PFNEGLINITIALIZE)(EGLDisplay dpy, EGLint* major, EGLint* minor);
typedef EGLBoolean (*PFNEGLTERMINATE)(EGLDisplay dpy);
typedef EGLint     (*PFNEGLGETERROR)(void);
typedef const char*(*PFNEGLQUERYSTRING)(EGLDisplay dpy, EGLint name);
typedef EGLBoolean (*PFNEGLBINDAPI)(EGLint api);
typedef EGLBoolean (*PFNEGLCHOOSECONFIG)(EGLDisplay dpy, const EGLint* attrib_list,
                                        EGLConfig* configs, EGLint config_size, EGLint* num_config);
typedef EGLContext (*PFNEGLCREATECONTEXT)(EGLDisplay dpy, EGLConfig config, EGLContext share_context,
                                         const EGLint* attrib_list);
typedef EGLSurface (*PFNEGLCREATEPBUFFERSURFACE)(EGLDisplay dpy, EGLConfig config, const EGLint* attrib_list);
typedef EGLBoolean (*PFNEGLDESTROYSURFACE)(EGLDisplay dpy, EGLSurface surface);
typedef EGLBoolean (*PFNEGLDESTROYCONTEXT)(EGLDisplay dpy, EGLContext ctx);
typedef EGLBoolean (*PFNEGLMAKECURRENT)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
typedef __eglMustCastToProperFunctionPointerType (*PFNEGLGETPROCADDRESS)(const char* procname);

// surfaceless extension: eglCreatePbufferSurface can be skipped if makeCurrent allows EGL_NO_SURFACE,
// but we still attempt pbuffer if surfaceless fails.

// --- Function pointer typedefs (GL) ---
typedef const unsigned char* (*PFNGLGETSTRING)(uint32_t name);

// --- Helpers ---
static void die(const char* msg) {
    fprintf(stderr, "FATAL: %s\n", msg);
    exit(1);
}

static void* must_dlsym(void* lib, const char* name) {
    void* p = dlsym(lib, name);
    if (!p) {
        fprintf(stderr, "dlsym failed for %s: %s\n", name, dlerror());
        exit(1);
    }
    return p;
}

static void print_egl_string(const char* label, PFNEGLQUERYSTRING eglQueryString,
                             EGLDisplay dpy, EGLint name) {
    const char* s = eglQueryString(dpy, name);
    printf("%s: %s\n", label, s ? s : "(null)");
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s /path/to/your_lib.so\n", argv[0]);
        return 2;
    }

    const char* lib_path = argv[1];
    printf("Loading EGL/GL compat library via dlopen: %s\n", lib_path);

    void* lib = dlopen(lib_path, RTLD_NOW | RTLD_LOCAL);
    if (!lib) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return 1;
    }

    // Load EGL entrypoints from YOUR library
    PFNEGLGETDISPLAY      eglGetDisplay      = (PFNEGLGETDISPLAY)must_dlsym(lib, "eglGetDisplay");
    PFNEGLINITIALIZE      eglInitialize      = (PFNEGLINITIALIZE)must_dlsym(lib, "eglInitialize");
    PFNEGLTERMINATE       eglTerminate       = (PFNEGLTERMINATE)must_dlsym(lib, "eglTerminate");
    PFNEGLGETERROR        eglGetError        = (PFNEGLGETERROR)must_dlsym(lib, "eglGetError");
    PFNEGLQUERYSTRING     eglQueryString     = (PFNEGLQUERYSTRING)must_dlsym(lib, "eglQueryString");
    PFNEGLBINDAPI         eglBindAPI         = (PFNEGLBINDAPI)must_dlsym(lib, "eglBindAPI");
    PFNEGLCHOOSECONFIG    eglChooseConfig    = (PFNEGLCHOOSECONFIG)must_dlsym(lib, "eglChooseConfig");
    PFNEGLCREATECONTEXT   eglCreateContext   = (PFNEGLCREATECONTEXT)must_dlsym(lib, "eglCreateContext");
    PFNEGLCREATEPBUFFERSURFACE eglCreatePbufferSurface =
        (PFNEGLCREATEPBUFFERSURFACE)must_dlsym(lib, "eglCreatePbufferSurface");
    PFNEGLDESTROYSURFACE  eglDestroySurface  = (PFNEGLDESTROYSURFACE)must_dlsym(lib, "eglDestroySurface");
    PFNEGLDESTROYCONTEXT  eglDestroyContext  = (PFNEGLDESTROYCONTEXT)must_dlsym(lib, "eglDestroyContext");
    PFNEGLMAKECURRENT     eglMakeCurrent     = (PFNEGLMAKECURRENT)must_dlsym(lib, "eglMakeCurrent");
    PFNEGLGETPROCADDRESS  eglGetProcAddress  = (PFNEGLGETPROCADDRESS)must_dlsym(lib, "eglGetProcAddress");

    // 1) Get display
    EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (dpy == EGL_NO_DISPLAY) {
        fprintf(stderr, "eglGetDisplay failed, eglGetError=0x%04x\n", eglGetError());
        return 1;
    }

    // 2) Initialize EGL
    EGLint maj = 0, min = 0;
    if (!eglInitialize(dpy, &maj, &min)) {
        fprintf(stderr, "eglInitialize failed, eglGetError=0x%04x\n", eglGetError());
        return 1;
    }
    printf("EGL initialized: %d.%d\n", maj, min);

    print_egl_string("EGL_VENDOR", eglQueryString, dpy, EGL_VENDOR);
    print_egl_string("EGL_VERSION", eglQueryString, dpy, EGL_VERSION);
    print_egl_string("EGL_CLIENT_APIS", eglQueryString, dpy, EGL_CLIENT_APIS);
    print_egl_string("EGL_EXTENSIONS", eglQueryString, dpy, EGL_EXTENSIONS);

    // 3) Bind OpenGL API (desktop)
    if (!eglBindAPI(EGL_OPENGL_API)) {
        fprintf(stderr, "eglBindAPI(EGL_OPENGL_API) failed, eglGetError=0x%04x\n", eglGetError());
        eglTerminate(dpy);
        return 1;
    }
    printf("eglBindAPI(EGL_OPENGL_API) OK\n");

    // 4) Choose config that supports OpenGL + PBUFFER
    const EGLint cfg_attribs[] = {
        EGL_SURFACE_TYPE,    EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE,   8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE,  8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };

    EGLConfig cfg = 0;
    EGLint num_cfg = 0;
    if (!eglChooseConfig(dpy, cfg_attribs, &cfg, 1, &num_cfg) || num_cfg < 1) {
        fprintf(stderr, "eglChooseConfig failed/none found, eglGetError=0x%04x, num_cfg=%d\n",
                eglGetError(), num_cfg);
        eglTerminate(dpy);
        return 1;
    }
    printf("eglChooseConfig OK (num_cfg=%d)\n", num_cfg);

    // 5) Create OpenGL 3.3 core context (KHR_create_context)
    const EGLint ctx_attribs[] = {
        EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
        EGL_CONTEXT_MINOR_VERSION_KHR, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
        // Optional flags:
        // EGL_CONTEXT_FLAGS_KHR, EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR,
        EGL_NONE
    };

    EGLContext ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, ctx_attribs);
    if (ctx == EGL_NO_CONTEXT) {
        fprintf(stderr, "eglCreateContext(GL 3.3 core) failed, eglGetError=0x%04x\n", eglGetError());
        fprintf(stderr, "Hint: check EGL_KHR_create_context & EGL_OPENGL_API support.\n");
        eglTerminate(dpy);
        return 1;
    }
    printf("eglCreateContext(GL 3.3 core) OK\n");

    // 6) Try surfaceless makeCurrent first
    EGLBoolean mc_ok = eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx);
    if (!mc_ok) {
        EGLint err = eglGetError();
        printf("Surfaceless eglMakeCurrent failed (err=0x%04x). Falling back to Pbuffer.\n", err);

        const EGLint pb_attribs[] = {
            EGL_WIDTH,  64,
            EGL_HEIGHT, 64,
            EGL_NONE
        };

        EGLSurface surf = eglCreatePbufferSurface(dpy, cfg, pb_attribs);
        if (surf == EGL_NO_SURFACE) {
            fprintf(stderr, "eglCreatePbufferSurface failed, eglGetError=0x%04x\n", eglGetError());
            eglDestroyContext(dpy, ctx);
            eglTerminate(dpy);
            return 1;
        }

        if (!eglMakeCurrent(dpy, surf, surf, ctx)) {
            fprintf(stderr, "eglMakeCurrent(pbuffer) failed, eglGetError=0x%04x\n", eglGetError());
            eglDestroySurface(dpy, surf);
            eglDestroyContext(dpy, ctx);
            eglTerminate(dpy);
            return 1;
        }
        printf("eglMakeCurrent(pbuffer) OK\n");

        // Load glGetString from eglGetProcAddress (avoid linking libGL)
        PFNGLGETSTRING glGetString =
            (PFNGLGETSTRING)(uintptr_t)eglGetProcAddress("glGetString");
        if (!glGetString) die("eglGetProcAddress(glGetString) returned NULL");

        printf("GL_VENDOR:   %s\n", (const char*)glGetString(GL_VENDOR));
        printf("GL_RENDERER: %s\n", (const char*)glGetString(GL_RENDERER));
        printf("GL_VERSION:  %s\n", (const char*)glGetString(GL_VERSION));
        printf("GLSL:        %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

        // Cleanup pbuffer path
        eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(dpy, surf);
    } else {
        printf("eglMakeCurrent(surfaceless) OK\n");

        PFNGLGETSTRING glGetString =
            (PFNGLGETSTRING)(uintptr_t)eglGetProcAddress("glGetString");
        if (!glGetString) die("eglGetProcAddress(glGetString) returned NULL");

        printf("GL_VENDOR:   %s\n", (const char*)glGetString(GL_VENDOR));
        printf("GL_RENDERER: %s\n", (const char*)glGetString(GL_RENDERER));
        printf("GL_VERSION:  %s\n", (const char*)glGetString(GL_VERSION));
        printf("GLSL:        %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

        eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }

    // Destroy context & terminate
    eglDestroyContext(dpy, ctx);
    eglTerminate(dpy);
    dlclose(lib);

    printf("Self-check finished OK.\n");
    return 0;
}
