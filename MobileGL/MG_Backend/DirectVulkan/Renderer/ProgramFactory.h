// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/ProgramFactory.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "../VkIncludes.h"
#include "MG_State/GLState/ProgramState/ProgramObject.h"
#include "MG_State/GLState/ProgramState/ShaderObject.h"
#include <Includes.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    class ProgramFactory {
    public:
        enum class CompileOptionBit : Uint {
            None = 0,
            PositionYFlip = 1 << 0,
            PositionZRemap = 1 << 1,
            SurfaceRotate90 = 1 << 2,
            SurfaceRotate180 = 1 << 3,
            SurfaceRotate270 = 1 << 4,
        };
        using CompileOptionFlags = Flags<CompileOptionBit>;
        using HashType = Uint64;
        struct BackendProgramObject {
            HashType hash = 0;
            VkDevice device = VK_NULL_HANDLE;
            Vector<VkPipelineShaderStageCreateInfo> stages;
            Vector<VkShaderModule> modules;

            BackendProgramObject() = default;
            BackendProgramObject(const BackendProgramObject&) = delete;
            BackendProgramObject& operator=(const BackendProgramObject&) = delete;
            BackendProgramObject(BackendProgramObject&& other) noexcept {
                hash = other.hash;
                device = other.device;
                stages = std::move(other.stages);
                modules = std::move(other.modules);
                other.hash = 0;
                other.device = VK_NULL_HANDLE;
            }
            BackendProgramObject& operator=(BackendProgramObject&& other) noexcept {
                if (this == &other) {
                    return *this;
                }
                DestroyModules();
                stages.clear();
                hash = other.hash;
                device = other.device;
                stages = std::move(other.stages);
                modules = std::move(other.modules);
                other.hash = 0;
                other.device = VK_NULL_HANDLE;
                return *this;
            }

            ~BackendProgramObject() {
                DestroyModules();
                stages.clear();
            }

        private:
            void DestroyModules() {
                for (auto module : modules) {
                    if (module != VK_NULL_HANDLE && device != VK_NULL_HANDLE) {
                        vkDestroyShaderModule(device, module, nullptr);
                    }
                }
                modules.clear();
            }
        };

        explicit ProgramFactory(VkDevice device, const VulkanRendererConfig& config)
            : m_device(device), m_config(config) {}
        ~ProgramFactory();
        ProgramFactory(const ProgramFactory&) = delete;

        HashType ComputeHash(const MG_State::GLState::ProgramObject& program, CompileOptionFlags flags) const;
        Vector<VkPipelineShaderStageCreateInfo>& GetOrCreatePipelineShaderStages(
            const MG_State::GLState::ProgramObject& program, CompileOptionFlags flags);

        static VkShaderStageFlagBits ToVkStage(ShaderStage stage);

    private:
        VkDevice m_device = VK_NULL_HANDLE;
        UnorderedMap<HashType, BackendProgramObject> m_cache;
        const VulkanRendererConfig& m_config;
        static inline XXH64_state_t* m_hashState = XXH64_createState();
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
