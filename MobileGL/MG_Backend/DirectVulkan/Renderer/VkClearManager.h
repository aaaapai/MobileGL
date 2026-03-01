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
        union {
            FloatVec4 color;
            Float depth{};
            Uint32 stencil;
        };
        FramebufferAttachmentType attachmentType = FramebufferAttachmentType::Color0;
    };

    class VkClearManager {
    public:
        Bool Initialize();
        void Shutdown();

        void QueueClear(GLbitfield mask, const ClearFramebufferPayload& clearPayload, const MG_State::GLState::FramebufferObject& drawFbo);
        void QueueClear(
            const ClearAttachmentPayload& clearPayload,
            const SharedPtr<MG_State::GLState::ITextureObject>& texture);
        Bool HasPendingClear(MG_State::GLState::ITextureObject* texture);
        Bool GetPendingClear(MG_State::GLState::ITextureObject* texture, ClearAttachmentPayload& outPayload);
        void PopPendingClear(MG_State::GLState::ITextureObject* texture);
        SizeT CollectGarbage();
    private:
        Uint8 m_gcCounter = 0;
        UnorderedMap<MG_State::GLState::ITextureObject*, ClearAttachmentPayload> m_pendingClears;
        UnorderedMap<MG_State::GLState::ITextureObject*, WeakPtr<MG_State::GLState::ITextureObject>> m_aliveObjects;
    };
} // namespace MobileGL::MG_Backend::DirectVulkan
