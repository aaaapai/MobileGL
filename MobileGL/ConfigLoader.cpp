// MobileGL - MobileGL/ConfigLoader.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "Config.h"

namespace MobileGL::MG_ConfigLoader {
    static UnorderedMap<String, String> acceptedEnvVariablesMap;

    static Bool IsAcceptedPrefix(const String& key) {
        return (key.compare(0, 6, "LIBGL_") == 0 || key.compare(0, 9, "MOBILEGL_") == 0);
    }

    inline void InitializeAcceptedEnvVariables() {
        acceptedEnvVariablesMap.clear();
        char** envPtr = nullptr;

#ifdef _WIN32
        envPtr = _environ;
#else // POSIX
        envPtr = ::environ;
#endif

        if (envPtr == nullptr) return;

        for (char** env = envPtr; *env != nullptr; ++env) {
            String entry(*env);
            SizeT pos = entry.find('=');
            if (pos != String::npos) {
                String key = entry.substr(0, pos);
                String value = entry.substr(pos + 1);

                if (IsAcceptedPrefix(key)) {
                    acceptedEnvVariablesMap[key] = value;
                }
            }
        }
    }

    inline void QueryEnvVariable(const String& key, String& outValue, const String& defaultValue) {
        auto it = acceptedEnvVariablesMap.find(key);
        if (it != acceptedEnvVariablesMap.end()) {
            outValue = it->second;
        } else {
            outValue = defaultValue;
        }
    }

    inline void InitBackendType() {
        String backendTypeStr;
        QueryEnvVariable("MOBILEGL_BACKEND_TYPE", backendTypeStr, "DirectGLES");
#define ENTRY(backendType)                                                                                             \
    if (backendTypeStr == #backendType) {                                                                              \
        MG_Config::ActiveBackendType = BackendType::backendType;                                                       \
        MGLOG_I("Config: Active backend type set to " #backendType);                                                   \
        return;                                                                                                        \
    }
        ENTRY(DirectGLES)
        ENTRY(DirectVulkan)
        ENTRY(Unknown)
        MG_Config::ActiveBackendType = BackendType::Unknown;
#undef ENTRY
    }

    void Init() {
        MGLOG_D("Loading configuration from environment variables...");
        InitializeAcceptedEnvVariables();

        InitBackendType();
    }
} // namespace MobileGL::MG_ConfigLoader
