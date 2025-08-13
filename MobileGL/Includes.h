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

// Include ankerl::unordered_dense
#include <ankerl/unordered_dense.h>

// Include spirv_cross
#include <spirv_cross/spirv_cross_c.h>

// Include glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Include OpenGL and EGL headers
#include <GL/gl.h>
#include <EGL/egl.h>
#include <GL/glext.h>
#include <GLES3/gl32.h>

// Include glslang headers
#include <glslang/Include/Types.h>
#include <glslang/Public/ShaderLang.h>
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
#endif

// Post-includes for significant project headers
#include "MG_Util/Types.h"
#include "MG_Util/Debug/Log.h"