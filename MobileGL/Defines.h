// MobileGL - MobileGL/Defines.h
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

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
#define MOBILEGL_LOG_ACTIVE_LEVEL MOBILEGL_LOG_LEVEL_INFO

#define MOBILEGL_LOG_ENABLE_CONSOLE 0
#define MOBILEGL_LOG_ENABLE_FILE 1
#define MOBILEGL_LOG_ENABLE_ANDROID 1

#ifdef __ANDROID__
#define MOBILEGL_LOG_FILE_PATH "/sdcard/MG/latest.log"
#else
#define MOBILEGL_LOG_FILE_PATH ""
#endif

#if defined _MSC_VER or defined __MINGW32__ or defined __MINGW64__
#define TRAP assert(false)
#elif __clang__
#define TRAP __builtin_debugtrap()
#else
#include <signal.h>
#define TRAP raise(SIGTRAP)
#endif

// =============================== Utils ================================ //
#define MOBILEGL_ASSERT(condition, ...)                                                                                \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            MGLOG_F("Assertion failed" __VA_OPT__(": ") __VA_ARGS__);                                                  \
            MGLOG_F("  at %s:%d (%s)", __FILE__, __LINE__, __func__);                                                  \
            TRAP;                                                                                                      \
        }                                                                                                              \
    } while (0)
