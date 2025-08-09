#include <Includes.h>
#include "../LookUp/LookUp.h"

MOBILEGL_GLX_API void* glXGetProcAddress(const char* name) {
    return MG_Impl::GLXImpl::GetProcAddress(name);
}

MOBILEGL_GLX_API void* glXGetProcAddressARB(const char* name) {
    return MG_Impl::GLXImpl::GetProcAddressARB(name);
}