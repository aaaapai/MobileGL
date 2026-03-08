// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkSamplerManager.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "../VkIncludes.h"
#include "../VulkanRendererConfig.h"
#include <Includes.h>
#include <MG_State/GLState/SamplerState/SamplerObject.h>

namespace MobileGL::MG_State::GLState {
class SamplerObject;
}

namespace MobileGL::MG_Backend::DirectVulkan {
class VkSamplerManager {
public:
    struct InitInfo {
        VkDevice device = VK_NULL_HANDLE;
        const VulkanRendererConfig* config = nullptr;
    };

    Bool Initialize(const InitInfo& initInfo);
    void Shutdown();

    VkSampler GetOrCreateSampler(const MG_State::GLState::SamplerObject& sampler);

private:
    struct SamplerCacheEntry {
        VkSampler handle = VK_NULL_HANDLE;
        Uint externalIndex = 0;
        Uint16 version = 0;
    };

    Uint64 BuildSamplerKey(const MG_State::GLState::SamplerObject& sampler) const;
    static VkFilter ToVkFilter(SamplerFilterMode mode);
    static VkSamplerMipmapMode ToVkMipmapMode(SamplerMipmapMode mode);
    static VkSamplerAddressMode ToVkAddressMode(SamplerWrapMode mode);
    static VkCompareOp ToVkCompareOp(SamplerCompareFunc func);

    VkDevice m_device = VK_NULL_HANDLE;
    const VulkanRendererConfig* m_config = nullptr;
    UnorderedMap<Uint64, SamplerCacheEntry> m_samplers;
    static inline XXH64_state_t* m_hashState = XXH64_createState();
};
} // namespace MobileGL::MG_Backend::DirectVulkan
