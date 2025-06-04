//
// Created by BZLZHH on 2025/3/15.
//

#include "Includes.h"

void MG_Initialize() {
    MG_Util::Debug::LogInit();
    MG_Util::Debug::LogI("MobileGL Initializing...");
    MG_State::Init();
    MG_GL::BackendLoader::Init();
}

void MG_Destroy() {
    MG_Util::Debug::LogI("MobileGL Exiting...");
    MG_State::Destroy();
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
BOOL WINAPI DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
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