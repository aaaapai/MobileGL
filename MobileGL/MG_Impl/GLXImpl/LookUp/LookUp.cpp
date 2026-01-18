// MobileGL - MobileGL/MG_Impl/GLXImpl/LookUp/LookUp.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "LookUp.h"

namespace MG_Impl::GLXImpl {
    // TODO: implement complete GLX functionality

    void* GetProcAddress(const char* name) {
        MGLOG_D("glXGetProcAddress(\"%s\")", name);
        void* proc = MobileGL::MG_Impl::GetProcAddress(name);
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