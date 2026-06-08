// MobileGL - MobileGL/MG_Util/BackendLoaders/Vulkan/Loader.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "Loader.h"

#include <cstdlib>
#include <cstring>

namespace MobileGL::MG_Util::BackendLoader {
    namespace {
        struct VulkanDynamicFunctions {
            PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = nullptr;
            PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2 = nullptr;
        };

        VulkanDynamicFunctions LoadVulkanDynamicFunctions(VkInstance instance) {
            VulkanDynamicFunctions loaded{};
            if (instance == VK_NULL_HANDLE) {
                MGLOG_E("Cannot load Vulkan instance-level function pointers: VkInstance is null");
                return loaded;
            }

            loaded.vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
                vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties"));
            loaded.vkGetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(
                vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2"));

            if (!loaded.vkGetPhysicalDeviceProperties) {
                MGLOG_E("Failed to resolve vkGetPhysicalDeviceProperties via vkGetInstanceProcAddr");
            }

            return loaded;
        }

        Bool IsShaderSubgroupForcedDisabled() {
            const char* value = std::getenv("MOBILEGL_DISABLE_SUBGROUP");
            if (!value) {
                return false;
            }
            return std::strcmp(value, "true") == 0 || std::strcmp(value, "TRUE") == 0;
        }
    } // namespace

    inline Version DecodeApiVersion(uint32_t version) {
        return {(Int)VK_VERSION_MAJOR(version), (Int)VK_VERSION_MINOR(version), (Int)VK_VERSION_PATCH(version)};
    }

    inline std::string DecodeDriverVersion(uint32_t driverVersion) {
        std::ostringstream oss;
        oss << VK_VERSION_MAJOR(driverVersion) << "." << VK_VERSION_MINOR(driverVersion) << "."
            << VK_VERSION_PATCH(driverVersion);
        return oss.str();
    }

    inline Bool HasUsableShaderSubgroupSupport(const VkPhysicalDeviceSubgroupProperties& subgroupProps) {
        return subgroupProps.subgroupSize > 0 &&
               (subgroupProps.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT) != 0 &&
               (subgroupProps.supportedOperations & VK_SUBGROUP_FEATURE_BASIC_BIT) != 0;
    }

    Bool QueryVulkanCapabilities(MobileGL::MG_External::VulkanCapabilities& caps, VkInstance instance,
                                 VkPhysicalDevice physicalDevice) {
        if (!physicalDevice) {
            MGLOG_E("Invalid physical device handle");
            return false;
        }

        const auto vk = LoadVulkanDynamicFunctions(instance);
        if (!vk.vkGetPhysicalDeviceProperties) {
            MGLOG_E("Vulkan dynamic loading failed for vkGetPhysicalDeviceProperties");
            return false;
        }

        VkPhysicalDeviceProperties props{};
        vk.vkGetPhysicalDeviceProperties(physicalDevice, &props);

        if (props.apiVersion < VK_API_VERSION_1_1) {
            MGLOG_E("Vulkan API version %u.%u.%u is not supported, requires at least 1.1",
                    VK_VERSION_MAJOR(props.apiVersion), VK_VERSION_MINOR(props.apiVersion),
                    VK_VERSION_PATCH(props.apiVersion));
            return false;
        }

        VkPhysicalDeviceSubgroupProperties subgroupProps{};
        subgroupProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;

        VkPhysicalDeviceProperties2 props2{};
        props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        props2.pNext = &subgroupProps;
        if (vk.vkGetPhysicalDeviceProperties2) {
            vk.vkGetPhysicalDeviceProperties2(physicalDevice, &props2);
        } else {
            MGLOG_W("vkGetPhysicalDeviceProperties2 not available, falling back to vkGetPhysicalDeviceProperties");
            props2.properties = props;
        }

        const VkPhysicalDeviceProperties& p = props2.properties;
        caps.VulkanAPIVersion = DecodeApiVersion(p.apiVersion);
        caps.DeviceName = p.deviceName;
        caps.DriverVersionString = DecodeDriverVersion(p.driverVersion);
        caps.UniformBufferOffsetAlignment = static_cast<int>(p.limits.minUniformBufferOffsetAlignment);
        caps.MaxShaderStorageBlockSize = static_cast<SizeT>(p.limits.maxStorageBufferRange);
        const Bool supportsShaderSubgroup = vk.vkGetPhysicalDeviceProperties2 &&
                                            HasUsableShaderSubgroupSupport(subgroupProps);
        const Bool forceDisableShaderSubgroup = IsShaderSubgroupForcedDisabled();
        caps.SupportsShaderSubgroup = supportsShaderSubgroup && !forceDisableShaderSubgroup;
        if (caps.SupportsShaderSubgroup) {
            caps.SubgroupSize = subgroupProps.subgroupSize;
            caps.SubgroupSupportedStages = subgroupProps.supportedStages;
            caps.SubgroupSupportedOperations = subgroupProps.supportedOperations;
            caps.SubgroupQuadOperationsInAllStages = subgroupProps.quadOperationsInAllStages == VK_TRUE;
        } else {
            caps.SubgroupSize = 0;
            caps.SubgroupSupportedStages = 0;
            caps.SubgroupSupportedOperations = 0;
            caps.SubgroupQuadOperationsInAllStages = false;
        }

        MGLOG_I("Vulkan shader subgroup support: detected=%s advertised=%s size=%u stages=0x%x operations=0x%x",
                supportsShaderSubgroup ? "true" : "false", caps.SupportsShaderSubgroup ? "true" : "false",
                subgroupProps.subgroupSize, subgroupProps.supportedStages, subgroupProps.supportedOperations);
        if (supportsShaderSubgroup && forceDisableShaderSubgroup) {
            MGLOG_W("Vulkan shader subgroup support forced off by MOBILEGL_DISABLE_SUBGROUP");
        }

        return true;
    }

    void FillInVulkanCapabilities(MobileGL::MG_External::VulkanCapabilities& caps,
                                  VkPhysicalDeviceProperties properties) {
        caps.VulkanAPIVersion = DecodeApiVersion(properties.apiVersion);
        caps.DeviceName = properties.deviceName;
        caps.DriverVersionString = DecodeDriverVersion(properties.driverVersion);
        caps.UniformBufferOffsetAlignment = static_cast<int>(properties.limits.minUniformBufferOffsetAlignment);
        caps.MaxShaderStorageBlockSize = static_cast<SizeT>(properties.limits.maxStorageBufferRange);
        caps.SupportsShaderSubgroup = false;
        caps.SubgroupSize = 0;
        caps.SubgroupSupportedStages = 0;
        caps.SubgroupSupportedOperations = 0;
        caps.SubgroupQuadOperationsInAllStages = false;
    }
} // namespace MobileGL::MG_Util::BackendLoader
