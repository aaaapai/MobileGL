// MobileGL - MobileGL/MG_Impl/GLXImpl/LookUp/LookUp.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
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