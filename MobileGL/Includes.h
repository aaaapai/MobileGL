#pragma once

#ifdef __ANDROID__
#define __ANDROID_API__ 26 // fix pthread_getname_np not defined
#endif

#ifdef _WIN32
#define MOBILEGL_EXPORT extern "C" __declspec(dllexport)
#else
#define MOBILEGL_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#define MOBILEGL_API MOBILEGL_EXPORT

#define MOBILEGL_GLX_API MOBILEGL_API
#define MOBILEGL_GL_API  MOBILEGL_API
#define MOBILEGL_EGL_API MOBILEGL_API

#include <cstring>
#include <iostream>
#include <cstdio>
#include <ctime>
#include <chrono>
#include <thread>
#include <string>
#include <map>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <atomic>
#include <regex>
#include <strstream>
#include <algorithm>
#include <array>
#include <random>
#include <optional>
#include <unordered_map>
#include <queue>
#include <format>
#include <memory>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/Types.h>
#include <glslang/Public/ShaderLang.h>
#include <spirv_cross/spirv_cross_c.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <ankerl/unordered_dense.h>
#include <GLES3/gl32.h>

#ifdef __ANDROID__
#include <android/log.h>
#include <pthread.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>
#include <android/native_window.h>
#include <dlfcn.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#include <processthreadsapi.h>
#else
#include <pthread.h>
#endif

#include <EGL/egl.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include "MG_Util/Debug/Log.h"
#include "MG_Util/Converters/GLToStr/GLEnumConverter.h"

#include "Config.h"

#include "MG_Impl/GLXImpl/LookUp/LookUp.h"
#include "MG_Impl/EGLImpl/Temporary/TemporaryEGLImpl.h"