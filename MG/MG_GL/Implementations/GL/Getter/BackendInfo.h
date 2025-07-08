//
// Created by BZLZHH on 2025/3/15.
//

#ifndef MOBILEGL_BACKENDINFO_H
#define MOBILEGL_BACKENDINFO_H

#include "../../../../Includes.h"

namespace MG_GL::Getter {
    const std::string GetGPUName();
    const std::string GetBackendName();
    const std::string GetBackendGfxAPIName();
    const uint32_t GetBackendGfxAPIVersionMajor();
    const uint32_t GetBackendGfxAPIVersionMinor();
    const std::string GetMGName();
    void LogMGInfo();
}

#endif //MOBILEGL_BACKENDINFO_H
