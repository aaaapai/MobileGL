// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/BufferSlice.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "../VkIncludes.h"
#include <Includes.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    struct BufferSlice {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceSize offset = 0;
        VkDeviceSize size = 0;
        void* mapped = nullptr;

        Bool IsValid() const { return buffer != VK_NULL_HANDLE; }
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
