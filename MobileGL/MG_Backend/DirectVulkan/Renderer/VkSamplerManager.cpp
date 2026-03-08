// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkSamplerManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VkSamplerManager.h"

#include "MG_State/GLState/Core.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    Bool VkSamplerManager::Initialize(const InitInfo& initInfo) {
        Shutdown();

        m_device = initInfo.device;
        m_config = initInfo.config;
        MOBILEGL_ASSERT(m_device != VK_NULL_HANDLE && m_config != nullptr,
                        "VkSamplerManager::Initialize failed: invalid initialization info");
        return true;
    }

    void VkSamplerManager::Shutdown() {
        for (auto& [_, sampler] : m_samplers) {
            if (m_device != VK_NULL_HANDLE && sampler.handle != VK_NULL_HANDLE) {
                vkDestroySampler(m_device, sampler.handle, nullptr);
            }
            sampler.handle = VK_NULL_HANDLE;
        }
        m_samplers.clear();

        m_device = VK_NULL_HANDLE;
        m_config = nullptr;
    }

    Uint64 VkSamplerManager::BuildSamplerKey(const MG_State::GLState::SamplerObject& sampler) const {
        MOBILEGL_ASSERT(m_config != nullptr, "VkSamplerManager::BuildSamplerKey: m_config is null");
        XXHASH_VERIFY(XXH64_reset(m_hashState, m_config->CacheVersion));

        const auto minFilter = sampler.GetMinFilter();
        XXHASH_VERIFY(XXH64_update(m_hashState, &minFilter, sizeof(minFilter)));
        const auto magFilter = sampler.GetMagFilter();
        XXHASH_VERIFY(XXH64_update(m_hashState, &magFilter, sizeof(magFilter)));
        const auto mipmapMode = sampler.GetMipmapMode();
        XXHASH_VERIFY(XXH64_update(m_hashState, &mipmapMode, sizeof(mipmapMode)));
        const auto wrapS = sampler.GetWrapS();
        XXHASH_VERIFY(XXH64_update(m_hashState, &wrapS, sizeof(wrapS)));
        const auto wrapT = sampler.GetWrapT();
        XXHASH_VERIFY(XXH64_update(m_hashState, &wrapT, sizeof(wrapT)));
        const auto wrapR = sampler.GetWrapR();
        XXHASH_VERIFY(XXH64_update(m_hashState, &wrapR, sizeof(wrapR)));
        const auto minLod = sampler.GetMinLod();
        XXHASH_VERIFY(XXH64_update(m_hashState, &minLod, sizeof(minLod)));
        const auto maxLod = sampler.GetMaxLod();
        XXHASH_VERIFY(XXH64_update(m_hashState, &maxLod, sizeof(maxLod)));
        const auto lodBias = sampler.GetLodBias();
        XXHASH_VERIFY(XXH64_update(m_hashState, &lodBias, sizeof(lodBias)));
        const auto compareMode = sampler.GetCompareMode();
        XXHASH_VERIFY(XXH64_update(m_hashState, &compareMode, sizeof(compareMode)));
        const auto compareFunc = sampler.GetSamplerCompareFunc();
        XXHASH_VERIFY(XXH64_update(m_hashState, &compareFunc, sizeof(compareFunc)));
        return XXH64_digest(m_hashState);
    }

    VkSampler VkSamplerManager::GetOrCreateSampler(const MG_State::GLState::SamplerObject& sampler) {
        const Uint64 key = BuildSamplerKey(sampler);
        auto it = m_samplers.find(key);
        if (it != m_samplers.end()) {
            return it->second.handle;
        }

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = ToVkFilter(sampler.GetMagFilter());
        samplerInfo.minFilter = ToVkFilter(sampler.GetMinFilter());
        samplerInfo.mipmapMode = ToVkMipmapMode(sampler.GetMipmapMode());
        samplerInfo.addressModeU = ToVkAddressMode(sampler.GetWrapS());
        samplerInfo.addressModeV = ToVkAddressMode(sampler.GetWrapT());
        samplerInfo.addressModeW = ToVkAddressMode(sampler.GetWrapR());
        samplerInfo.mipLodBias = sampler.GetLodBias();
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.compareEnable = sampler.GetCompareMode() == SamplerCompareMode::CompareToTexture ? VK_TRUE : VK_FALSE;
        samplerInfo.compareOp = ToVkCompareOp(sampler.GetSamplerCompareFunc());
        samplerInfo.minLod = sampler.GetMinLod();
        samplerInfo.maxLod = sampler.GetMaxLod();
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        VkSampler vkSampler = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateSampler(m_device, &samplerInfo, nullptr, &vkSampler), "vkCreateSampler(texture)");

        SamplerCacheEntry entry{};
        entry.handle = vkSampler;
        entry.externalIndex = sampler.GetExternalIndex();
        entry.version = sampler.GetVersion();
        m_samplers[key] = entry;
        return vkSampler;
    }

    VkFilter VkSamplerManager::ToVkFilter(SamplerFilterMode mode) {
        return mode == SamplerFilterMode::Nearest ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
    }

    VkSamplerMipmapMode VkSamplerManager::ToVkMipmapMode(SamplerMipmapMode mode) {
        switch (mode) {
        case SamplerMipmapMode::Nearest:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case SamplerMipmapMode::Linear:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        case SamplerMipmapMode::None:
        default:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        }
    }

    VkSamplerAddressMode VkSamplerManager::ToVkAddressMode(SamplerWrapMode mode) {
        switch (mode) {
        case SamplerWrapMode::ClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SamplerWrapMode::MirroredRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case SamplerWrapMode::Repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerWrapMode::ClampToBorder:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case SamplerWrapMode::MirrorClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        default:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    VkCompareOp VkSamplerManager::ToVkCompareOp(SamplerCompareFunc func) {
        switch (func) {
        case SamplerCompareFunc::Never:
            return VK_COMPARE_OP_NEVER;
        case SamplerCompareFunc::Less:
            return VK_COMPARE_OP_LESS;
        case SamplerCompareFunc::Equal:
            return VK_COMPARE_OP_EQUAL;
        case SamplerCompareFunc::LessEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case SamplerCompareFunc::Greater:
            return VK_COMPARE_OP_GREATER;
        case SamplerCompareFunc::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case SamplerCompareFunc::GreaterEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case SamplerCompareFunc::Always:
        default:
            return VK_COMPARE_OP_ALWAYS;
        }
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
