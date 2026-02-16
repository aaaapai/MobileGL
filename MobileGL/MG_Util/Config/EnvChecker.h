#pragma once
#include <Includes.h>

namespace MobileGL {
namespace MG_Util {

static inline bool CheckEnvANGLE(bool forceRefresh = false) {
    // 使用 static 变量缓存结果
    static const char* cachedValue = nullptr;
    static bool initialized = false;
    
    // 强制刷新或第一次调用时获取环境变量
    if (forceRefresh || !initialized) {
        cachedValue = std::getenv("LIBGL_EGL");
        initialized = true;
    }
    
    // 如果环境变量不存在
    if (cachedValue == nullptr) {
        return false;
    }
    
    // 比较环境变量的值是否完全匹配 "libEGL_ANGLE.so"
    return std::strcmp(cachedValue, "libEGL_ANGLE.so") == 0;
}

static inline bool CheckGLESDriverEnvExists(bool forceRefresh = false) {
    // 使用 static 变量缓存结果
    static const char* cachedValue = nullptr;
    static bool initialized = false;
    
    // 强制刷新或第一次调用时获取环境变量
    if (forceRefresh || !initialized) {
        cachedValue = std::getenv("LIBGL_EGL");
        initialized = true;
    }
    
    return cachedValue != nullptr;
}

static inline const char* GetGLESDriverEnvValue(bool forceRefresh = false) {
    // 使用 static 变量缓存结果
    static const char* cachedValue = nullptr;
    static bool initialized = false;
    
    // 强制刷新或第一次调用时获取环境变量
    if (forceRefresh || !initialized) {
        cachedValue = std::getenv("LIBGL_EGL");
        initialized = true;
    }
    
    return cachedValue;
}

static inline bool CheckEnv(const char* envName, const char* expectedValue = nullptr, bool forceRefresh = true) {
    static const char* cachedEnvName = nullptr;
    static const char* cachedValue = nullptr;
    static bool initialized = false;
    
    if (forceRefresh || !initialized || 
        (envName != cachedEnvName && 
         (envName == nullptr || cachedEnvName == nullptr || 
          std::strcmp(envName, cachedEnvName) != 0))) {
        
        cachedEnvName = envName;
        cachedValue = std::getenv(envName);
        initialized = true;
    }
    
    if (cachedValue == nullptr) {
        return false;
    }
    
    if (expectedValue == nullptr) {
        return true;
    }
    
    return std::strcmp(cachedValue, expectedValue) == 0;
}

} // namespace MG_Util
} // namespace MobileGL
