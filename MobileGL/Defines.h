#pragma once

// ============== Platform-specific definitions and macros ============== //
#ifdef __ANDROID__
#undef __ANDROID_API__
#define __ANDROID_API__ 26 // force Android API level to 26 for compatibility
#endif

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX 1 // prevent Windows.h from defining min and max macros
#endif
#endif

// ================== MobileGL definitions and macros =================== //
#ifdef _WIN32
#define MOBILEGL_EXPORT extern "C" __declspec(dllexport)
#else
#define MOBILEGL_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#define MOBILEGL_API MOBILEGL_EXPORT

#define MOBILEGL_GLX_API MOBILEGL_API
#define MOBILEGL_GL_API MOBILEGL_API
#define MOBILEGL_EGL_API MOBILEGL_API

#define MOBILEGL_BACKEND_TYPE_UNKNOWN 0
#define MOBILEGL_BACKEND_TYPE_DILIGENT 1
#define MOBILEGL_BACKEND_TYPE_MRHI 2
#define MOBILEGL_BACKEND_TYPE_DIRECT_GLES 3

// ====================== MobileGL configurations ======================= //
#define MOBILEGL_LOG_ACTIVE_LEVEL MOBILEGL_LOG_LEVEL_DEBUG

#define MOBILEGL_LOG_ENABLE_CONSOLE 0
#define MOBILEGL_LOG_ENABLE_FILE 1
#define MOBILEGL_LOG_ENABLE_ANDROID 1

#ifdef __ANDROID__
#define MOBILEGL_LOG_FILE_PATH "/sdcard/MG/latest.log"
#else
#define MOBILEGL_LOG_FILE_PATH ""
#endif

// =============================== Utils ================================ //
#define MOBILEGL_ASSERT(condition, ...)                                                                                \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            MGLOG_F("Assertion failed" __VA_OPT__(": ") __VA_ARGS__);                                                  \
            MGLOG_F("  at %s:%d (%s)", __FILE__, __LINE__, __func__);                                                  \
            assert(false);                                                                                             \
        }                                                                                                              \
    } while (0)
