#!/usr/bin/env python3
"""
generate_vulkan_loader.py
自动从 Vulkan 头文件生成完整的动态加载器（支持 VULKAN_PTR 为指针地址或库路径）。
用法: python3 generate_vulkan_loader.py /path/to/vulkan/include
输出: vulkan_loader.h 和 vulkan_loader.cpp
"""

import os
import re
import sys
from pathlib import Path

# ---------- 解析头文件 ----------
def find_vulkan_headers(include_dir):
    """递归查找 vulkan 目录下所有 .h 文件"""
    vulkan_dir = Path(include_dir) / "vulkan"
    if not vulkan_dir.exists():
        raise FileNotFoundError(f"Vulkan directory not found in {include_dir}")
    headers = []
    for root, _, files in os.walk(vulkan_dir):
        for file in files:
            if file.endswith(".h"):
                headers.append(Path(root) / file)
    return headers

def extract_function_declarations(headers):
    """从头文件列表中提取所有 VKAPI_ATTR 函数声明"""
    func_pattern = re.compile(
        r'VKAPI_ATTR\s+'          # 属性宏
        r'([\w\s]+?)'             # 返回类型（可能含空格，如 VkResult）
        r'\s+VKAPI_CALL\s+'       # 调用约定宏
        r'(\w+)\s*'               # 函数名
        r'\(([^;]*?)\)\s*;',      # 参数列表（直到分号）
        re.DOTALL | re.MULTILINE
    )
    functions = []
    for header in headers:
        with open(header, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        # 移除注释
        content = re.sub(r'//.*?$', '', content, flags=re.MULTILINE)
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
        for match in func_pattern.finditer(content):
            ret_type = match.group(1).strip()
            func_name = match.group(2).strip()
            params = match.group(3).strip()
            if not func_name.startswith('vk'):
                continue
            functions.append((func_name, ret_type, params))
    return functions

def generate_loader(include_dir, output_h, output_cpp):
    headers = find_vulkan_headers(include_dir)
    functions = extract_function_declarations(headers)
    if not functions:
        raise RuntimeError("No Vulkan functions found. Check header path.")

    # 去重
    unique_funcs = {}
    for name, ret, params in functions:
        if name not in unique_funcs:
            unique_funcs[name] = (ret, params)
    functions = [(name, ret, params) for name, (ret, params) in unique_funcs.items()]

    # ---------- 生成头文件 ----------
    h_content = """// vulkan_loader.h - Auto-generated Vulkan dynamic loader
#pragma once

#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

// 核心入口指针（由初始化函数填充）
extern PFN_vkGetInstanceProcAddr g_vkGetInstanceProcAddr;

// 所有其他 Vulkan 函数指针声明
"""
    for name, _, _ in functions:
        h_content += f"extern PFN_{name} g_{name};\n"

    h_content += "\n// 重定向宏：将函数名映射到全局指针\n"
    for name, _, _ in functions:
        h_content += f"#define {name} g_{name}\n"

    h_content += """
// 初始化函数：加载 Vulkan 库或使用 VULKAN_PTR 直接获取入口，并填充所有函数指针
// 返回 true 成功，false 失败（错误信息可通过 GetVulkanLoaderError() 获取）
bool VulkanLoader_Init(void);

// 获取最后一次错误信息（线程安全，返回静态字符串指针）
const char* VulkanLoader_GetError(void);

#ifdef __cplusplus
}
#endif
"""

    # ---------- 生成 cpp 文件 ----------
    cpp_content = f"""// vulkan_loader.cpp - Auto-generated Vulkan dynamic loader
#include "vulkan_loader.h"

#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cstdio>

// 平台相关头文件
#if defined(_WIN32)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #include <dlfcn.h>
#endif

// 定义核心入口指针
PFN_vkGetInstanceProcAddr g_vkGetInstanceProcAddr = nullptr;

// 定义所有函数指针
"""
    for name, _, _ in functions:
        cpp_content += f"PFN_{name} g_{name} = nullptr;\n"

    cpp_content += """
// 错误信息缓冲区
static char s_error_msg[256] = {0};

// 设置错误信息（带格式化）
static void SetError(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(s_error_msg, sizeof(s_error_msg), fmt, args);
    va_end(args);
}

// ---------- 平台相关动态库加载辅助 ----------
#if defined(_WIN32)
static HMODULE g_vulkan_lib = nullptr;
static void* GetProcAddress(const char* name) {
    return (void*)::GetProcAddress(g_vulkan_lib, name);
}
static bool LoadLibraryFile(const char* path) {
    g_vulkan_lib = ::LoadLibraryA(path);
    return g_vulkan_lib != nullptr;
}
#else
static void* g_vulkan_lib = nullptr;
static void* GetProcAddress(const char* name) {
    return dlsym(g_vulkan_lib, name);
}
static bool LoadLibraryFile(const char* path) {
    g_vulkan_lib = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    return g_vulkan_lib != nullptr;
}
#endif

// ---------- 加载所有函数 ----------
static bool LoadAllFunctions(VkInstance instance, VkDevice device) {
    // 辅助宏：尝试从 instance 获取，如果失败再从 device 获取
#define LOAD_FUNC(ptr, name) do { \\
    ptr = (PFN_##name)g_vkGetInstanceProcAddr(instance, #name); \\
    if (!ptr && device != VK_NULL_HANDLE) { \\
        ptr = (PFN_##name)g_vkGetInstanceProcAddr(device, #name); \\
    } \\
    if (!ptr) { \\
        SetError("Failed to load function: %s", #name); \\
        return false; \\
    } \\
} while(0)
"""
    for name, _, _ in functions:
        cpp_content += f"    LOAD_FUNC(g_{name}, {name});\n"

    cpp_content += """
#undef LOAD_FUNC
    return true;
}

// ---------- 初始化入口 ----------
bool VulkanLoader_Init(void) {
    if (g_vkGetInstanceProcAddr != nullptr)
        return true; // 已经初始化

    // 1. 检查 VULKAN_PTR 环境变量
    const char* vulkan_ptr_env = std::getenv("VULKAN_PTR");
    bool loaded_from_ptr = false;

    if (vulkan_ptr_env && vulkan_ptr_env[0] != '\\0') {
        // 尝试将其解析为十六进制地址（如 "0x1234" 或 "1234"）
        char* end = nullptr;
        uintptr_t addr = std::strtoull(vulkan_ptr_env, &end, 0); // 0 自动检测进制
        if (end != vulkan_ptr_env && *end == '\\0') {
            // 成功解析为数字，视为函数指针
            g_vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(addr);
            loaded_from_ptr = true;
        }
    }

    // 2. 如果没有从指针解析成功，则尝试加载动态库
    if (!loaded_from_ptr) {
        const char* lib_path = vulkan_ptr_env ? vulkan_ptr_env :
#if defined(_WIN32)
            "vulkan-1.dll";
#else
            "libvulkan.so";
#endif

        if (!LoadLibraryFile(lib_path)) {
            SetError("Failed to load Vulkan library: %s", lib_path);
            return false;
        }

        // 从库中获取 vkGetInstanceProcAddr
        g_vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress("vkGetInstanceProcAddr");
        if (!g_vkGetInstanceProcAddr) {
            SetError("vkGetInstanceProcAddr not found in library");
            return false;
        }
    }

    // 3. 现在使用 g_vkGetInstanceProcAddr 创建临时实例和设备来加载所有函数
    // 获取 vkCreateInstance
    PFN_vkCreateInstance createInstance = (PFN_vkCreateInstance)
        g_vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance");
    if (!createInstance) {
        SetError("vkCreateInstance not available");
        return false;
    }

    VkApplicationInfo appInfo = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        nullptr,
        "",
        0,
        "",
        0,
        VK_API_VERSION_1_0
    };
    VkInstanceCreateInfo instInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        nullptr,
        0,
        &appInfo,
        0,
        nullptr,
        0,
        nullptr
    };
    VkInstance temp_instance = VK_NULL_HANDLE;
    if (createInstance(&instInfo, nullptr, &temp_instance) != VK_SUCCESS) {
        SetError("Failed to create temporary Vulkan instance");
        return false;
    }

    // 获取 vkEnumeratePhysicalDevices 和 vkCreateDevice
    PFN_vkEnumeratePhysicalDevices enumerateDevices = (PFN_vkEnumeratePhysicalDevices)
        g_vkGetInstanceProcAddr(temp_instance, "vkEnumeratePhysicalDevices");
    PFN_vkCreateDevice createDevice = (PFN_vkCreateDevice)
        g_vkGetInstanceProcAddr(temp_instance, "vkCreateDevice");
    if (!enumerateDevices || !createDevice) {
        SetError("Failed to get device creation functions from temporary instance");
        g_vkGetInstanceProcAddr(temp_instance, "vkDestroyInstance")(temp_instance, nullptr);
        return false;
    }

    // 枚举物理设备
    uint32_t deviceCount = 0;
    enumerateDevices(temp_instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        SetError("No physical devices found");
        g_vkGetInstanceProcAddr(temp_instance, "vkDestroyInstance")(temp_instance, nullptr);
        return false;
    }
    VkPhysicalDevice physDevices[1];
    enumerateDevices(temp_instance, &deviceCount, physDevices);

    // 创建临时设备（无需任何队列）
    VkDeviceCreateInfo devInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        nullptr,
        0,
        0,
        nullptr,
        0,
        nullptr,
        0,
        nullptr,
        nullptr
    };
    VkDevice temp_device = VK_NULL_HANDLE;
    if (createDevice(physDevices[0], &devInfo, nullptr, &temp_device) != VK_SUCCESS) {
        SetError("Failed to create temporary Vulkan device");
        g_vkGetInstanceProcAddr(temp_instance, "vkDestroyInstance")(temp_instance, nullptr);
        return false;
    }

    // 加载所有函数
    bool success = LoadAllFunctions(temp_instance, temp_device);

    // 销毁临时资源
    PFN_vkDestroyDevice destroyDevice = (PFN_vkDestroyDevice)
        g_vkGetInstanceProcAddr(temp_instance, "vkDestroyDevice");
    if (destroyDevice) destroyDevice(temp_device, nullptr);
    PFN_vkDestroyInstance destroyInstance = (PFN_vkDestroyInstance)
        g_vkGetInstanceProcAddr(temp_instance, "vkDestroyInstance");
    if (destroyInstance) destroyInstance(temp_instance, nullptr);

    if (!success) {
        // 错误已由 LoadAllFunctions 设置
        return false;
    }

    return true;
}

const char* VulkanLoader_GetError(void) {
    return s_error_msg;
}
"""

    with open(output_h, 'w') as f:
        f.write(h_content)
    with open(output_cpp, 'w') as f:
        f.write(cpp_content)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 generate_vulkan_loader.py <path_to_vulkan_include>")
        sys.exit(1)
    include_dir = sys.argv[1]
    generate_loader(include_dir, "vulkan_loader.h", "vulkan_loader.cpp")
    print("Generated vulkan_loader.h and vulkan_loader.cpp successfully.")
