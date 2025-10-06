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
        Depth,
        Stencil,
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
                Bool IsEmpty();
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

                FramebufferObject();

                void AttachTexture(FramebufferAttachmentType type, SharedPtr<ITextureObject> texture, int level = 0);
                void AttachRenderbuffer(FramebufferAttachmentType type,
                                        std::shared_ptr<RenderbufferObjectStub> renderbuffer);
                void Detach(FramebufferAttachmentType type);
                const FramebufferAttachment& GetAttachment(FramebufferAttachmentType type) const;
                Bool CheckCompleteness() const;
                void SetDrawBuffers(const std::vector<FramebufferAttachmentType>& buffers);
                const Vector<FramebufferAttachmentType>& GetDrawBuffers() const;

            private:
                Array<FramebufferAttachment,
                      static_cast<SizeT>(FramebufferAttachmentType::FramebufferAttachmentTypeCount)>
                    m_attachments;
                Bool m_drawBuffersDirty = true;
                Vector<FramebufferAttachmentType> m_drawBuffers;
            };

        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
