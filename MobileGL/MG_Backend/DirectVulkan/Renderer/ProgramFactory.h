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
#include "MG_State/GLState/TextureState/TextureEnum.h"

#include <Includes.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    class ProgramFactory {
    public:
        enum class DescriptorBindingKind : Uint8 {
            None = 0,
            UniformBufferDynamic,
            CombinedImageSampler
        };

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

        struct VkProgramObject {
            HashType hash = 0;
            Vector<VkPipelineShaderStageCreateInfo> stages;
            Vector<VkShaderModule> modules;
            static inline VkDevice s_device = VK_NULL_HANDLE;

            VkProgramObject() = default;
            VkProgramObject(const VkProgramObject&) = delete;
            VkProgramObject& operator=(const VkProgramObject&) = delete;
            VkProgramObject(VkProgramObject&& other) noexcept {
                hash = other.hash;
                stages = std::move(other.stages);
                modules = std::move(other.modules);
                other.hash = 0;
            }
            VkProgramObject& operator=(VkProgramObject&& other) noexcept {
                if (this == &other) {
                    return *this;
                }
                DestroyModules();
                stages.clear();
                hash = other.hash;
                stages = std::move(other.stages);
                modules = std::move(other.modules);
                other.hash = 0;
                return *this;
            }

            ~VkProgramObject() {
                DestroyModules();
                stages.clear();
            }

        private:
            void DestroyModules() {
                for (auto module : modules) {
                    if (module != VK_NULL_HANDLE && s_device != VK_NULL_HANDLE) {
                        vkDestroyShaderModule(s_device, module, nullptr);
                    }
                }
                modules.clear();
            }
        };

        struct VkProgramLayout {
            HashType hash = 0;
            VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
            VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
            Vector<DescriptorBindingKind> bindingKinds;
            Vector<Uint32> dynamicBindings;
            Vector<Int> samplerUniformLocationByBinding;
            Vector<TextureTarget> samplerTextureTargetByBinding;
            Int globalUboBinding = -1;
        };

        explicit ProgramFactory(VkDevice device, const VulkanRendererConfig& config, Uint32 maxBindings = 16)
            : m_device(device), m_config(config), m_maxBindings(maxBindings) {
            VkProgramObject::s_device = device;
        }
        ~ProgramFactory();
        ProgramFactory(const ProgramFactory&) = delete;

        HashType ComputeHash(const MG_State::GLState::ProgramObject& program, CompileOptionFlags flags) const;
        Vector<VkPipelineShaderStageCreateInfo>& GetOrCreatePipelineShaderStages(
            const MG_State::GLState::ProgramObject& program, CompileOptionFlags flags);
        const VkProgramLayout* GetOrCreateProgramLayout(const MG_State::GLState::ProgramObject& program);
        VkPipelineLayout GetOrCreatePipelineLayout(const MG_State::GLState::ProgramObject& program);

        static VkShaderStageFlagBits ToVkStage(ShaderStage stage);

    private:
        HashType ComputeLayoutHash(const MG_State::GLState::ProgramObject& program) const;
        static TextureTarget UniformTypeToTextureTarget(GLenum glType);
        Bool ReflectBindingKinds(const MG_State::GLState::ProgramObject& program,
                                 Vector<DescriptorBindingKind>& outKinds) const;
        Bool ReflectSamplerBindings(const MG_State::GLState::ProgramObject& program, VkProgramLayout& layout) const;
        Bool ReflectGlobalUboBinding(const MG_State::GLState::ProgramObject& program, VkProgramLayout& layout) const;
        void DestroyLayoutCache();

        VkDevice m_device = VK_NULL_HANDLE;
        Uint32 m_maxBindings = 0;
        UnorderedMap<HashType, VkProgramObject> m_cache;
        UnorderedMap<HashType, VkProgramLayout> m_layoutCache;
        const VulkanRendererConfig& m_config;
        static inline XXH64_state_t* m_hashState = XXH64_createState();
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
