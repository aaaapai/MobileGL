//
// Created by Swung 0x48 on 2025/6/20.
//

#include "GLXFuncsDefinitions.h"

MG_EXPORT void* glXGetProcAddress(const char *name) {
    return MG_GL::GLX::GetProcAddress(name);
}

MG_EXPORT void* glXGetProcAddressARB(const char *name) {
    return MG_GL::GLX::GetProcAddressARB(name);
}
