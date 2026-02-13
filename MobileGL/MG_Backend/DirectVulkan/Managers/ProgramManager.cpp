// MobileGL - MobileGL/MG_Backend/DirectVulkan/Managers/ProgramManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "ProgramManager.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    ProgramManager::ProgramManager(VulkanContext &ctx): m_ctx(ctx) {}

    ProgramManager::~ProgramManager() {
        Cleanup();
    }

    void ProgramManager::Cleanup() {
        auto device = m_ctx.GetDevice();
        if (device != VK_NULL_HANDLE) {
            for (auto& [_, stages] : m_shaderStageCreateInfo) {
                for (auto& stageInfo : stages) {
                    if (stageInfo.module != VK_NULL_HANDLE) {
                        vkDestroyShaderModule(device, stageInfo.module, nullptr);
                        stageInfo.module = VK_NULL_HANDLE;
                    }
                }
            }
        }
        m_shaderStageCreateInfo.clear();
        XXH64_freeState(m_hashState);
    }

    Vector<VkPipelineShaderStageCreateInfo>& ProgramManager::CreatePipelineShaderStages(ProgramObject* programObject) {
        auto hash = GetHash(programObject);
        MOBILEGL_ASSERT(m_shaderStageCreateInfo.find(hash) == m_shaderStageCreateInfo.end(),
            "A program with the same hash has already been created");
        Vector<VkPipelineShaderStageCreateInfo> info;
        auto& shaders = programObject->GetAttachedShaders();
        auto& spirvBinaries = programObject->GetGeneratedSpirv();
        MOBILEGL_ASSERT(shaders.size() == spirvBinaries.size(),
            "Shader and SPIR-V binary count mismatch");

        info.reserve(shaders.size());

        auto device = m_ctx.GetDevice();
        MOBILEGL_ASSERT(device != VK_NULL_HANDLE, "Vulkan device is null");

        for (SizeT i = 0; i < shaders.size(); ++i) {
            auto shaderStage = shaders[i]->GetShaderStage();
            VkShaderStageFlagBits vkStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
            switch (shaderStage) {
                case ShaderStage::Vertex:
                    vkStage = VK_SHADER_STAGE_VERTEX_BIT;
                    break;
                case ShaderStage::TessControl:
                    vkStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    break;
                case ShaderStage::TessEval:
                    vkStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    break;
                case ShaderStage::Geometry:
                    vkStage = VK_SHADER_STAGE_GEOMETRY_BIT;
                    break;
                case ShaderStage::Fragment:
                    vkStage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    break;
                case ShaderStage::Compute:
                    vkStage = VK_SHADER_STAGE_COMPUTE_BIT;
                    break;
                default:
                    MOBILEGL_ASSERT(false, "Unsupported shader stage");
                    break;
            }

            auto& spirv = spirvBinaries[i];
            MOBILEGL_ASSERT(!spirv.empty(), "SPIR-V binary is empty");
            VkShaderModuleCreateInfo smci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
            smci.codeSize = spirv.size() * sizeof(Uint);
            smci.pCode = reinterpret_cast<const uint32_t*>(spirv.data());

            VkShaderModule shaderModule = VK_NULL_HANDLE;
            VK_VERIFY(vkCreateShaderModule(device, &smci, nullptr, &shaderModule), "vkCreateShaderModule");
            MOBILEGL_ASSERT(shaderModule != VK_NULL_HANDLE, "Failed to create shader module");

            VkPipelineShaderStageCreateInfo stageInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
            stageInfo.stage = vkStage;
            stageInfo.module = shaderModule;
            stageInfo.pName = "main";
            info.emplace_back(stageInfo);
        }

        auto [it, inserted] = m_shaderStageCreateInfo.emplace(hash, Move(info));
        MOBILEGL_ASSERT(inserted, "Failed to cache shader stage create info");
        return it->second;
    }

    Vector<VkPipelineShaderStageCreateInfo>* ProgramManager::GetPipelineShaderStages(HashType hash) {
        auto it = m_shaderStageCreateInfo.find(hash);
        if (it == m_shaderStageCreateInfo.end()) return nullptr;
        return &it->second;
    }

    Vector<VkPipelineShaderStageCreateInfo>* ProgramManager::GetPipelineShaderStages(ProgramObject* programObject) {
        return GetPipelineShaderStages(GetHash(programObject));
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
