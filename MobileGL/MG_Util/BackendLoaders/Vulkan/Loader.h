// MobileGL - MobileGL/MG_Util/BackendLoaders/Vulkan/Loader.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MobileGL {
    namespace MG_External {
        struct VulkanCapabilities {
            Version VulkanAPIVersion{1, 0, 0};
            String DeviceName;
            String DriverVersionString;
            Int UniformBufferOffsetAlignment = 256;
            SizeT MaxShaderStorageBlockSize = 128 * 1024 * 1024;
        };
    } // namespace MG_External
    namespace MG_Util::BackendLoader {
        Bool QueryVulkanCapabilities(MobileGL::MG_External::VulkanCapabilities& caps, VkInstance instance,
                                     VkPhysicalDevice physicalDevice);
        void FillInVulkanCapabilities(MobileGL::MG_External::VulkanCapabilities& caps,
                                      VkPhysicalDeviceProperties properties);
    } // namespace MG_Util::BackendLoader
} // namespace MobileGL
