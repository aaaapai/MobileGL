#include "Includes.h"

namespace MobileGL {
    void MG_Initialize() {
        MG_Util::Debug::InitFile();
		MGLOG_I("MobileGL Initializing...");
        MG_Backend::Init();
        glslang::InitializeProcess();
    }

    void MG_Destroy() {
        MGLOG_I("MobileGL Closing...");
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
}