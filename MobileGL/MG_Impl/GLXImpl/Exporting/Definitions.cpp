// MobileGL - MobileGL/MG_Impl/GLXImpl/Exporting/Definitions.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include <Includes.h>
#include "../LookUp/LookUp.h"

MOBILEGL_GLX_API void* glXGetProcAddress(const char* name) {
    return MG_Impl::GLXImpl::GetProcAddress(name);
}

MOBILEGL_GLX_API void* glXGetProcAddressARB(const char* name) {
    return MG_Impl::GLXImpl::GetProcAddressARB(name);
}