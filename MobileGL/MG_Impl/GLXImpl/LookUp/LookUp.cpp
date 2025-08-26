#include "LookUp.h"

namespace MG_Impl::GLXImpl {
    // TODO: implement complete GLX functionality

    void* GetProcAddress(const char* name) {
        MGLOG_D("glXGetProcAddress(\"%s\")", name);
#if !defined(WIN32) && !defined(__APPLE__)
        void* proc = dlsym(RTLD_DEFAULT, (const char*)name);
#else
        void* proc = NULL;
#endif
        if (!proc) {
            MGLOG_W("Failed to get function: %s", (const char*)name);
            return nullptr;
        }

        return proc;
    }

    void* GetProcAddressARB(const char* name) {
        return GetProcAddress(name);
    }
} // namespace MG_Impl::GLXImpl