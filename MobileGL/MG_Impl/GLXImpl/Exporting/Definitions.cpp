// MobileGL - MobileGL/MG_Impl/GLXImpl/Exporting/Definitions.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include <Includes.h>
#include "../LookUp/LookUp.h"

MOBILEGL_GLX_API void* glXGetProcAddress(const char* name) {
    return MG_Impl::GLXImpl::GetProcAddress(name);
}

MOBILEGL_GLX_API void* glXGetProcAddressARB(const char* name) {
    return MG_Impl::GLXImpl::GetProcAddressARB(name);
}