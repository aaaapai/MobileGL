// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkClearManager.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once

#include "../VkIncludes.h"
#include "../VulkanRendererConfig.h"
#include "MG_State/GLState/FramebufferState/FramebufferObject.h"
#include "MG_Util/Math/VectorTypes.h"

#include <Includes.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    struct ClearFramebufferPayload {
        FloatVec4 color;
        Float depth{};
        Uint32 stencil{};
    };

    struct ClearAttachmentPayload {
        GLbitfield mask = 0;
        FloatVec4 color = FloatVec4(0.0f, 0.0f, 0.0f, 0.0f);
        Float depth = 1.0f;
        Uint32 stencil = 0;
    };

    struct PendingClearKey {
        MG_State::GLState::ITextureObject* texture = nullptr;
        Uint32 mipLevel = 0;
        Uint32 baseArrayLayer = 0;
        Uint32 layerCount = 1;

        Bool operator==(const PendingClearKey& other) const {
            return texture == other.texture && mipLevel == other.mipLevel &&
                   baseArrayLayer == other.baseArrayLayer && layerCount == other.layerCount;
        }
    };

    struct PendingClearEntry {
        PendingClearKey key{};
        ClearAttachmentPayload payload{};
    };

    struct PendingClearKeyHash {
        SizeT operator()(const PendingClearKey& key) const {
            const SizeT textureHash = std::hash<MG_State::GLState::ITextureObject*>{}(key.texture);
            const SizeT mipHash = std::hash<Uint32>{}(key.mipLevel);
            const SizeT layerHash = std::hash<Uint32>{}(key.baseArrayLayer);
            const SizeT layerCountHash = std::hash<Uint32>{}(key.layerCount);
            SizeT hash = textureHash;
            hash ^= mipHash + 0x9e3779b9u + (hash << 6) + (hash >> 2);
            hash ^= layerHash + 0x9e3779b9u + (hash << 6) + (hash >> 2);
            hash ^= layerCountHash + 0x9e3779b9u + (hash << 6) + (hash >> 2);
            return hash;
        }
    };

    class VkClearManager {
    public:
        static PendingClearKey MakePendingClearKey(const MG_State::GLState::FramebufferAttachmentObject& attachment);
        static PendingClearKey MakePendingClearKey(MG_State::GLState::ITextureObject* texture, Uint32 mipLevel = 0,
                                                   Uint32 baseArrayLayer = 0, Uint32 layerCount = 1);

        Bool Initialize();
        void Shutdown();

        void QueueClear(GLbitfield mask, const ClearFramebufferPayload& clearPayload, const MG_State::GLState::FramebufferObject& drawFbo);
        void QueueClear(
            const ClearAttachmentPayload& clearPayload,
            const SharedPtr<MG_State::GLState::ITextureObject>& texture);
        void QueueClear(const ClearAttachmentPayload& clearPayload,
                        const MG_State::GLState::FramebufferAttachmentObject& attachment);
        Bool HasPendingClear(MG_State::GLState::ITextureObject* texture);
        Bool HasPendingClear(const PendingClearKey& key);
        Bool HasPendingClear(const MG_State::GLState::FramebufferAttachmentObject& attachment);
        Bool GetPendingClear(const PendingClearKey& key, ClearAttachmentPayload& outPayload);
        Bool GetPendingClear(const MG_State::GLState::FramebufferAttachmentObject& attachment,
                             ClearAttachmentPayload& outPayload);
        Bool GetPendingClears(MG_State::GLState::ITextureObject* texture, Vector<PendingClearEntry>& outEntries);
        void PopPendingClear(MG_State::GLState::ITextureObject* texture);
        void PopPendingClear(const PendingClearKey& key);
        void PopPendingClear(const MG_State::GLState::FramebufferAttachmentObject& attachment);
        SizeT CollectGarbage();
    private:
        Uint8 m_gcCounter = 0;
        UnorderedMap<PendingClearKey, ClearAttachmentPayload, PendingClearKeyHash> m_pendingClears;
        UnorderedMap<MG_State::GLState::ITextureObject*, WeakPtr<MG_State::GLState::ITextureObject>> m_aliveObjects;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
