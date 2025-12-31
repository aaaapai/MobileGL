// MobileGL - MobileGL/Init.cpp
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "Includes.h"
#include <MG_Impl/Init.h>
#include <MG_Backend/Backends.h>
#include <MG_State/GLState/Core.h>

namespace MobileGL {
    void MG_Initialize() {
        MG_Util::Debug::InitFile();
        MGLOG_I("Initializing MobileGL...");
        MG_State::Init();
        MGLOG_D("MobileGL State initialized");
        MG_Backend::Init();
        MGLOG_D("MobileGL Backend initialized");
        MG_Impl::Init();
        MGLOG_D("MobileGL Implementation initialized");
        glslang::InitializeProcess();
        MGLOG_D("glslang initialized");
        MGLOG_I("MobileGL initialized");
    }

    void MG_Destroy() {
        MGLOG_I("MobileGL closing...");
        glslang::FinalizeProcess();
        MG_Util::Debug::Close();
    }

#if defined(__linux__) || defined(__APPLE__)
    __attribute__((constructor)) static void AutoInit() {
        MG_Initialize();
    }

    __attribute__((destructor)) static void AutoDestroy() {
        MG_Destroy();
    }
#endif

#ifdef _WIN32
    BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
        switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            MG_Initialize();
            break;

        case DLL_PROCESS_DETACH:
            MG_Destroy();
            break;
        }
        return TRUE;
    }
#endif
} // namespace MobileGL