#pragma once
#include <Includes.h>

namespace MobileGL {
namespace MG_Util {

static inline bool checkEnvExists(const char* envName, const char* expectedValue = nullptr) {
    if (envName == nullptr || envName[0] == '\0') {
        return false;
    }
    
    const char* envValue = std::getenv(envName);
    
    if (envValue == nullptr) {
        return false;
    }
    
    if (expectedValue == nullptr) {
        return true;
    }
    
    return std::strcmp(envValue, expectedValue) == 0;
}

} // namespace MG_Util
} // namespace MobileGL

