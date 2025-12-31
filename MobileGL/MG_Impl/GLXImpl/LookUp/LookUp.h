// MobileGL - MobileGL/MG_Impl/GLXImpl/LookUp/LookUp.h
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MG_Impl {
    namespace GLXImpl {
        void* GetProcAddress(const char* name);
        void* GetProcAddressARB(const char* name);
    } // namespace GLXImpl
} // namespace MG_Impl