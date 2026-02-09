// MobileGL - MobileGL/MG_Backend/DirectVulkan/Managers/ProgramManager.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include "Includes.h"
#include "MG_State/GLState/ProgramState/ProgramObject.h"
#include "Config.h"
#include "xxhash.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    class VulkanContext;

    class ProgramManager {
    public:
        using ProgramObject = MobileGL::MG_State::GLState::ProgramObject;
        using HashType = XXH64_hash_t;
        ProgramManager(VulkanContext& ctx);
        ~ProgramManager();

        Vector<VkPipelineShaderStageCreateInfo>& CreatePipelineShaderStages(ProgramObject* programObject);
    private:

        void Cleanup();
        HashType GetHash(ProgramObject* programObject);
        VulkanContext& m_ctx;
        UnorderedMap<HashType, Vector<VkPipelineShaderStageCreateInfo>> m_shaderStageCreateInfo;
        XXH64_state_t* const m_hashState = XXH64_createState();
    };
} // namespace MobileGL::MG_Backend::DirectVulkan