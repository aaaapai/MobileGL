//
// Created by BZLZHH on 2025/3/15.
//
#pragma once

#define BACKEND_TYPE BACKEND_DILIGENT

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
#include <set>
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

#include "MG_Include/UncertainBool.hpp"
#include "MG_Include/IndexGenerator.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

#ifndef _WIN32
#include <dlfcn.h>
#include <unistd.h>
#include <vulkan/vulkan.h>
#endif

#ifdef __ANDROID__
#include <android/log.h>
#include <pthread.h>
#include <vulkan/vulkan_android.h>
#include <android/native_window.h>
#elif _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#else
#include <pthread.h>
#endif

#include <EGL/egl.h>
#include <GL/gl.h>
#include <GL/glext.h>

#if BACKEND_TYPE == BACKEND_DILIGENT
#include "EngineFactory.h"
#include "EngineFactoryOpenGL.h"
#include "EngineFactoryVk.h"
#include "RefCntAutoPtr.hpp"

#include "PipelineState.h"
#include "GraphicsTypes.h"
#include "DeviceContext.h"
#endif


#include "Constants.h"
#include "Global.h"
#include "MG_GL/Implementations/EGL/Diligent/DiligentConfig.h"
#include "MG_GL/BackendLoader/GLES/Loader.h"
#include "MG_GL/Implementations/GL/Framebuffer/GL_Framebuffer.h"
#include "MG_GL/Implementations/GL/Getter/GL_Getter.h"
#include "MG_GL/Implementations/GL/Getter/BackendInfo.h"
#include "MG_GL/Implementations/GL/Buffer/GL_Buffer.h"
#include "MG_GL/Implementations/GL/Drawing/GL_Drawing.h"
#include "MG_GL/Implementations/GL/Program/GL_Program.h"
#include "MG_GL/Implementations/GL/Texture/GL_Texture.h"
#include "MG_GL/Implementations/GL/VertexArray/GL_VertexArray.h"
#include "MG_GL/State/Common/CommonState.h"
#include "MG_GL/State/Texture/TextureState.h"
#include "MG_GL/State/VertexArray/VertexArrayState.h"
#include "MG_GL/State/Buffer/BufferState.h"
#include "MG_GL/State/Program/ShaderObject.h"
#include "MG_GL/State/Program/ProgramState.h"
#include "MG_GL/State/Framebuffer/FramebufferState.h"
#include "MG_GL/State/Core/GLState.h"
#include "MG_GL/Implementations/EGL/Diligent/EGL_impl.h"
#include "MG_GL/Implementations/GLX/LookUp/LookUp.h"
#include "MG_UTIL/Debug/Debug.h"
#include "MG_UTIL/Program/GLSLTool.h"
#include "MG_UTIL/Program/DebugTool.h"
#include "MG_GL/Implementations/GL/Common/GL_Common.h"
#include "MG_GL/Implementations/EGL/VK/EGL_EMU.h"
#include "MG_GL/Implementations/EGL/GLES/EGL_WRAP.h"
#include "MG_RHI/GLES/Test/TestDrawing.h"
