// MobileGL - MobileGL/MG_State/GLState/FramebufferState/FramebufferObject.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "FramebufferObject.h"
#include "MG_Util/Types.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            // FramebufferAttachment
            FramebufferAttachment::FramebufferAttachment(SharedPtr<MG_State::GLState::ITextureObject> texture,
                                                         Int level)
                : m_texture(texture), m_textureLevel(level) {}
            FramebufferAttachment::FramebufferAttachment(SharedPtr<RenderbufferObject> renderbuffer)
                : m_renderbuffer(renderbuffer) {}
            FramebufferAttachment::FramebufferAttachment(Bool IsValid) : m_texture(nullptr), m_renderbuffer(nullptr) {
                m_isValid = IsValid;
            }

            Bool FramebufferAttachment::IsTexture() const {
                return m_texture != nullptr;
            }

            Bool FramebufferAttachment::IsRenderbuffer() const {
                return m_renderbuffer != nullptr;
            }

            Bool FramebufferAttachment::IsEmpty() const {
                return m_texture == nullptr && m_renderbuffer == nullptr;
            }

            SharedPtr<MG_State::GLState::ITextureObject> FramebufferAttachment::GetTexture() const {
                return m_texture;
            }

            SharedPtr<RenderbufferObject> FramebufferAttachment::GetRenderbuffer() const {
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
                    Bool complete = m_renderbuffer->IsAllocated();
                    return complete;
                }

                return false;
            }

            IntVec3 FramebufferAttachment::GetSize() const {
                if (IsTexture()) {
                    // TODO: get correct upload target
                    MOBILEGL_ASSERT(nullptr != dynamic_cast<MG_State::GLState::TextureObjectMipmap*>(m_texture.get()),
                                    "Texture object here should always be an object with mipmap");
                    auto textureMipmapObject = static_cast<MG_State::GLState::TextureObjectMipmap*>(m_texture.get());
                    return textureMipmapObject->GetMipmapTexelSize(TextureUploadTarget::Texture2D, m_textureLevel);
                } else if (IsRenderbuffer()) {
                    return IntVec3(m_renderbuffer->GetWidth(), m_renderbuffer->GetHeight(), 1);
                }
                return {0, 0, 0};
            }

            Bool FramebufferAttachment::IsValid() const {
                return m_isValid;
            }

            // FramebufferObject
            FramebufferObject::FramebufferObject(Uint externalIndex) : m_externalIndex(externalIndex) {
                m_attachments.fill(FramebufferAttachment(false));
                m_drawBuffers.fill(FramebufferAttachmentType::None);
                m_drawBuffers[0] = FramebufferAttachmentType::Color0;
            }

            void FramebufferObject::AttachTexture(FramebufferAttachmentType type, SharedPtr<ITextureObject> texture,
                                                  int level) {
                m_attachments[static_cast<SizeT>(type)] = FramebufferAttachment(std::move(texture), level);
                m_drawBuffersDirty = true;
            }

            void FramebufferObject::AttachRenderbuffer(FramebufferAttachmentType type,
                                                       std::shared_ptr<RenderbufferObject> renderbuffer) {
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

            const Array<FramebufferAttachment,
                        static_cast<SizeT>(FramebufferAttachmentType::FramebufferAttachmentTypeCount)>&
            FramebufferObject::GetAllAttachments() const {
                return m_attachments;
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

            void FramebufferObject::SetDrawBuffer(Uint index, FramebufferAttachmentType buffer) {
                if (m_drawBuffers[index] == buffer) return;
                m_drawBuffersDirty = true;
                m_drawBuffers[index] = buffer;
            }

            //            void FramebufferObject::SetDrawBuffers(const Vector<FramebufferAttachmentType>& buffers) {
            //                m_drawBuffers = buffers;
            //                m_drawBuffersDirty = true;
            //            }
            //            void SetDrawBuffer(Uint index, FramebufferAttachmentType buffer) {
            //
            //            }

            const Array<FramebufferAttachmentType, FramebufferObject::MAX_DRAW_BUFFERS>& FramebufferObject::
                GetDrawBuffers() const {
                return m_drawBuffers;
            }

            Uint FramebufferObject::GetExternalIndex() const {
                return m_externalIndex;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
