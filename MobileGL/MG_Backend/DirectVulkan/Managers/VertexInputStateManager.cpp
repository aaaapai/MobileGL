// MobileGL - MobileGL/MG_Backend/DirectVulkan/Managers/VertexInputStateManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VertexInputStateManager.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    VertexInputStateManager::VertexInputStateManager() {}

    VertexInputStateManager::~VertexInputStateManager() {
        Cleanup();
    }

    void VertexInputStateManager::Cleanup() {
        m_vertexInputStateInfo.clear();
        XXH64_freeState(m_hashState);
    }

    void VertexInputStateManager::PatchCreateInfoPointers(VertexInputStateInfo& stateInfo) {
        stateInfo.CreateInfo.vertexBindingDescriptionCount = static_cast<Uint>(stateInfo.BindingDescriptions.size());
        stateInfo.CreateInfo.pVertexBindingDescriptions =
            stateInfo.BindingDescriptions.empty() ? nullptr : stateInfo.BindingDescriptions.data();
        stateInfo.CreateInfo.vertexAttributeDescriptionCount = static_cast<Uint>(stateInfo.AttributeDescriptions.size());
        stateInfo.CreateInfo.pVertexAttributeDescriptions =
            stateInfo.AttributeDescriptions.empty() ? nullptr : stateInfo.AttributeDescriptions.data();
    }

    VertexInputStateManager::VertexInputStateInfo&
    VertexInputStateManager::CreatePipelineVertexInputState(VertexArrayObject* vaoObject) {
        auto hash = GetHash(vaoObject);
        MOBILEGL_ASSERT(m_vertexInputStateInfo.find(hash) == m_vertexInputStateInfo.end(),
                        "A VAO vertex input state with the same hash has already been created");

        VertexInputStateInfo stateInfo;
        const auto& allAttributes = vaoObject->GetAllAttributes();

        for (Uint attribIndex = 0; attribIndex < allAttributes.size(); ++attribIndex) {
            const auto& attrib = allAttributes[attribIndex];
            if (!attrib.Enabled) continue;

            MOBILEGL_ASSERT(attrib.Size >= 1 && attrib.Size <= 4, "Vertex attribute size out of range");
            MOBILEGL_ASSERT(attrib.Divisor <= 1,
                            "Vulkan core pipeline vertex input only supports divisor 0/1 without extra extensions");

            VkVertexInputBindingDescription bindingDesc{};
            bindingDesc.binding = attribIndex;
            bindingDesc.inputRate = (attrib.Divisor == 0) ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;

            auto componentByteSize = GetDataTypeByteSize(attrib.Type);
            auto computedStride = static_cast<Uint>(componentByteSize * static_cast<Uint>(attrib.Size));
            bindingDesc.stride = (attrib.Stride > 0) ? static_cast<Uint>(attrib.Stride) : computedStride;
            stateInfo.BindingDescriptions.emplace_back(bindingDesc);

            VkVertexInputAttributeDescription attrDesc{};
            attrDesc.location = attribIndex;
            attrDesc.binding = attribIndex;
            attrDesc.format = ResolveVertexFormat(attrib);
            attrDesc.offset = static_cast<Uint>(attrib.Offset);
            stateInfo.AttributeDescriptions.emplace_back(attrDesc);
        }

        PatchCreateInfoPointers(stateInfo);

        auto [it, inserted] = m_vertexInputStateInfo.emplace(hash, Move(stateInfo));
        MOBILEGL_ASSERT(inserted, "Failed to cache pipeline vertex input state create info");
        PatchCreateInfoPointers(it->second);
        return it->second;
    }

    VertexInputStateManager::VertexInputStateInfo* VertexInputStateManager::GetPipelineVertexInputState(HashType hash) {
        auto it = m_vertexInputStateInfo.find(hash);
        if (it == m_vertexInputStateInfo.end()) return nullptr;
        return &it->second;
    }

    VertexInputStateManager::VertexInputStateInfo*
    VertexInputStateManager::GetPipelineVertexInputState(VertexArrayObject* vaoObject) {
        return GetPipelineVertexInputState(GetHash(vaoObject));
    }

    VertexInputStateManager::HashType VertexInputStateManager::GetHash(VertexArrayObject* vaoObject) {
        MOBILEGL_ASSERT(vaoObject != nullptr, "vao object is null");
        MOBILEGL_ASSERT(m_hashState != nullptr, "Hash state should already be initialized");

        XXH64_hash_t const seed = MG_Config::CacheVersion;
        auto errc = XXH64_reset(m_hashState, seed);
        MOBILEGL_ASSERT(errc == XXH_OK, "Hash state reset failed");

        const auto& allAttributes = vaoObject->GetAllAttributes();
        for (const auto& attrib : allAttributes) {
            errc = XXH64_update(m_hashState, &attrib.Enabled, sizeof(attrib.Enabled));
            MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
            errc = XXH64_update(m_hashState, &attrib.Size, sizeof(attrib.Size));
            MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
            errc = XXH64_update(m_hashState, &attrib.Type, sizeof(attrib.Type));
            MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
            errc = XXH64_update(m_hashState, &attrib.Normalized, sizeof(attrib.Normalized));
            MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
            errc = XXH64_update(m_hashState, &attrib.Stride, sizeof(attrib.Stride));
            MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
            errc = XXH64_update(m_hashState, &attrib.Offset, sizeof(attrib.Offset));
            MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
            errc = XXH64_update(m_hashState, &attrib.IsInteger, sizeof(attrib.IsInteger));
            MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
            errc = XXH64_update(m_hashState, &attrib.Divisor, sizeof(attrib.Divisor));
            MOBILEGL_ASSERT(errc == XXH_OK, "Hash state update failed");
        }

        return XXH64_digest(m_hashState);
    }

    Uint VertexInputStateManager::GetDataTypeByteSize(DataType type) {
        switch (type) {
        case DataType::Int8:
        case DataType::Uint8:
            return 1;
        case DataType::Int16:
        case DataType::Uint16:
        case DataType::Float16:
            return 2;
        case DataType::Int32:
        case DataType::Uint32:
        case DataType::Float32:
        case DataType::Fixed32:
            return 4;
        case DataType::Float64:
            return 8;
        default:
            MOBILEGL_ASSERT(false, "Unsupported vertex data type");
            return 0;
        }
    }

    VkFormat VertexInputStateManager::ResolveVertexFormat(const MG_State::GLState::VertexAttribute& attrib) {
        auto size = attrib.Size;

        auto selectFormat = [size](VkFormat c1, VkFormat c2, VkFormat c3, VkFormat c4) {
            switch (size) {
            case 1:
                return c1;
            case 2:
                return c2;
            case 3:
                return c3;
            case 4:
                return c4;
            default:
                return VK_FORMAT_UNDEFINED;
            }
        };

        if (attrib.IsInteger) {
            MOBILEGL_ASSERT(!attrib.Normalized, "Integer attributes cannot be normalized in Vulkan integer pipeline IO");
            switch (attrib.Type) {
            case DataType::Int8:
                return selectFormat(VK_FORMAT_R8_SINT, VK_FORMAT_R8G8_SINT, VK_FORMAT_R8G8B8_SINT,
                                    VK_FORMAT_R8G8B8A8_SINT);
            case DataType::Uint8:
                return selectFormat(VK_FORMAT_R8_UINT, VK_FORMAT_R8G8_UINT, VK_FORMAT_R8G8B8_UINT,
                                    VK_FORMAT_R8G8B8A8_UINT);
            case DataType::Int16:
                return selectFormat(VK_FORMAT_R16_SINT, VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16B16_SINT,
                                    VK_FORMAT_R16G16B16A16_SINT);
            case DataType::Uint16:
                return selectFormat(VK_FORMAT_R16_UINT, VK_FORMAT_R16G16_UINT, VK_FORMAT_R16G16B16_UINT,
                                    VK_FORMAT_R16G16B16A16_UINT);
            case DataType::Int32:
                return selectFormat(VK_FORMAT_R32_SINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32B32_SINT,
                                    VK_FORMAT_R32G32B32A32_SINT);
            case DataType::Uint32:
                return selectFormat(VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32B32_UINT,
                                    VK_FORMAT_R32G32B32A32_UINT);
            default:
                MOBILEGL_ASSERT(false, "Unsupported integer vertex attribute data type");
                return VK_FORMAT_UNDEFINED;
            }
        }

        if (attrib.Normalized) {
            switch (attrib.Type) {
            case DataType::Int8:
                return selectFormat(VK_FORMAT_R8_SNORM, VK_FORMAT_R8G8_SNORM, VK_FORMAT_R8G8B8_SNORM,
                                    VK_FORMAT_R8G8B8A8_SNORM);
            case DataType::Uint8:
                return selectFormat(VK_FORMAT_R8_UNORM, VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8B8_UNORM,
                                    VK_FORMAT_R8G8B8A8_UNORM);
            case DataType::Int16:
                return selectFormat(VK_FORMAT_R16_SNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16B16_SNORM,
                                    VK_FORMAT_R16G16B16A16_SNORM);
            case DataType::Uint16:
                return selectFormat(VK_FORMAT_R16_UNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16B16_UNORM,
                                    VK_FORMAT_R16G16B16A16_UNORM);
            default:
                MOBILEGL_ASSERT(false, "Unsupported normalized vertex attribute data type");
                return VK_FORMAT_UNDEFINED;
            }
        }

        switch (attrib.Type) {
        case DataType::Float16:
            return selectFormat(VK_FORMAT_R16_SFLOAT, VK_FORMAT_R16G16_SFLOAT, VK_FORMAT_R16G16B16_SFLOAT,
                                VK_FORMAT_R16G16B16A16_SFLOAT);
        case DataType::Float32:
            return selectFormat(VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT,
                                VK_FORMAT_R32G32B32A32_SFLOAT);
        case DataType::Int8:
            return selectFormat(VK_FORMAT_R8_SSCALED, VK_FORMAT_R8G8_SSCALED, VK_FORMAT_R8G8B8_SSCALED,
                                VK_FORMAT_R8G8B8A8_SSCALED);
        case DataType::Uint8:
            return selectFormat(VK_FORMAT_R8_USCALED, VK_FORMAT_R8G8_USCALED, VK_FORMAT_R8G8B8_USCALED,
                                VK_FORMAT_R8G8B8A8_USCALED);
        case DataType::Int16:
            return selectFormat(VK_FORMAT_R16_SSCALED, VK_FORMAT_R16G16_SSCALED, VK_FORMAT_R16G16B16_SSCALED,
                                VK_FORMAT_R16G16B16A16_SSCALED);
        case DataType::Uint16:
            return selectFormat(VK_FORMAT_R16_USCALED, VK_FORMAT_R16G16_USCALED, VK_FORMAT_R16G16B16_USCALED,
                                VK_FORMAT_R16G16B16A16_USCALED);
        default:
            MOBILEGL_ASSERT(false, "Unsupported floating-point vertex attribute data type");
            return VK_FORMAT_UNDEFINED;
        }
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
