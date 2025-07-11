#pragma once

namespace MG_Impl {
    namespace GLXImpl {
        void* GetProcAddress(const char* name);
        void* GetProcAddressARB(const char* name);
    }
}