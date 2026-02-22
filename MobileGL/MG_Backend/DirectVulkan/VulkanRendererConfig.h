// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VulkanRendererConfig.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include "Config.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    struct VulkanRendererConfig {
        Uint32 MaxFramesInFlight = 2;
        String AppName = "MobileGL-VulkanRenderer";
        Version Version = MG_Config::CoreVersion;
        Uint64 CacheVersion = MG_Config::CacheVersion;
        Bool EnableValidationLayers = true;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan