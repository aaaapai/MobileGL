// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/ProgramFactory.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "ProgramFactory.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    ProgramFactory::~ProgramFactory() {}

    VkShaderStageFlagBits ProgramFactory::ToVkStage(ShaderStage stage) {
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

    ProgramFactory::HashType ProgramFactory::ComputeHash(const MG_State::GLState::ProgramObject& program,
                                                         CompileOptionFlags flags) const {
        XXHASH_VERIFY(XXH64_reset(m_hashState, m_config.CacheVersion));
        // We expect shader stages in program object are sorted
        const auto& spirvs = program.GetGeneratedSpirv();
        for (const auto& spv : spirvs) {
            XXHASH_VERIFY(XXH64_update(m_hashState, spv.data(), spv.size() * sizeof(Uint)));
        }
        XXHASH_VERIFY(XXH64_update(m_hashState, &flags, sizeof(CompileOptionFlags)));
        HashType hash = XXH64_digest(m_hashState);
        return hash;
    }

    Vector<VkPipelineShaderStageCreateInfo>& ProgramFactory::GetOrCreatePipelineShaderStages(
        const MG_State::GLState::ProgramObject& program, CompileOptionFlags flags) {
        auto hash = ComputeHash(program, flags);
        auto it = m_cache.find(hash);
        if (it != m_cache.end()) {
            return it->second.stages;
        }

        auto& entry = m_cache[hash];
        entry.device = m_device;
        entry.hash = hash;
        auto& shaders = program.GetAttachedShaders();
        auto& spirv = program.GetGeneratedSpirv();

        for (SizeT i = 0; i < shaders.size(); ++i) {
            auto& spv = spirv[i];
            if (spv.empty())
                continue;

            // TODO: Do SPIR-V postprocessing here

            VkShaderModuleCreateInfo smci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
            smci.codeSize = spv.size() * sizeof(Uint);
            smci.pCode = spv.data();

            VkShaderModule module = VK_NULL_HANDLE;
            VK_VERIFY(vkCreateShaderModule(m_device, &smci, nullptr, &module), "vkCreateShaderModule");

            VkPipelineShaderStageCreateInfo stage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
            ShaderStage shaderStage = shaders[i]->GetShaderStage();
            stage.stage = ToVkStage(shaderStage);
            stage.module = module;
            stage.pName = "main";

            entry.modules.push_back(module);
            entry.stages.push_back(stage);
        }

        return entry.stages;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
