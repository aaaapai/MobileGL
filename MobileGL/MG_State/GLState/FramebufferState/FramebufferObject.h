// MobileGL - MobileGL/MG_State/GLState/FramebufferState/FramebufferObject.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>
#include <MG_State/GLState/RenderbufferState/RenderbufferObject.h>

namespace MobileGL {
    enum class FramebufferTarget {
        Draw,
        Read,
        FramebufferTargetCount,
        Unknown = -1
    };

    enum class FramebufferAttachmentType {
        None,

        FrontLeft,
        FrontRight,
        BackLeft,
        BackRight,

        Depth,
        Stencil,

        Color0,
        Color1,
        Color2,
        Color3,
        Color4,
        Color5,
        Color6,
        Color7,
        Color8,
        Color9,
        Color10,
        Color11,
        Color12,
        Color13,
        Color14,
        Color15,
        Color16,
        Color17,
        Color18,
        Color19,
        Color20,
        Color21,
        Color22,
        Color23,
        Color24,
        Color25,
        Color26,
        Color27,
        Color28,
        Color29,
        Color30,
        Color31,

        FramebufferAttachmentTypeCount,
        Unknown = -1
    };

    namespace MG_State {
        namespace GLState {
            class FramebufferAttachmentObject {
            public:
                explicit FramebufferAttachmentObject(SharedPtr<MG_State::GLState::ITextureObject> texture, Int level = 0);
                explicit FramebufferAttachmentObject(SharedPtr<RenderbufferObject> renderbuffer);
                explicit FramebufferAttachmentObject(Bool IsValid = true);

                Bool IsTexture() const;
                Bool IsRenderbuffer() const;
                Bool IsEmpty() const;
                SharedPtr<MG_State::GLState::ITextureObject> GetTexture() const;
                SharedPtr<RenderbufferObject> GetRenderbuffer() const;
                Int GetTextureLevel() const;
                Bool IsComplete() const;
                IntVec3 GetSize() const;
                Bool IsValid() const;

            private:
                SharedPtr<MG_State::GLState::ITextureObject> m_texture = nullptr;
                SharedPtr<RenderbufferObject> m_renderbuffer = nullptr;
                Int m_textureLevel = 0;
                Bool m_isValid = true;
            };

            class FramebufferObject {
            public:
                static constexpr Uint MAX_DRAW_BUFFERS = 8;

                using TargetEnum = FramebufferTarget;
                using FramebufferAttachmentObjectArray =
                        Array<FramebufferAttachmentObject, static_cast<SizeT>(FramebufferAttachmentType::FramebufferAttachmentTypeCount)>;
                using FramebufferAttachmentArray = Array<FramebufferAttachmentType, MAX_DRAW_BUFFERS>;

                FramebufferObject(Uint externalIndex);

                void AttachTexture(FramebufferAttachmentType type, SharedPtr<ITextureObject> texture, int level = 0);
                void AttachRenderbuffer(FramebufferAttachmentType type,
                                        std::shared_ptr<RenderbufferObject> renderbuffer);
                void Detach(FramebufferAttachmentType type);
                const FramebufferAttachmentObject& GetAttachment(FramebufferAttachmentType type) const;
                const FramebufferAttachmentObjectArray& GetAllAttachments() const;
                Bool CheckCompleteness() const;
                // aka. `buffer` as in glDrawBuffers/glReadBuffers
                void SetDrawBuffer(Uint index, FramebufferAttachmentType buffer);
                bool DrawBuffersIsDirty() const { return m_drawBuffersDirty; }
                void ClearDrawBuffersDirtyState() { m_drawBuffersDirty = false; }
                const FramebufferAttachmentArray& GetDrawBuffers() const;
                void SetReadBuffer(FramebufferAttachmentType buf) { m_readBuffer = buf; }
                FramebufferAttachmentType GetReadBuffer() const { return m_readBuffer; }
                Uint GetExternalIndex() const;

            private:
                const Uint m_externalIndex = 0;
                FramebufferAttachmentObjectArray m_attachments;
                FramebufferAttachmentArray m_drawBuffers;
                FramebufferAttachmentType m_readBuffer = FramebufferAttachmentType::Color0;

                Bool m_drawBuffersDirty = false;
            };

        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
