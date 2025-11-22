#pragma once
#include "MG_Util/Types.h"
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

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
            class RenderbufferObjectStub {}; // TODO

            class FramebufferAttachment {
            public:
                // TODO: add Renderbuffer when it is implemented
                explicit FramebufferAttachment(SharedPtr<MG_State::GLState::ITextureObject> texture, Int level = 0);
                explicit FramebufferAttachment(SharedPtr<RenderbufferObjectStub> renderbuffer);
                explicit FramebufferAttachment(Bool IsValid = true);

                Bool IsTexture() const;
                Bool IsRenderbuffer() const;
                Bool IsEmpty() const;
                SharedPtr<MG_State::GLState::ITextureObject> GetTexture() const;
                SharedPtr<RenderbufferObjectStub> GetRenderbuffer() const;
                Int GetTextureLevel() const;
                Bool IsComplete() const;
                IntVec3 GetSize() const;
                Bool IsValid() const;

            private:
                SharedPtr<MG_State::GLState::ITextureObject> m_texture = nullptr;
                SharedPtr<RenderbufferObjectStub> m_renderbuffer = nullptr;
                Int m_textureLevel = 0;
                Bool m_isValid = true;
            };

            class FramebufferObject {
            public:
                using TargetEnum = FramebufferTarget;
                static constexpr Uint MAX_DRAW_BUFFERS = 8;

                FramebufferObject(Uint externalIndex);

                void AttachTexture(FramebufferAttachmentType type, SharedPtr<ITextureObject> texture, int level = 0);
                void AttachRenderbuffer(FramebufferAttachmentType type,
                                        std::shared_ptr<RenderbufferObjectStub> renderbuffer);
                void Detach(FramebufferAttachmentType type);
                const FramebufferAttachment& GetAttachment(FramebufferAttachmentType type) const;
                const Array<FramebufferAttachment,
                            static_cast<SizeT>(FramebufferAttachmentType::FramebufferAttachmentTypeCount)>&
                GetAllAttachments() const;
                Bool CheckCompleteness() const;
                // aka. `buffer` as in glDrawBuffers/glReadBuffers
                void SetDrawBuffer(Uint index, FramebufferAttachmentType buffer);
                bool DrawBuffersIsDirty() const { return m_drawBuffersDirty; }
                void ClearDrawBuffersDirtyState() { m_drawBuffersDirty = false; }
                const Array<FramebufferAttachmentType, MAX_DRAW_BUFFERS>& GetDrawBuffers() const;
                FramebufferAttachmentType GetReadBuffer() const { return m_readBuffer; }
                Uint GetExternalIndex() const;

            private:
                const Uint m_externalIndex = 0;
                Array<FramebufferAttachment,
                      static_cast<SizeT>(FramebufferAttachmentType::FramebufferAttachmentTypeCount)>
                    m_attachments;
                Bool m_drawBuffersDirty = false;
                Array<FramebufferAttachmentType, MAX_DRAW_BUFFERS> m_drawBuffers;
                FramebufferAttachmentType m_readBuffer = FramebufferAttachmentType::Color0;
            };

        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
