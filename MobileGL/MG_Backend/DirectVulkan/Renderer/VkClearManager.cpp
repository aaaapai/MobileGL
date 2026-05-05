// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/VkClearManager.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VkClearManager.h"

#include "MG_Util/Converters/MGToStr/FramebufferEnumConverter.h"
#include "MG_Util/Converters/MGToStr/TextureEnumConverter.h"

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
                    .mask = GL_COLOR_BUFFER_BIT,
                    .color = clearPayload.color
                }, drawFbo.GetAttachment(drawbuf).GetTexture());
                MGLOG_D("%s: %s (texture %d) - color = (%.2f, %.2f, %.2f, %.2f)", __func__,
                    MG_Util::ConvertFramebufferAttachmentTypeToString(drawbuf).c_str(),
                    drawFbo.GetAttachment(drawbuf).GetTexture()->GetExternalIndex(),
                    clearPayload.color[0], clearPayload.color[1], clearPayload.color[2], clearPayload.color[3]);
            }
        }

        if (mask & GL_DEPTH_BUFFER_BIT &&
            !drawFbo.GetAttachment(FramebufferAttachmentType::Depth).IsRenderbuffer()) {
            QueueClear({
                .mask = GL_DEPTH_BUFFER_BIT,
                .depth = clearPayload.depth,
            }, drawFbo.GetAttachment(FramebufferAttachmentType::Depth).GetTexture());
            MGLOG_D("%s: Depth (texture %d) - depth = (%.2f)", __func__,
                drawFbo.GetAttachment(FramebufferAttachmentType::Depth).GetTexture()->GetExternalIndex(), clearPayload.depth);
        }

        if (mask & GL_STENCIL_BUFFER_BIT &&
            !drawFbo.GetAttachment(FramebufferAttachmentType::Stencil).IsRenderbuffer()) {
            QueueClear({
                .mask = GL_STENCIL_BUFFER_BIT,
                .stencil = clearPayload.stencil,
            }, drawFbo.GetAttachment(FramebufferAttachmentType::Stencil).GetTexture());
            MGLOG_D("%s: Stencil (texture %d) - stencil = (%u)", __func__,
                drawFbo.GetAttachment(FramebufferAttachmentType::Stencil).GetTexture()->GetExternalIndex(), clearPayload.stencil);
        }
    }

    void VkClearManager::QueueClear(const ClearAttachmentPayload& clearPayload,
                                    const SharedPtr<MG_State::GLState::ITextureObject>& texture) {
        if (clearPayload.mask == 0) {
            return;
        }
        WeakPtr<MG_State::GLState::ITextureObject> weakTexturePtr = texture;
        if (weakTexturePtr.expired())
            return;
        auto* pTexture = weakTexturePtr.lock().get();
        m_aliveObjects[pTexture] = weakTexturePtr;
        auto& pending = m_pendingClears[pTexture];
        pending.mask |= clearPayload.mask;
        if (clearPayload.mask & GL_COLOR_BUFFER_BIT) {
            pending.color = clearPayload.color;
        }
        if (clearPayload.mask & GL_DEPTH_BUFFER_BIT) {
            pending.depth = clearPayload.depth;
        }
        if (clearPayload.mask & GL_STENCIL_BUFFER_BIT) {
            pending.stencil = clearPayload.stencil;
        }
    }

    Bool VkClearManager::HasPendingClear(MG_State::GLState::ITextureObject* texture) {
        return m_pendingClears.find(texture) != m_pendingClears.end();
    }

    Bool VkClearManager::GetPendingClear(MG_State::GLState::ITextureObject* texture, ClearAttachmentPayload& outPayload) {
        if (m_aliveObjects.find(texture) == m_aliveObjects.end() ||
            m_pendingClears.find(texture) == m_pendingClears.end()) {
            MGLOG_D("%s: Failed getting pending clear for texture %d", __func__, texture->GetExternalIndex());
            return false;
        }

        outPayload = m_pendingClears[texture];
        MGLOG_D("%s: Got pending clear for texture %d (%s), mask=0x%x clear value: color = (%.2f, %.2f, %.2f, %.2f), depth = (%.2f), stencil = (%u)", __func__,
            texture->GetExternalIndex(),
            MG_Util::ConvertTextureInternalFormatToString(texture->GetFormat()).c_str(),
            static_cast<Uint32>(outPayload.mask),
            outPayload.color[0], outPayload.color[1], outPayload.color[2], outPayload.color[3],
            outPayload.depth,
            outPayload.stencil);
        return true;
    }

    void VkClearManager::PopPendingClear(MG_State::GLState::ITextureObject* texture) {
        MGLOG_D("%s: Pop pending clear for texture %d", __func__, texture->GetExternalIndex());
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
