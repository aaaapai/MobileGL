#include "FramebufferObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            // FramebufferAttachment
            FramebufferAttachment::FramebufferAttachment(SharedPtr<MG_State::GLState::ITextureObject> texture,
                                                         Int level)
                : m_texture(texture), m_textureLevel(level) {}
            FramebufferAttachment::FramebufferAttachment(SharedPtr<RenderbufferObjectStub> renderbuffer)
                : m_renderbuffer(renderbuffer) {}
            FramebufferAttachment::FramebufferAttachment(Bool IsValid) : m_texture(nullptr), m_renderbuffer(nullptr) {
                m_isValid = IsValid;
            }

            Bool FramebufferAttachment::IsTexture() const {
                return m_texture != nullptr;
            }

            Bool FramebufferAttachment::IsRenderbuffer() const {
                return false;
            }

            Bool FramebufferAttachment::IsEmpty() {
                return m_texture == nullptr && m_renderbuffer == nullptr;
            }

            SharedPtr<MG_State::GLState::ITextureObject> FramebufferAttachment::GetTexture() const {
                return m_texture;
            }

            SharedPtr<RenderbufferObjectStub> FramebufferAttachment::GetRenderbuffer() const {
                return m_renderbuffer;
            }

            Int FramebufferAttachment::GetTextureLevel() const {
                return m_textureLevel;
            }

            Bool FramebufferAttachment::IsComplete() const {
                if (IsTexture()) {
                    Bool complete = m_texture->IsComplete();
                    return complete;
                } else if (IsRenderbuffer()) {
                    // TODO
                    return false;
                }

                return false;
            }

            IntVec3 FramebufferAttachment::GetSize() const {
                if (IsTexture()) {
                    return m_texture->GetMipmap(m_textureLevel).size;
                } else if (IsRenderbuffer()) {
                    // TODO
                }
                return {0, 0};
            }

            Bool FramebufferAttachment::IsValid() const {
                return m_isValid;
            }

            // FramebufferObject
            FramebufferObject::FramebufferObject() {
                m_attachments.fill(FramebufferAttachment(false));
            }

            void FramebufferObject::AttachTexture(FramebufferAttachmentType type, SharedPtr<ITextureObject> texture,
                                                  int level) {
                m_attachments[static_cast<SizeT>(type)] = FramebufferAttachment(std::move(texture), level);
                m_drawBuffersDirty = true;
            }

            void FramebufferObject::AttachRenderbuffer(FramebufferAttachmentType type,
                                                       std::shared_ptr<RenderbufferObjectStub> renderbuffer) {
                m_attachments[static_cast<SizeT>(type)] = FramebufferAttachment(renderbuffer);
                m_drawBuffersDirty = true;
            }

            void FramebufferObject::Detach(FramebufferAttachmentType type) {
                m_attachments[static_cast<SizeT>(type)] = FramebufferAttachment(false);
                m_drawBuffersDirty = true;
            }

            const FramebufferAttachment& FramebufferObject::GetAttachment(FramebufferAttachmentType type) const {
                return m_attachments[static_cast<SizeT>(type)];
            }

            Bool FramebufferObject::CheckCompleteness() const {
                if (m_attachments.empty()) {
                    return false;
                }

                Int width = -1, height = -1;
                Int validAttachmentCount = 0;
                for (SizeT i = 0; i < m_attachments.size(); ++i) {
                    if (!m_attachments[i].IsValid()) continue;

                    ++validAttachmentCount;
                    const auto& attachment = m_attachments[i];
                    auto attachmentSize = attachment.GetSize();
                    Int w = attachmentSize.x();
                    Int h = attachmentSize.y();

                    if (width == -1) {
                        width = w;
                        height = h;
                    } else if (width != w || height != h) {
                        return false;
                    }

                    if (!attachment.IsComplete()) {
                        return false;
                    }
                }

                if (validAttachmentCount == 0) return false;
                return true;
            }

            void FramebufferObject::SetDrawBuffers(const std::vector<FramebufferAttachmentType>& buffers) {
                m_drawBuffers = buffers;
                m_drawBuffersDirty = true;
            }

            const Vector<FramebufferAttachmentType>& FramebufferObject::GetDrawBuffers() const {
                return m_drawBuffers;
            }

        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
