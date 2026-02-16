// MobileGL - MobileGL/MG_Backend/DirectVulkan/VkIncludes.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#define VK_VERIFY(expr, ...)                                                                                           \
    do {                                                                                                               \
        VkResult _vk_verify_result = (expr);                                                                           \
        MOBILEGL_ASSERT(_vk_verify_result == VK_SUCCESS, "Vulkan error %d at %s:%d" __VA_OPT__(" - ") __VA_ARGS__, _vk_verify_result, __FILE__, __LINE__);  \
    } while (0)

#define ENUM_STR_CASE(c) case c: return #c;