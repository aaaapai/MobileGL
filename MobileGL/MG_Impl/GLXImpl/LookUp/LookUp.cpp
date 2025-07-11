#include "../../../Includes.h"

namespace MG_Impl::GLXImpl {
	// TODO: Implement complete glx functionality

    void* GetProcAddress(const char* name) {
        MGLOG_D("glXGetProcAddress(\"%s\")", name);
        void* proc = dlsym(RTLD_DEFAULT, (const char*)name);

        if (!proc) {
            MGLOG_W("Failed to get function: %s", (const char*)name);
            return nullptr;
        }

        return proc;
    }

    void* GetProcAddressARB(const char* name) {
        return GetProcAddress(name);
    }
}