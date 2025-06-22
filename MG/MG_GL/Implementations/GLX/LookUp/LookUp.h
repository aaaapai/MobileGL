//
// Created by BZLZHH on 2025/3/15.
//

#ifndef MOBILEGL_LOOKUP_H
#define MOBILEGL_LOOKUP_H

#include <GL/gl.h>
#ifdef __cplusplus
extern "C" {
#endif

GLAPI GLAPIENTRY void *glXGetProcAddress(const char *name);
GLAPI GLAPIENTRY void *glXGetProcAddressARB(const char *name);

namespace MG_GL::GLX {
    void* GetProcAddress(const char *name);
    void* GetProcAddressARB(const char *name);
}

#ifdef __cplusplus
}
#endif

#endif //MOBILEGL_LOOKUP_H
