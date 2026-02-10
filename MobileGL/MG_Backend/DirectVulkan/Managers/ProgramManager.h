// MobileGL - MobileGL/MG_Backend/DirectVulkan/Managers/ProgramManager.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include "../Renderer/VulkanContext.h"
#include "../Renderer/VkCommon.h"
#include "MG_State/GLState/ProgramState/ProgramObject.h"

namespace MobileGL::MG_Backend::DirectVulkan::VkManager {
    class ProgramManager {
    public:
        using HashType = Uint64;

        explicit ProgramManager(VulkanContext& ctx) : m_ctx(ctx) {}
        ~ProgramManager();

        ProgramManager(const ProgramManager&) = delete;
        ProgramManager& operator=(const ProgramManager&) = delete;

        Vector<VkPipelineShaderStageCreateInfo>& CreatePipelineShaderStages(
            MG_State::GLState::ProgramObject* program);

    private:
        struct ProgramStages {
            HashType hash = 0;
            Vector<VkPipelineShaderStageCreateInfo> stages;
            Vector<VkShaderModule> modules;
        };

        void DestroyStages(ProgramStages& stages);
        HashType ComputeSpvHash(MG_State::GLState::ProgramObject* program) const;
        VkShaderStageFlagBits ToVkStage(ShaderStage stage) const;

        VulkanContext& m_ctx;
        UnorderedMap<const MG_State::GLState::ProgramObject*, ProgramStages> m_cache;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan::VkManager
