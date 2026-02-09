// MobileGL - MobileGL/MG_Backend/DirectVulkan/Managers/ProgramManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "ProgramManager.h"
#include "MG_Backend/DirectVulkan/Renderer/VulkanContext.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    ProgramManager::ProgramManager(VulkanContext &ctx): m_ctx(ctx) {}

    ProgramManager::~ProgramManager() {
        Cleanup();
    }

    void ProgramManager::Cleanup() {
        XXH64_freeState(m_hashState);
    }

    Vector<VkPipelineShaderStageCreateInfo>& ProgramManager::CreatePipelineShaderStages(ProgramObject* programObject) {
        auto hash = GetHash(programObject);
        MOBILEGL_ASSERT(m_shaderStageCreateInfo.find(hash) == m_shaderStageCreateInfo.end(),
            "A program with the same hash has already been created");
        Vector<VkPipelineShaderStageCreateInfo> info;



        return info;
    }

    ProgramManager::HashType ProgramManager::GetHash(ProgramObject *programObject) {
        MOBILEGL_ASSERT(programObject && programObject->GetLinkStatus(), "program object is null or not successfully linked!");
        MOBILEGL_ASSERT(m_hashState != nullptr, "Hash state should already be initialized");

        XXH64_hash_t const seed = MG_Config::CacheVersion;
        auto errc = XXH64_reset(m_hashState, seed);
        MOBILEGL_ASSERT(errc == XXH_OK, "Hash state reset failed");

        auto& shaders = programObject->GetAttachedShaders();
        auto& spirvBinaries = programObject->GetGeneratedSpirv();
        auto count = shaders.size();

        for (auto i = 0; i < count; i++) {
            auto& shader = shaders[i];
            auto stage = shader->GetShaderStage();
            errc = XXH64_update(m_hashState, &stage, sizeof(stage));
            MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
            errc = XXH64_update(m_hashState, spirvBinaries[i].data(), spirvBinaries[i].size() * sizeof(unsigned));
            MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
        }

        HashType hash = XXH64_digest(m_hashState);

        return hash;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan