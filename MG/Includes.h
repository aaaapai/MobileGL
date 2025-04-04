//
// Created by BZLZHH on 2025/3/15.
//

#include "Constants.h"
#define BACKEND_TYPE BACKEND_VULKAN
#include "Global.h"
#ifndef MOBILEGL_GL_FRAMEBUFFER_H
#include "MG_GL/GL/Framebuffer/GL_Framebuffer.h"
#endif
#ifndef MOBILEGL_GL_GETTER_H
#include "MG_GL/GL/Getter/GL_Getter.h"
#endif
#ifndef MOBILEGL_BACKENDINFO_H
#include "MG_GL/GL/Getter/BackendInfo.h"
#endif
#ifndef MOBILEGL_GL_BUFFER_H
#include "MG_GL/GL/Buffer/GL_Buffer.h"
#endif
#ifndef MOBILEGL_GL_COMMAND_H
#include "MG_GL/GL/Command/GL_Command.h"
#endif
#ifndef MOBILEGL_GL_PROGRAM_H
#include "MG_GL/GL/Program/GL_Program.h"
#endif
#ifndef MOBILEGL_GL_SHADER_H
#include "MG_GL/GL/Shader/GL_Shader.h"
#endif
#ifndef MOBILEGL_GL_TEXTURE_H
#include "MG_GL/GL/Texture/GL_Texture.h"
#endif
#ifndef MOBILEGL_GL_VERTEXARRAY_H
#include "MG_GL/GL/VertexArray/GL_VertexArray.h"
#endif
#ifndef MOBILEGL_GLSTATE_H
#include "MG_GL/State/Core/GLState.h"
#endif
#ifndef MOBILEGL_TEXTURESTATE_H
#include "MG_GL/State/Texture/TextureState.h"
#endif
#ifndef MOBILEGL_LOOKUP_H
#include "MG_GL/GLX/LookUp/LookUp.h"
#endif
#ifndef MOBILEGL_DEBUG_H
#include "MG_UTIL/Debug/Debug.h"
#endif
#ifndef MOBILEGL_GL_DEPTH_H
#include "MG_GL/GL/Common/GL_Common.h"
#endif
#ifndef MOBILEGL_EGL_VK_EMU_H
#include "MG_GL/EGL/VK/EGL_EMU.h"
#endif

#include <cstring>
#include <iostream>
#include <cstdio>
#include <dlfcn.h>
#include <unistd.h>
#include <ctime>
#include <chrono>
#include <thread>
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <atomic>
#include <regex>
#include <strstream>
#include <algorithm>
#include <array>
#include <random>
#include <unordered_map>
#include <queue>
#include <vulkan/vulkan.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/Types.h>
#include <glslang/Public/ShaderLang.h>
#include <spirv_cross/spirv_cross_c.h>
#include <glslang/SPIRV/GlslangToSpv.h>

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
