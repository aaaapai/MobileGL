// MobileGL - MobileGL/MG_Backend/DirectVulkan/Managers/ProgramManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "ProgramManager.h"

namespace MobileGL::MG_Backend::DirectVulkan::VkManager {
    ProgramManager::~ProgramManager() {
        for (auto& [_, stages] : m_cache) {
            DestroyStages(stages);
        }
        m_cache.clear();
    }

    ProgramManager::HashType ProgramManager::ComputeSpvHash(MG_State::GLState::ProgramObject* program) const {
        if (!program) return 0;
        XXH64_state_t* state = XXH64_createState();
        XXH64_reset(state, 0xC0FFEEu);
        auto& spirvs = program->GetGeneratedSpirv();
        for (const auto& spv : spirvs) {
            if (spv.empty()) continue;
            XXH64_update(state, spv.data(), spv.size() * sizeof(Uint));
        }
        HashType hash = XXH64_digest(state);
        XXH64_freeState(state);
        return hash;
    }

    VkShaderStageFlagBits ProgramManager::ToVkStage(ShaderStage stage) const {
        switch (stage) {
        case ShaderStage::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::Geometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderStage::TessControl:
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderStage::TessEval:
            return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case ShaderStage::Compute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        default:
            return VK_SHADER_STAGE_ALL_GRAPHICS;
        }
    }

    void ProgramManager::DestroyStages(ProgramStages& stages) {
        for (auto module : stages.modules) {
            if (module != VK_NULL_HANDLE) {
                vkDestroyShaderModule(m_ctx.GetDevice(), module, nullptr);
            }
        }
        stages.modules.clear();
        stages.stages.clear();
        stages.hash = 0;
    }

    Vector<VkPipelineShaderStageCreateInfo>& ProgramManager::CreatePipelineShaderStages(
        MG_State::GLState::ProgramObject* program) {
        auto& entry = m_cache[program];
        HashType newHash = ComputeSpvHash(program);
        if (!entry.stages.empty() && entry.hash == newHash) return entry.stages;

        DestroyStages(entry);
        entry.hash = newHash;

        if (!program) return entry.stages;

        auto& spirvs = program->GetGeneratedSpirv();
        auto& shaders = program->GetAttachedShaders();

        for (SizeT i = 0; i < spirvs.size(); ++i) {
            auto& spv = spirvs[i];
            if (spv.empty()) continue;

            VkShaderModuleCreateInfo smci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
            smci.codeSize = spv.size() * sizeof(Uint);
            smci.pCode = spv.data();

            VkShaderModule module = VK_NULL_HANDLE;
            VK_VERIFY(vkCreateShaderModule(m_ctx.GetDevice(), &smci, nullptr, &module), "vkCreateShaderModule");

            VkPipelineShaderStageCreateInfo stage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
            ShaderStage shaderStage = ShaderStage::Unknown;
            if (i < shaders.size()) shaderStage = shaders[i]->GetShaderStage();
            stage.stage = ToVkStage(shaderStage);
            stage.module = module;
            stage.pName = "main";

            entry.modules.push_back(module);
            entry.stages.push_back(stage);
        }

        return entry.stages;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan::VkManager
