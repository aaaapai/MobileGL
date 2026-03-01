// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkClearManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VkClearManager.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    Bool VkClearManager::Initialize() {
        return true;
    }

    void VkClearManager::Shutdown() {

    }

    void VkClearManager::QueueClear(GLbitfield mask, const ClearFramebufferPayload& clearPayload,
                                    const MG_State::GLState::FramebufferObject& drawFbo) {
        if (mask & GL_COLOR_BUFFER_BIT) {
            auto& drawbufs = drawFbo.GetDrawBuffers();
            // This should automatically work on default & offscreen FBO
            for (auto drawbuf: drawbufs) {
                if (drawbuf == FramebufferAttachmentType::None ||
                    drawFbo.GetAttachment(drawbuf).IsRenderbuffer())
                    continue;

                QueueClear({
                    .color = clearPayload.color,
                    .attachmentType = drawbuf
                }, drawFbo.GetAttachment(drawbuf).GetTexture());
            }
        }

        if (mask & GL_DEPTH_BUFFER_BIT &&
            !drawFbo.GetAttachment(FramebufferAttachmentType::Depth).IsRenderbuffer()) {
            QueueClear({
                .depth = clearPayload.depth,
                .attachmentType = FramebufferAttachmentType::Depth,
            }, drawFbo.GetAttachment(FramebufferAttachmentType::Depth).GetTexture());
        }

        if (mask & GL_STENCIL_BUFFER_BIT &&
            !drawFbo.GetAttachment(FramebufferAttachmentType::Stencil).IsRenderbuffer()) {
            QueueClear({
                .stencil = clearPayload.stencil,
                .attachmentType = FramebufferAttachmentType::Stencil,
            }, drawFbo.GetAttachment(FramebufferAttachmentType::Depth).GetTexture());
        }
    }

    void VkClearManager::QueueClear(const ClearAttachmentPayload& clearPayload,
                                    const SharedPtr<MG_State::GLState::ITextureObject>& texture) {
        WeakPtr<MG_State::GLState::ITextureObject> weakTexturePtr = texture;
        if (weakTexturePtr.expired())
            return;
        auto* pTexture = weakTexturePtr.lock().get();
        m_aliveObjects[pTexture] = weakTexturePtr;
        m_pendingClears[pTexture] = clearPayload;
    }

    Bool VkClearManager::HasPendingClear(MG_State::GLState::ITextureObject* texture) {
        return m_pendingClears.find(texture) != m_pendingClears.end();
    }

    Bool VkClearManager::GetPendingClear(MG_State::GLState::ITextureObject* texture, ClearAttachmentPayload& outPayload) {
        if (m_aliveObjects.find(texture) == m_aliveObjects.end() ||
            m_pendingClears.find(texture) == m_pendingClears.end()) {
            return false;
        }

        outPayload = m_pendingClears[texture];
        return true;
    }

    void VkClearManager::PopPendingClear(MG_State::GLState::ITextureObject* texture) {
        m_aliveObjects.erase(texture);
        m_pendingClears.erase(texture);
    }

    SizeT VkClearManager::CollectGarbage() {
        m_gcCounter++;
        if (m_gcCounter != 0) {
            return 0;
        }

        SizeT count = 0;
        for (const auto& [raw, weak]: m_aliveObjects) {
            if (weak.expired()) {
                count++;
                m_pendingClears.erase(raw);
                m_aliveObjects.erase(raw);
            }
        }
        return count;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
