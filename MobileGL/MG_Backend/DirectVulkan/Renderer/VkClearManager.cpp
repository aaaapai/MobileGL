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
    static Bool IsCubeMapFaceUploadTarget(TextureUploadTarget target) {
        return target >= TextureUploadTarget::CubeMapPositiveX &&
               target <= TextureUploadTarget::CubeMapNegativeZ;
    }

    static Uint32 ResolveAttachmentBaseArrayLayer(TextureUploadTarget target) {
        if (!IsCubeMapFaceUploadTarget(target)) {
            return 0;
        }
        return static_cast<Uint32>(target) - static_cast<Uint32>(TextureUploadTarget::CubeMapPositiveX);
    }

    static const MG_State::GLState::FramebufferAttachmentObject* GetClearableAttachment(
        const MG_State::GLState::FramebufferObject& drawFbo, FramebufferAttachmentType attachmentType) {
        if (attachmentType == FramebufferAttachmentType::None) {
            return nullptr;
        }

        const auto& attachment = drawFbo.GetAttachment(attachmentType);
        if (!attachment.IsTexture() || attachment.IsRenderbuffer()) {
            return nullptr;
        }

        return &attachment;
    }

    PendingClearKey VkClearManager::MakePendingClearKey(MG_State::GLState::ITextureObject* texture, Uint32 mipLevel,
                                                        Uint32 baseArrayLayer, Uint32 layerCount) {
        return PendingClearKey {
            .texture = texture,
            .mipLevel = mipLevel,
            .baseArrayLayer = baseArrayLayer,
            .layerCount = layerCount,
        };
    }

    PendingClearKey VkClearManager::MakePendingClearKey(
        const MG_State::GLState::FramebufferAttachmentObject& attachment) {
        MOBILEGL_ASSERT(attachment.IsTexture() && !attachment.IsRenderbuffer(),
                        "MakePendingClearKey requires a texture framebuffer attachment");
        auto* texture = attachment.GetTexture().get();
        MOBILEGL_ASSERT(texture != nullptr, "MakePendingClearKey: texture attachment resolved to null");
        const Uint32 mipLevel = static_cast<Uint32>(std::max(attachment.GetTextureLevel(), 0));
        const TextureUploadTarget uploadTarget = attachment.GetTextureUploadTarget();
        const Uint32 baseArrayLayer = ResolveAttachmentBaseArrayLayer(uploadTarget);
        const Uint32 layerCount = IsCubeMapFaceUploadTarget(uploadTarget) ? 1u : 1u;
        return MakePendingClearKey(texture, mipLevel, baseArrayLayer, layerCount);
    }

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
                const auto* attachment = GetClearableAttachment(drawFbo, drawbuf);
                if (!attachment) {
                    continue;
                }

                QueueClear({
                    .mask = GL_COLOR_BUFFER_BIT,
                    .color = clearPayload.color
                }, *attachment);

                MGLOG_D("%s: %s (texture %d) - color = (%.2f, %.2f, %.2f, %.2f)", __func__,
                    MG_Util::ConvertFramebufferAttachmentTypeToString(drawbuf).c_str(),
                    attachment->GetTexture()->GetExternalIndex(),
                    clearPayload.color[0], clearPayload.color[1], clearPayload.color[2], clearPayload.color[3]);
            }
        }

        if (mask & GL_DEPTH_BUFFER_BIT) {
            const auto* attachment = GetClearableAttachment(drawFbo, FramebufferAttachmentType::Depth);
            if (attachment) {
                QueueClear({
                    .mask = GL_DEPTH_BUFFER_BIT,
                    .depth = clearPayload.depth,
                }, *attachment);

                MGLOG_D("%s: Depth (texture %d) - depth = (%.2f)", __func__,
                    attachment->GetTexture()->GetExternalIndex(), clearPayload.depth);
            }
        }

        if (mask & GL_STENCIL_BUFFER_BIT) {
            const auto* attachment = GetClearableAttachment(drawFbo, FramebufferAttachmentType::Stencil);
            if (attachment) {
                QueueClear({
                    .mask = GL_STENCIL_BUFFER_BIT,
                    .stencil = clearPayload.stencil,
                }, *attachment);

                MGLOG_D("%s: Stencil (texture %d) - stencil = (%u)", __func__,
                    attachment->GetTexture()->GetExternalIndex(), clearPayload.stencil);
            }
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
        auto& pending = m_pendingClears[MakePendingClearKey(pTexture)];
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

    void VkClearManager::QueueClear(const ClearAttachmentPayload& clearPayload,
                                    const MG_State::GLState::FramebufferAttachmentObject& attachment) {
        if (clearPayload.mask == 0 || !attachment.IsTexture() || attachment.IsRenderbuffer()) {
            return;
        }
        const auto texture = attachment.GetTexture();
        if (!texture) {
            return;
        }
        WeakPtr<MG_State::GLState::ITextureObject> weakTexturePtr = texture;
        if (weakTexturePtr.expired()) {
            return;
        }
        auto* pTexture = weakTexturePtr.lock().get();
        m_aliveObjects[pTexture] = weakTexturePtr;
        auto& pending = m_pendingClears[MakePendingClearKey(attachment)];
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
        if (texture == nullptr) {
            return false;
        }
        return std::any_of(m_pendingClears.begin(), m_pendingClears.end(),
                           [texture](const auto& item) { return item.first.texture == texture; });
    }

    Bool VkClearManager::HasPendingClear(const PendingClearKey& key) {
        return key.texture != nullptr && m_pendingClears.find(key) != m_pendingClears.end();
    }

    Bool VkClearManager::HasPendingClear(const MG_State::GLState::FramebufferAttachmentObject& attachment) {
        if (!attachment.IsTexture() || attachment.IsRenderbuffer() || !attachment.GetTexture()) {
            return false;
        }
        return HasPendingClear(MakePendingClearKey(attachment));
    }

    Bool VkClearManager::GetPendingClear(const PendingClearKey& key, ClearAttachmentPayload& outPayload) {
        if (key.texture == nullptr || m_aliveObjects.find(key.texture) == m_aliveObjects.end()) {
            return false;
        }
        auto it = m_pendingClears.find(key);
        if (it == m_pendingClears.end()) {
            MGLOG_D("%s: Failed getting pending clear for texture %d mip=%u layer=%u count=%u", __func__,
                    key.texture ? key.texture->GetExternalIndex() : 0, key.mipLevel, key.baseArrayLayer, key.layerCount);
            return false;
        }

        outPayload = it->second;
        MGLOG_D("%s: Got pending clear for texture %d (%s), mip=%u layer=%u count=%u mask=0x%x clear value: color = (%.2f, %.2f, %.2f, %.2f), depth = (%.2f), stencil = (%u)", __func__,
            key.texture->GetExternalIndex(),
            MG_Util::ConvertTextureInternalFormatToString(key.texture->GetFormat()).c_str(),
            key.mipLevel, key.baseArrayLayer, key.layerCount,
            static_cast<Uint32>(outPayload.mask),
            outPayload.color[0], outPayload.color[1], outPayload.color[2], outPayload.color[3],
            outPayload.depth,
            outPayload.stencil);
        return true;
    }

    Bool VkClearManager::GetPendingClear(const MG_State::GLState::FramebufferAttachmentObject& attachment,
                                         ClearAttachmentPayload& outPayload) {
        if (!attachment.IsTexture() || attachment.IsRenderbuffer() || !attachment.GetTexture()) {
            MGLOG_D("%s: Failed getting pending clear for non-texture framebuffer attachment", __func__);
            return false;
        }
        return GetPendingClear(MakePendingClearKey(attachment), outPayload);
    }

    Bool VkClearManager::GetPendingClears(MG_State::GLState::ITextureObject* texture,
                                          Vector<PendingClearEntry>& outEntries) {
        outEntries.clear();
        if (texture == nullptr || m_aliveObjects.find(texture) == m_aliveObjects.end()) {
            return false;
        }
        for (const auto& [key, payload] : m_pendingClears) {
            if (key.texture != texture) {
                continue;
            }
            outEntries.emplace_back(PendingClearEntry{.key = key, .payload = payload});
        }
        return !outEntries.empty();
    }

    void VkClearManager::PopPendingClear(MG_State::GLState::ITextureObject* texture) {
        if (texture == nullptr) {
            return;
        }
        MGLOG_D("%s: Pop all pending clears for texture %d", __func__, texture->GetExternalIndex());
        m_aliveObjects.erase(texture);
        for (auto it = m_pendingClears.begin(); it != m_pendingClears.end();) {
            if (it->first.texture == texture) {
                it = m_pendingClears.erase(it);
            } else {
                ++it;
            }
        }
    }

    void VkClearManager::PopPendingClear(const PendingClearKey& key) {
        if (key.texture == nullptr) {
            return;
        }
        MGLOG_D("%s: Pop pending clear for texture %d mip=%u layer=%u count=%u", __func__,
                key.texture->GetExternalIndex(), key.mipLevel, key.baseArrayLayer, key.layerCount);
        m_pendingClears.erase(key);
    }

    void VkClearManager::PopPendingClear(const MG_State::GLState::FramebufferAttachmentObject& attachment) {
        if (!attachment.IsTexture() || attachment.IsRenderbuffer() || !attachment.GetTexture()) {
            return;
        }
        PopPendingClear(MakePendingClearKey(attachment));
    }

    SizeT VkClearManager::CollectGarbage() {
        m_gcCounter++;
        if (m_gcCounter != 0) {
            return 0;
        }

        SizeT count = 0;
        for (auto it = m_aliveObjects.begin(); it != m_aliveObjects.end();) {
            auto current = it++;
            if (current->second.expired()) {
                for (auto clearIt = m_pendingClears.begin(); clearIt != m_pendingClears.end();) {
                    if (clearIt->first.texture == current->first) {
                        clearIt = m_pendingClears.erase(clearIt);
                    } else {
                        ++clearIt;
                    }
                }
                m_aliveObjects.erase(current);
                ++count;
            }
        }
        return count;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
