// MobileGL - MobileGL/MG_Impl/GLXImpl/LookUp/LookUp.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include "MG_Impl/GetProcAddress.h"

namespace MG_Impl {
    namespace GLXImpl {
        void* GetProcAddress(const char* name);
        void* GetProcAddressARB(const char* name);
    } // namespace GLXImpl
} // namespace MG_Impl