#pragma once

// Include significant project headers
#include "Defines.h"
#include "MG_Util/PlatformStubs.h"

// Include necessary standard headers
#include <bit>
#include <map>
#include <array>
#include <ctime>
#include <queue>
#include <regex>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <format>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <numeric>
#include <expected>
#include <iostream>
#include <optional>
#include <algorithm>
#include <stdexcept>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <mutex>
#include <bitset>

// Include FastSTL
#include <FastSTL/UnorderedMap.h>

// Include spirv_cross
#include <spirv_cross/spirv_cross_c.h>

// Include OpenGL and EGL headers
#include <GL/gl.h>
#include <EGL/egl.h>
#include <GL/glext.h>
#include <GLES3/gl32.h>

// Include glslang headers
#include <glslang/Include/Types.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Include/intermediate.h>
#include <glslang/MachineIndependent/localintermediate.h>

// Include headers for platform-specific functionality
#ifdef __linux__
#include <dlfcn.h>
#include <pthread.h>
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#include <processthreadsapi.h>
#endif

#ifdef __ANDROID__
#include <unistd.h>
#include <pthread.h>
#include <android/log.h>
#include <vulkan/vulkan.h>
#include <android/native_window.h>
#include <vulkan/vulkan_android.h>
#include <arm_neon.h>
#endif

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#define TRACY_ZONECOLOR_ENTRY 0xFF0000
#define TRACY_ZONECOLOR_FRONTEND 0x00FF00
#define TRACY_ZONECOLOR_BACKEND 0x00FF00
#endif

// Post-includes for significant project headers
#include "MG_Util/Debug/Log.h"
#include "MG_Util/Types.h"
