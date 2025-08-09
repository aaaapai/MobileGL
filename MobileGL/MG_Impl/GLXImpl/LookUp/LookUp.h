#pragma once
#include <Includes.h>

namespace MG_Impl {
    namespace GLXImpl {
        void* GetProcAddress(const char* name);
        void* GetProcAddressARB(const char* name);
    } // namespace GLXImpl
} // namespace MG_Impl