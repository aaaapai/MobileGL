// MobileGL - MobileGL/MG_Backend/DirectVulkan/Managers/VertexInputStateManager.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include "Includes.h"
#include "MG_State/GLState/VertexArrayState/VertexArrayObject.h"
#include "Config.h"
#include "xxhash.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    class VertexInputStateManager {
    public:
        using VertexArrayObject = MobileGL::MG_State::GLState::VertexArrayObject;
        using HashType = XXH64_hash_t;

        struct VertexInputStateInfo {
            VkPipelineVertexInputStateCreateInfo CreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
            Vector<VkVertexInputBindingDescription> BindingDescriptions;
            Vector<VkVertexInputAttributeDescription> AttributeDescriptions;
        };

        VertexInputStateManager();
        ~VertexInputStateManager();

        VertexInputStateInfo& CreatePipelineVertexInputState(VertexArrayObject* vaoObject);
        VertexInputStateInfo* GetPipelineVertexInputState(HashType hash);
        VertexInputStateInfo* GetPipelineVertexInputState(VertexArrayObject* vaoObject);

    private:
        void Cleanup();
        static void PatchCreateInfoPointers(VertexInputStateInfo& stateInfo);
        HashType GetHash(VertexArrayObject* vaoObject);
        static VkFormat ResolveVertexFormat(const MG_State::GLState::VertexAttribute& attrib);
        static Uint GetDataTypeByteSize(DataType type);

        UnorderedMap<HashType, VertexInputStateInfo> m_vertexInputStateInfo;
        XXH64_state_t* const m_hashState = XXH64_createState();
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
