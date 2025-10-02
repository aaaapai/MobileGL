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
            FramebufferAttachment::FramebufferAttachment(std::nullptr_t)
                : m_texture(nullptr), m_renderbuffer(nullptr) {}

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
                    return m_texture->IsComplete();
                } else if (IsRenderbuffer()) {
                    // TODO
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

            // FramebufferObject
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
                m_attachments[static_cast<SizeT>(type)] = FramebufferAttachment(nullptr);
                m_drawBuffersDirty = true;
            }

            const FramebufferAttachment& FramebufferObject::GetAttachment(FramebufferAttachmentType type) const {
                return m_attachments[static_cast<SizeT>(type)];
            }

            Bool FramebufferObject::CheckCompleteness() const {
                if (m_attachments.empty()) {
                    return false;
                }

                int width = -1, height = -1;
                for (const auto& attachment : m_attachments) {
                    auto attachmentSize = attachment.GetSize();
                    int w = attachmentSize.x();
                    int h = attachmentSize.y();
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
