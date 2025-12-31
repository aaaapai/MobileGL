// MobileGL - MobileGL/Config.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_Backend/Backends.h>

namespace MobileGL {
    namespace MG_Config {
        inline const String ProjectName = "MobileGL";
        inline const String CoreName = "MobileGL Core";
        inline const String CoreVendor = "MobileGL-Dev";
        inline const Version CoreVersion = {26, 1, 0, "-dev", VersionType::Development};
        inline const VersionStringFormatAttrib DefaultVersionStringFormatAttrib = {2, 2, 0, true, true};
        extern UniquePtr<RendererInfo> RendererInfoPtr;

        namespace Backend {
#define MOBILEGL_BACKEND MOBILEGL_BACKEND_TYPE_DIRECT_GLES

            namespace Diligent {
                inline const MG_Backend::Diligent::SpecificBackendType SpecificBackend =
                    MG_Backend::Diligent::SpecificBackendType::Vulkan;
            }
        } // namespace Backend
    } // namespace MG_Config
} // namespace MobileGL