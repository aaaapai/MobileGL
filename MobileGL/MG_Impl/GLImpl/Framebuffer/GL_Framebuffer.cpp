// MobileGL - MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "GL_Framebuffer.h"
#include "Validators.h"
#include "Config.h"
#include <MG_Backend/BackendObjects.h>
#include <MG_Util/Metrics/TextureMetrics.h>
#include <MG_Impl/GLImpl/Texture/Validators.h>
#include <MG_State/GLState/ErrorState/Error.h>
#include <MG_Util/Converters/GLToStr/GLEnumConverter.h>
#include <MG_Util/Converters/GLToMG/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToMG/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToGL/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToStr/TextureEnumConverter.h>
#include <MG_Util/Converters/GLToMG/FramebufferEnumConverter.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace {
        Bool IsActiveBackendDirectVulkan() {
            auto* activeBackend = MG_Backend::pActiveBackendObject.get();
            return activeBackend != nullptr && activeBackend->GetBackendType() == BackendType::DirectVulkan;
        }

        Bool HasDistinctCompleteDepthStencilTextureAttachments(
            const MG_State::GLState::FramebufferObject& framebufferObject) {
            if (framebufferObject.GetExternalIndex() == 0) {
                return false;
            }

            const auto& depthAttachment = framebufferObject.GetAttachment(FramebufferAttachmentType::Depth);
            const auto& stencilAttachment = framebufferObject.GetAttachment(FramebufferAttachmentType::Stencil);
            if (!depthAttachment.IsComplete() || !stencilAttachment.IsComplete() ||
                !depthAttachment.IsTexture() || !stencilAttachment.IsTexture()) {
                return false;
            }

            return depthAttachment.GetTexture().get() != stencilAttachment.GetTexture().get() ||
                   depthAttachment.GetTextureUploadTarget() != stencilAttachment.GetTextureUploadTarget() ||
                   depthAttachment.GetTextureLevel() != stencilAttachment.GetTextureLevel();
        }

        Bool HasCompleteRenderbufferAttachment(const MG_State::GLState::FramebufferObject& framebufferObject) {
            if (framebufferObject.GetExternalIndex() == 0) {
                return false;
            }

            for (const auto& attachment : framebufferObject.GetAllAttachmentObjects()) {
                if (attachment.IsRenderbuffer() && attachment.IsComplete()) {
                    return true;
                }
            }
            return false;
        }

        Bool IsUnsupportedFramebufferForDirectVulkan(
            const MG_State::GLState::FramebufferObject& framebufferObject) {
            return HasDistinctCompleteDepthStencilTextureAttachments(framebufferObject) ||
                   HasCompleteRenderbufferAttachment(framebufferObject);
        }

        void RecordUnsupportedFramebufferTextureAttachmentError(const char* functionName, const char* detail) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", functionName, detail));
        }

        Bool ResolveRepresentableFramebufferTextureUploadTarget(const MG_State::GLState::ITextureObject& textureObject,
                                                                TextureUploadTarget& outUploadTarget) {
            switch (textureObject.GetTarget()) {
            case TextureTarget::Texture1D:
                outUploadTarget = TextureUploadTarget::Texture1D;
                return true;
            case TextureTarget::Texture2D:
                outUploadTarget = TextureUploadTarget::Texture2D;
                return true;
            case TextureTarget::TextureRectangle:
                outUploadTarget = TextureUploadTarget::TextureRectangle;
                return true;
            case TextureTarget::Texture2DMultisample:
                outUploadTarget = TextureUploadTarget::Texture2DMultisample;
                return true;
            default:
                outUploadTarget = TextureUploadTarget::Unknown;
                return false;
            }
        }

        void AttachFramebufferTextureWithUploadTarget(const char* functionName, GLenum target, GLenum attachment,
                                                      GLuint texture, GLint level,
                                                      TextureUploadTarget textureUploadTarget) {
            if (target == GL_FRAMEBUFFER) {
                target = GL_DRAW_FRAMEBUFFER;
            }

            if (attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
                AttachFramebufferTextureWithUploadTarget(functionName, target, GL_DEPTH_ATTACHMENT, texture, level,
                                                        textureUploadTarget);
                AttachFramebufferTextureWithUploadTarget(functionName, target, GL_STENCIL_ATTACHMENT, texture, level,
                                                        textureUploadTarget);
                return;
            }

            const FramebufferAttachmentType attachmentType = MG_Util::ConvertGLEnumToFramebufferAttachmentType(attachment);
            const FramebufferTarget framebufferTarget = MG_Util::ConvertGLEnumToFramebufferTarget(target);
            if (!FramebufferImpl::ValidateFramebufferAttachmentType(attachmentType)) return;
            if (!FramebufferImpl::ValidateFramebufferTarget(framebufferTarget)) return;
            if (!TextureImpl::ValidateTextureName(texture, true)) return;

            auto& bindingSlot = MG_State::pGLContext->GetFramebufferBindingSlot(framebufferTarget);
            auto& framebufferObject = bindingSlot.GetBoundObject();
            if (!framebufferObject) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", functionName,
                                                 "Framebuffer target is bound to no framebuffer object."));
                return;
            }

            if (texture == 0) {
                framebufferObject->Detach(attachmentType);
                return;
            }

            auto& textureObject = MG_State::pGLContext->GetTextureObject(texture);
            if (!textureObject) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", functionName,
                                                 std::format("Texture object {} is not valid.", texture)));
                return;
            }

            const auto expectedTextureTarget = MG_Util::ConvertTextureUploadTargetToTextureTarget(textureUploadTarget);
            if (expectedTextureTarget == TextureTarget::Unknown ||
                textureObject->GetTarget() != expectedTextureTarget) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeUnique<GenericErrorInfo>(
                        "MG_Impl/GLImpl", functionName,
                        std::format("Attachment target {} does not match texture {} target {}.",
                                    MG_Util::ConvertTextureUploadTargetToString(textureUploadTarget), texture,
                                    MG_Util::ConvertTextureTargetToString(textureObject->GetTarget()))));
                return;
            }

            framebufferObject->AttachTexture(attachmentType, textureObject, textureUploadTarget, level);
        }
    } // namespace

    void BlitFramebuffer_Backend(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0,
                                 GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
        MG_Backend::gBackendFunctionsTable.GL.BlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1,
                                                              mask, filter);
    }

    void SampleMaski_State(GLuint maskNumber, GLbitfield mask) {
        if (maskNumber != 0) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "SampleMaski_State",
                                             "Only sample mask word 0 is currently supported."));
            return;
        }

        MG_State::pGLContext->SetSampleMaskValue(static_cast<Uint32>(mask));
    }

    void RenderbufferStorageMultisample_State(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
                                              GLsizei height) {
        RenderbufferTarget rbTarget = MG_Util::ConvertGLEnumToRenderbufferTarget(target);
        if (!FramebufferImpl::ValidateRenderbufferTarget(rbTarget)) return;

        auto& bindingSlot = MG_State::pGLContext->GetRenderbufferBindingSlot(rbTarget);
        auto& renderbufferObject = bindingSlot.GetBoundObject();
        if (!renderbufferObject) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "RenderbufferStorageMultisample_State",
                                             "Renderbuffer target is bound to no renderbuffer object."));
            return;
        }

        TextureInternalFormat format = MG_Util::ConvertGLEnumToTextureInternalFormat(internalformat);
        if (!TextureImpl::ValidateTextureInternalFormat(format)) return;

        if (samples < 0) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "RenderbufferStorageMultisample_State",
                                             "Sample count must be non-negative."));
            return;
        }
        if (width < 0 || height < 0) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "RenderbufferStorageMultisample_State",
                                             "Width and height must be non-negative."));
            return;
        }

        renderbufferObject->AllocateStorage({width, height});
        renderbufferObject->SetInternalFormat(format);
        renderbufferObject->SetSamples(samples);
    }

    void RenderbufferStorage_State(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
        RenderbufferTarget rbTarget = MG_Util::ConvertGLEnumToRenderbufferTarget(target);
        if (!FramebufferImpl::ValidateRenderbufferTarget(rbTarget)) return;
        auto& bindingSlot = MG_State::pGLContext->GetRenderbufferBindingSlot(rbTarget);
        auto& renderbufferObject = bindingSlot.GetBoundObject();
        if (!renderbufferObject) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "RenderbufferStorage_State",
                                             "Renderbuffer target is bound to no renderbuffer object."));
            return;
        }
        TextureInternalFormat format = MG_Util::ConvertGLEnumToTextureInternalFormat(internalformat);
        if (!TextureImpl::ValidateTextureInternalFormat(format)) return;
        if (width < 0 || height < 0) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue, MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "RenderbufferStorage_State",
                                                                      "Width and height must be non-negative."));
            return;
        }
        renderbufferObject->AllocateStorage({width, height});
        renderbufferObject->SetInternalFormat(format);
        renderbufferObject->SetSamples(0);
    }

    GLboolean IsRenderbuffer_State(GLuint renderbuffer) {
        return MG_State::pGLContext->ValidateRenderbufferObject(renderbuffer);
    }

    GLboolean IsFramebuffer_State(GLuint framebuffer) {
        return MG_State::pGLContext->ValidateFramebufferObject(framebuffer);
    }

    void GetFramebufferAttachmentParameteriv_State(GLenum target, GLenum attachment, GLenum pname, GLint* params) {
        if (params == nullptr) return;
        if (target == GL_FRAMEBUFFER) {
            target = GL_DRAW_FRAMEBUFFER;
        }

        FramebufferTarget framebufferTarget = MG_Util::ConvertGLEnumToFramebufferTarget(target);
        if (!FramebufferImpl::ValidateFramebufferTarget(framebufferTarget)) return;

        const Bool depthStencilAlias = attachment == GL_DEPTH_STENCIL_ATTACHMENT;
        FramebufferAttachmentType attachmentType = depthStencilAlias
            ? FramebufferAttachmentType::Depth
            : MG_Util::ConvertGLEnumToFramebufferAttachmentType(attachment);
        if (!FramebufferImpl::ValidateFramebufferAttachmentType(attachmentType)) return;

        auto& bindingSlot = MG_State::pGLContext->GetFramebufferBindingSlot(framebufferTarget);
        auto& framebufferObject = bindingSlot.GetBoundObject();
        if (!framebufferObject) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "GetFramebufferAttachmentParameteriv_State",
                                             "Framebuffer target is bound to no framebuffer object."));
            return;
        }

        const auto* attachmentObject = [&]() -> const MG_State::GLState::FramebufferAttachmentObject* {
            if (!depthStencilAlias) {
                return &framebufferObject->GetAttachment(attachmentType);
            }

            const auto& depthAttachment = framebufferObject->GetAttachment(FramebufferAttachmentType::Depth);
            if (depthAttachment.IsValid() && !depthAttachment.IsEmpty()) return &depthAttachment;

            const auto& stencilAttachment = framebufferObject->GetAttachment(FramebufferAttachmentType::Stencil);
            if (stencilAttachment.IsValid() && !stencilAttachment.IsEmpty()) return &stencilAttachment;

            return nullptr;
        }();

        switch (pname) {
        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
            if (attachmentObject == nullptr || attachmentObject->IsEmpty() || !attachmentObject->IsValid()) {
                *params = GL_NONE;
            } else if (attachmentObject->IsTexture()) {
                *params = GL_TEXTURE;
            } else if (attachmentObject->IsRenderbuffer()) {
                *params = GL_RENDERBUFFER;
            } else {
                *params = GL_NONE;
            }
            break;
        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
            if (attachmentObject == nullptr || attachmentObject->IsEmpty() || !attachmentObject->IsValid()) {
                *params = 0;
            } else if (attachmentObject->IsTexture()) {
                const auto& textureObject = attachmentObject->GetTexture();
                *params = textureObject ? static_cast<GLint>(textureObject->GetExternalIndex()) : 0;
            } else if (attachmentObject->IsRenderbuffer()) {
                const auto& renderbufferObject = attachmentObject->GetRenderbuffer();
                *params = renderbufferObject ? static_cast<GLint>(renderbufferObject->GetExternalIndex()) : 0;
            } else {
                *params = 0;
            }
            break;
        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
            *params = (attachmentObject != nullptr && attachmentObject->IsTexture() && attachmentObject->IsValid())
                ? static_cast<GLint>(attachmentObject->GetTextureLevel())
                : 0;
            break;
        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
            if (attachmentObject != nullptr && attachmentObject->IsTexture() && attachmentObject->IsValid()) {
                const GLenum glUploadTarget =
                    MG_Util::ConvertTextureUploadTargetToGLEnum(attachmentObject->GetTextureUploadTarget());
                switch (glUploadTarget) {
                case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
                case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
                case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
                case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
                case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
                case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
                    *params = static_cast<GLint>(glUploadTarget);
                    break;
                default:
                    *params = 0;
                    break;
                }
            } else {
                *params = 0;
            }
            break;
        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
            *params = 0;
            break;
        default:
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidEnum,
                MakeUnique<GenericErrorInfo>(
                    "MG_Impl/GLImpl", "GetFramebufferAttachmentParameteriv_State",
                    std::format("pname {} is not an accepted value.", MG_Util::ConvertGLEnumToString(pname))));
            return;
        }
    }

    void GenRenderbuffers_State(GLsizei n, GLuint* renderbuffers) {
        if (n < 0) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "GenRenderbuffers_State", "n must be non-negative"));
            return;
        }
        static thread_local Vector<GLuint> renderbufferNames;
        MG_State::pGLContext->GenRenderbufferNames(n, renderbufferNames);
        Memcpy(renderbuffers, renderbufferNames.data(), sizeof(GLuint) * static_cast<SizeT>(n));
    }

    void GenFramebuffers_State(GLsizei n, GLuint* framebuffers) {
        if (n < 0) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "GenFramebuffers_State", "n must be non-negative"));
            return;
        }
        static thread_local Vector<GLuint> framebuffersNames;
        MG_State::pGLContext->GenFramebufferNames(n, framebuffersNames);
        Memcpy(framebuffers, framebuffersNames.data(), sizeof(GLuint) * static_cast<SizeT>(n));
    }

    void FramebufferTextureLayer_State(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
        if (texture == 0) {
            const TextureUploadTarget detachTarget = TextureUploadTarget::Texture2D;
            AttachFramebufferTextureWithUploadTarget(__func__, target, attachment, texture, level, detachTarget);
            return;
        }

        static_cast<void>(layer);
        RecordUnsupportedFramebufferTextureAttachmentError(
            __func__,
            "Layered framebuffer texture attachments are not represented by the current framebuffer attachment model.");
    }

    void FramebufferTexture3D_State(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level,
                                    GLint zoffset) {
        if (texture == 0) {
            TextureUploadTarget textureUploadTarget = MG_Util::ConvertGLEnumToTextureUploadTarget(textarget);
            if (!TextureImpl::ValidateTextureUploadTarget(textureUploadTarget)) return;
            AttachFramebufferTextureWithUploadTarget(__func__, target, attachment, texture, level, textureUploadTarget);
            return;
        }

        static_cast<void>(zoffset);
        RecordUnsupportedFramebufferTextureAttachmentError(
            __func__,
            "3D framebuffer texture slice attachments are not represented by the current framebuffer attachment model.");
    }

    void FramebufferTexture2D_State(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
        if (target == GL_FRAMEBUFFER) {
            target = GL_DRAW_FRAMEBUFFER;
        }

        if (attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
            FramebufferTexture2D_State(target, GL_DEPTH_ATTACHMENT, textarget, texture, level);
            FramebufferTexture2D_State(target, GL_STENCIL_ATTACHMENT, textarget, texture, level);
            return;
        }

        FramebufferAttachmentType attachmentType = MG_Util::ConvertGLEnumToFramebufferAttachmentType(attachment);
        FramebufferTarget framebufferTarget = MG_Util::ConvertGLEnumToFramebufferTarget(target);

        if (!FramebufferImpl::ValidateFramebufferAttachmentType(attachmentType)) return;
        if (!FramebufferImpl::ValidateFramebufferTarget(framebufferTarget)) return;
        if (!TextureImpl::ValidateTextureName(texture, true)) return;
        TextureUploadTarget textureUploadTarget = TextureUploadTarget::Unknown;
        if (texture != 0) {
            textureUploadTarget = MG_Util::ConvertGLEnumToTextureUploadTarget(textarget);
            if (!TextureImpl::ValidateTextureUploadTarget(textureUploadTarget)) return;
        }

        auto& bindingSlot = MG_State::pGLContext->GetFramebufferBindingSlot(framebufferTarget);
        auto& framebufferObject = bindingSlot.GetBoundObject();
        if (!framebufferObject) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "FramebufferTexture2D_State",
                                             "Framebuffer target is bound to no framebuffer object."));
            return;
        }

        if (texture == 0) {
            framebufferObject->Detach(attachmentType);
            return;
        }

        auto& textureObject = MG_State::pGLContext->GetTextureObject(texture);
        if (!textureObject) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "FramebufferTexture2D_State",
                                             std::format("Texture object {} is not valid.", texture)));
            return;
        }

        const auto expectedTextureTarget = MG_Util::ConvertTextureUploadTargetToTextureTarget(textureUploadTarget);
        if (expectedTextureTarget == TextureTarget::Unknown ||
            textureObject->GetTarget() != expectedTextureTarget) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>(
                    "MG_Impl/GLImpl", "FramebufferTexture2D_State",
                    std::format("Attachment target {} does not match texture {} target {}.",
                                MG_Util::ConvertGLEnumToString(textarget),
                                texture,
                                MG_Util::ConvertTextureTargetToString(textureObject->GetTarget()))));
            return;
        }

        framebufferObject->AttachTexture(attachmentType, textureObject, textureUploadTarget, textureObject ? level : 0);
    }

    void FramebufferTexture1D_State(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
        TextureUploadTarget textureUploadTarget = TextureUploadTarget::Unknown;
        if (texture != 0) {
            textureUploadTarget = MG_Util::ConvertGLEnumToTextureUploadTarget(textarget);
            if (!TextureImpl::ValidateTextureUploadTarget(textureUploadTarget)) return;
        } else {
            textureUploadTarget = TextureUploadTarget::Texture1D;
        }
        AttachFramebufferTextureWithUploadTarget(__func__, target, attachment, texture, level, textureUploadTarget);
    }

    void FramebufferTexture_State(GLenum target, GLenum attachment, GLuint texture, GLint level) {
        if (texture == 0) {
            AttachFramebufferTextureWithUploadTarget(__func__, target, attachment, texture, level,
                                                     TextureUploadTarget::Texture2D);
            return;
        }

        auto& textureObject = MG_State::pGLContext->GetTextureObject(texture);
        if (!textureObject) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                             std::format("Texture object {} is not valid.", texture)));
            return;
        }

        TextureUploadTarget textureUploadTarget = TextureUploadTarget::Unknown;
        if (!ResolveRepresentableFramebufferTextureUploadTarget(*textureObject, textureUploadTarget)) {
            RecordUnsupportedFramebufferTextureAttachmentError(
                __func__,
                "Layered or multi-image framebuffer texture targets are not fully represented by the current framebuffer attachment model.");
            return;
        }

        AttachFramebufferTextureWithUploadTarget(__func__, target, attachment, texture, level, textureUploadTarget);
    }

    void FramebufferRenderbuffer_State(GLenum target, GLenum attachment, GLenum renderbuffertarget,
                                       GLuint renderbuffer) {
        if (target == GL_FRAMEBUFFER) {
            target = GL_DRAW_FRAMEBUFFER;
        }

        FramebufferAttachmentType attachmentType = MG_Util::ConvertGLEnumToFramebufferAttachmentType(attachment);
        FramebufferTarget framebufferTarget = MG_Util::ConvertGLEnumToFramebufferTarget(target);
        RenderbufferTarget rbTarget = MG_Util::ConvertGLEnumToRenderbufferTarget(renderbuffertarget);
        if (!FramebufferImpl::ValidateFramebufferAttachmentType(attachmentType)) return;
        if (!FramebufferImpl::ValidateFramebufferTarget(framebufferTarget)) return;
        if (!FramebufferImpl::ValidateRenderbufferTarget(rbTarget)) return;
        if (!FramebufferImpl::ValidateRenderbufferName(renderbuffer, true)) return;
        auto& bindingSlot = MG_State::pGLContext->GetFramebufferBindingSlot(framebufferTarget);
        auto& framebufferObject = bindingSlot.GetBoundObject();
        if (!framebufferObject) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "FramebufferRenderbuffer_State",
                                             "Framebuffer target is bound to no framebuffer object."));
            return;
        }

        if (renderbuffer == 0) {
            framebufferObject->Detach(attachmentType);
            return;
        }

        auto& renderbufferObject = MG_State::pGLContext->GetRenderbufferObject(renderbuffer);
        if (!renderbufferObject) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "FramebufferRenderbuffer_State",
                                             std::format("Renderbuffer object {} is not valid.", renderbuffer)));
            return;
        }

        framebufferObject->AttachRenderbuffer(attachmentType, renderbufferObject);
    }

    void DrawBuffers_State(GLsizei n, const GLenum* bufs) {
        if (n < 0) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__, "`n` is less than 0."));
            return;
        } else if (n > MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__, "`n` is greater than `GL_MAX_DRAW_BUFFERS`."));
            return;
        }

        // Get bound framebuffer
        auto& bindingSlot = MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw);
        auto& fbo = bindingSlot.GetBoundObject();
        if (!fbo) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__, "No framebuffer bound to draw target."));
            return;
        }
        bool isDefaultFBO = (fbo == FramebufferImpl::pDefaultFramebufferInfo->defaultFBO);

        static int existenceMap[(SizeT)FramebufferAttachmentType::FramebufferAttachmentTypeCount] = {-1};
        std::fill(existenceMap, existenceMap + (SizeT)FramebufferAttachmentType::FramebufferAttachmentTypeCount, -1);

        for (GLsizei i = 0; i < n; ++i) {
            auto attType = MG_Util::ConvertGLEnumToFramebufferAttachmentType(bufs[i]);

            // ------------------- Check validity begin ------------------------
            if (attType == FramebufferAttachmentType::Unknown) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                                 std::format("bufs[{}] = {} is not an accepted value.", i,
                                                             MG_Util::ConvertGLEnumToString(bufs[i]))));
                return;
            }

            if (isDefaultFBO && attType != FramebufferAttachmentType::None &&
                (attType < FramebufferAttachmentType::FrontLeft || attType > FramebufferAttachmentType::BackRight)) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeUnique<GenericErrorInfo>(
                        "MG_Impl/GLImpl", __func__,
                        std::format("FBO is default FBO, but bufs[{}] = {} is not `GL_NONE` or one of the default "
                                    "framebuffer color buffer tokens.",
                                    i, MG_Util::ConvertGLEnumToString(bufs[i]))));
                return;
            }

            if (!isDefaultFBO && attType != FramebufferAttachmentType::None &&
                (attType < FramebufferAttachmentType::Color0 || attType > FramebufferAttachmentType::Color31)) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeUnique<GenericErrorInfo>(
                        "MG_Impl/GLImpl", __func__,
                        std::format("FBO is not default FBO, but bufs[{}] = {} is anything other than `GL_NONE` or "
                                    "one of the `GL_COLOR_ATTACHMENTn` tokens.",
                                    i, MG_Util::ConvertGLEnumToString(bufs[i]))));
                return;
            }

            if (attType != FramebufferAttachmentType::None && existenceMap[(SizeT)attType] >= 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                                 std::format("a symbolic constant other than `GL_NONE` appears "
                                                             "more than once in bufs. bufs[{}] == bufs[{}] == {}.",
                                                             i, existenceMap[(SizeT)attType],
                                                             MG_Util::ConvertGLEnumToString(bufs[i]))));
                return;
            }

            existenceMap[(SizeT)attType] = i;

            if ((SizeT)attType >
                (SizeT)FramebufferAttachmentType::Color0 + MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                                 std::format("bufs[{}] == {} indicates a color buffer that does "
                                                             "not exist in the current GL context.",
                                                             i, MG_Util::ConvertGLEnumToString(bufs[i]))));
                return;
            }
            // ------------------------- Check validity end ----------------------------------
            fbo->SetDrawBuffer(i, attType);
        }
        for (GLsizei i = n; i < MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS; ++i) {
            fbo->SetDrawBuffer(i, FramebufferAttachmentType::None);
        }
    }

    void DrawBuffer_State(GLenum buf) {
        if (buf == GL_NONE) {
            DrawBuffers_State(0, nullptr);
        } else {
            static GLenum bufs[] = {buf};
            DrawBuffers_State(1, bufs);
        }
    }

    void ReadBuffer_State(GLenum mode) {
        auto attType = MG_Util::ConvertGLEnumToFramebufferAttachmentType(mode);

        // ------------------- Check validity begin ------------------------
        if (attType == FramebufferAttachmentType::Unknown) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidEnum,
                MakeUnique<GenericErrorInfo>(
                    "MG_Impl/GLImpl", __func__,
                    std::format("`mode` = {} is not an accepted value.", MG_Util::ConvertGLEnumToString(mode))));
            return;
        }

        // Get bound framebuffer
        auto& bindingSlot = MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Read);
        auto& fbo = bindingSlot.GetBoundObject();
        if (!fbo) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__, "No framebuffer bound to read target."));
            return;
        }
        const Bool isDefaultFBO = (fbo == FramebufferImpl::pDefaultFramebufferInfo->defaultFBO);
        if (isDefaultFBO && attType != FramebufferAttachmentType::None &&
            (attType < FramebufferAttachmentType::FrontLeft || attType > FramebufferAttachmentType::BackRight)) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidEnum,
                MakeUnique<GenericErrorInfo>(
                    "MG_Impl/GLImpl", __func__,
                    std::format("Default framebuffer read buffer {} is not valid.", MG_Util::ConvertGLEnumToString(mode))));
            return;
        }
        if (!isDefaultFBO && attType != FramebufferAttachmentType::None &&
            (attType < FramebufferAttachmentType::Color0 || attType > FramebufferAttachmentType::Color31)) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidEnum,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                             std::format("Framebuffer object read buffer {} is not valid.",
                                                         MG_Util::ConvertGLEnumToString(mode))));
            return;
        }
        if (!isDefaultFBO &&
            static_cast<SizeT>(attType) >
                static_cast<SizeT>(FramebufferAttachmentType::Color0) +
                    MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                             std::format("Read buffer {} indicates a color buffer that does not exist "
                                                         "in the current GL context.",
                                                         MG_Util::ConvertGLEnumToString(mode))));
            return;
        }
        fbo->SetReadBuffer(attType);
    }

    void DeleteRenderbuffers_State(GLsizei n, const GLuint* renderbuffers) {
        if (n < 0) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "DeleteRenderbuffers_State", "n must be non-negative."));
            return;
        }

        if (!renderbuffers) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue, MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "DeleteRenderbuffers_State",
                                                                      "Renderbuffer names array cannot be null."));
            return;
        }

        for (SizeT i = 0; i < static_cast<SizeT>(n); ++i) {
            Uint bufferName = renderbuffers[i];
            if (bufferName == 0) continue;
            if (!FramebufferImpl::ValidateRenderbufferName(bufferName)) continue;
            MG_State::pGLContext->MarkRenderbufferObjectForDeletion(bufferName);
        }
    }

    void DeleteFramebuffers_State(GLsizei n, const GLuint* framebuffers) {
        if (n < 0) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidValue,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "DeleteFramebuffers_State", "n must be non-negative."));
            return;
        }

        if (!framebuffers) {
            MG_State::pGLContext->RecordError(ErrorCode::InvalidValue,
                                              MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "DeleteFramebuffers_State",
                                                                           "Framebuffer names array cannot be null."));
            return;
        }

        for (SizeT i = 0; i < static_cast<SizeT>(n); ++i) {
            Uint bufferName = framebuffers[i];
            if (bufferName == 0) continue;
            if (!FramebufferImpl::ValidateFramebufferName(bufferName)) continue;
            MG_State::pGLContext->MarkFramebufferObjectForDeletion(bufferName);
        }
    }

    GLenum CheckFramebufferStatus_State(GLenum target) {
        FramebufferTarget framebufferTarget = MG_Util::ConvertGLEnumToFramebufferTarget(target);
        if (!FramebufferImpl::ValidateFramebufferTarget(framebufferTarget)) return GL_FRAMEBUFFER_UNDEFINED;

        auto& bindingSlot = MG_State::pGLContext->GetFramebufferBindingSlot(framebufferTarget);
        auto& framebufferObject = bindingSlot.GetBoundObject();
        if (!framebufferObject) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "CheckFramebufferStatus_State",
                                             "Framebuffer target is bound to no framebuffer object."));
            return GL_FRAMEBUFFER_UNDEFINED;
        }

        // TODO: distinguish GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT and GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
        // TODO: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER, GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
        //       additional GL_FRAMEBUFFER_UNSUPPORTED cases, GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
        //       GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
        if (!framebufferObject->CheckCompleteness()) {
            return GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
        }
        if (IsActiveBackendDirectVulkan() &&
            IsUnsupportedFramebufferForDirectVulkan(*framebufferObject)) {
            return GL_FRAMEBUFFER_UNSUPPORTED;
        }
        return GL_FRAMEBUFFER_COMPLETE;
    }

    void BindRenderbuffer_State(GLenum target, GLuint renderbuffer) {
        if (!FramebufferImpl::ValidateRenderbufferName(renderbuffer, true)) return;
        RenderbufferTarget renderbufferTarget = MG_Util::ConvertGLEnumToRenderbufferTarget(target);
        if (!FramebufferImpl::ValidateRenderbufferTarget(renderbufferTarget)) return;

        auto& bindingSlot = MG_State::pGLContext->GetRenderbufferBindingSlot(renderbufferTarget);
        if (renderbuffer == 0) {
            bindingSlot.Bind(nullptr);
            return;
        }

        Bool doesRenderbufferCreated = MG_State::pGLContext->ValidateRenderbufferObject(renderbuffer);
        if (!doesRenderbufferCreated) {
            MG_State::pGLContext->CreateRenderbufferObject(renderbuffer);
        }
        auto& renderbufferObject = MG_State::pGLContext->GetRenderbufferObject(renderbuffer);

        bindingSlot.Bind(renderbufferObject);
    }

    void BindFramebuffer_State(GLenum target, GLuint framebuffer) {
        if (target == GL_FRAMEBUFFER) {
            BindFramebuffer_State(GL_DRAW_FRAMEBUFFER, framebuffer);
            BindFramebuffer_State(GL_READ_FRAMEBUFFER, framebuffer);
            return;
        }

        if (!FramebufferImpl::ValidateFramebufferName(framebuffer)) return;
        FramebufferTarget framebufferTarget = MG_Util::ConvertGLEnumToFramebufferTarget(target);
        if (!FramebufferImpl::ValidateFramebufferTarget(framebufferTarget)) return;

        Bool doesFramebufferCreated = MG_State::pGLContext->ValidateFramebufferObject(framebuffer);
        if (!doesFramebufferCreated) {
            MG_State::pGLContext->CreateFramebufferObject(framebuffer);
        }
        auto& framebufferObject = MG_State::pGLContext->GetFramebufferObject(framebuffer);

        auto& bindingSlot = MG_State::pGLContext->GetFramebufferBindingSlot(framebufferTarget);
        bindingSlot.Bind(framebufferObject);
    }

    void GetRenderbufferParameteriv_State(GLenum target, GLenum pname, GLint* params) {
        if (!params) return;

        RenderbufferTarget renderbufferTarget = MG_Util::ConvertGLEnumToRenderbufferTarget(target);
        if (!FramebufferImpl::ValidateRenderbufferTarget(renderbufferTarget)) return;
        auto& bindingSlot = MG_State::pGLContext->GetRenderbufferBindingSlot(renderbufferTarget);
        auto& renderbufferObject = bindingSlot.GetBoundObject();
        if (!renderbufferObject) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "GetRenderbufferParameteriv_State",
                                             "Renderbuffer target is bound to no renderbuffer object."));
            return;
        }

        switch (pname) {
        case GL_RENDERBUFFER_WIDTH:
            *params = static_cast<GLint>(renderbufferObject->GetWidth());
            break;
        case GL_RENDERBUFFER_HEIGHT:
            *params = static_cast<GLint>(renderbufferObject->GetHeight());
            break;
        case GL_RENDERBUFFER_INTERNAL_FORMAT:
            *params = MG_Util::ConvertTextureInternalFormatToGLEnum(renderbufferObject->GetInternalFormat());
            break;
        case GL_RENDERBUFFER_RED_SIZE:
            *params = static_cast<GLint>(renderbufferObject->GetRedSize());
            break;
        case GL_RENDERBUFFER_GREEN_SIZE:
            *params = static_cast<GLint>(renderbufferObject->GetGreenSize());
            break;
        case GL_RENDERBUFFER_BLUE_SIZE:
            *params = static_cast<GLint>(renderbufferObject->GetBlueSize());
            break;
        case GL_RENDERBUFFER_ALPHA_SIZE:
            *params = static_cast<GLint>(renderbufferObject->GetAlphaSize());
            break;
        case GL_RENDERBUFFER_DEPTH_SIZE:
            *params = static_cast<GLint>(renderbufferObject->GetDepthSize());
            break;
        case GL_RENDERBUFFER_STENCIL_SIZE:
            *params = static_cast<GLint>(renderbufferObject->GetStencilSize());
            break;
        case GL_RENDERBUFFER_SAMPLES:
            *params = static_cast<GLint>(renderbufferObject->GetSamples());
            break;
        default:
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidEnum,
                MakeUnique<GenericErrorInfo>(
                    "MG_Impl/GLImpl", "GetRenderbufferParameteriv_State",
                    std::format("pname {} is not an accepted value.", MG_Util::ConvertGLEnumToString(pname))));
            return;
        }
    }

    void ClearBufferfi_Backend(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
        MG_Backend::gBackendFunctionsTable.GL.ClearBufferfi(buffer, drawbuffer, depth, stencil);
    }

    void ClearBufferfv_Backend(GLenum buffer, GLint drawbuffer, const GLfloat* value) {
        MG_Backend::gBackendFunctionsTable.GL.ClearBufferfv(buffer, drawbuffer, value);
    }

    void ClearBufferuiv_Backend(GLenum buffer, GLint drawbuffer, const GLuint* value) {
        MG_Backend::gBackendFunctionsTable.GL.ClearBufferuiv(buffer, drawbuffer, value);
    }

    void ClearBufferiv_Backend(GLenum buffer, GLint drawbuffer, const GLint* value) {
        MG_Backend::gBackendFunctionsTable.GL.ClearBufferiv(buffer, drawbuffer, value);
    }

    Bool ReadPixels_State(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels) {
        TextureInputFormat textureInputFormat = MG_Util::ConvertGLEnumToTextureInputFormat(format);
        TexturePixelDataType texturePixelDataType = MG_Util::ConvertGLEnumToTexturePixelDataType(type);

        // Check width/height
        if (width < 0 || height < 0) {
            MG_State::pGLContext->RecordError(ErrorCode::InvalidValue,
                                              MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "ReadPixels_State",
                                                                           "Width and height must be non-negative"));
            return false;
        }

        // Validate format
        if (!TextureImpl::ValidateTextureInputFormat(textureInputFormat)) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidEnum,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "ReadPixels_State", "Invalid format"));
            return false;
        }

        // Validate type
        if (!TextureImpl::ValidateTexturePixelDataType(texturePixelDataType)) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidEnum,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "ReadPixels_State", "Invalid pixel data type"));
            return false;
        }

        // Get bound framebuffer
        auto& bindingSlot = MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Read);
        auto& framebufferObject = bindingSlot.GetBoundObject();

        if (!framebufferObject) {
            MG_State::pGLContext->RecordError(ErrorCode::InvalidOperation,
                                              MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "ReadPixels_State",
                                                                           "No framebuffer bound to read target"));
            return false;
        }

        // Check framebuffer completeness
        if (!framebufferObject->CheckCompleteness()) {
            MG_State::pGLContext->RecordError(
                ErrorCode::InvalidFramebufferOperation,
                MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "ReadPixels_State", "Framebuffer is incomplete"));
            return false;
        }

        // Check for required buffers
        if (textureInputFormat == TextureInputFormat::StencilIndex) {
            if (!framebufferObject->GetAttachment(FramebufferAttachmentType::Stencil).IsValid()) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "ReadPixels_State",
                                                 "No stencil buffer for stencil index format"));
                return false;
            }
        } else if (textureInputFormat == TextureInputFormat::DepthComponent) {
            if (!framebufferObject->GetAttachment(FramebufferAttachmentType::Depth).IsValid()) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "ReadPixels_State",
                                                 "No depth buffer for depth component format"));
                return false;
            }
        } else if (textureInputFormat == TextureInputFormat::DepthStencil) {
            if (!framebufferObject->GetAttachment(FramebufferAttachmentType::Depth).IsValid() ||
                !framebufferObject->GetAttachment(FramebufferAttachmentType::Stencil).IsValid()) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "ReadPixels_State",
                                                 "No depth/stencil buffer for depth-stencil format"));
                return false;
            }

            // Validate type for depth/stencil
            if (texturePixelDataType != TexturePixelDataType::UnsignedInt248 &&
                texturePixelDataType != TexturePixelDataType::Float32UnsignedInt248Rev) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum, MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "ReadPixels_State",
                                                                         "Invalid type for depth-stencil format"));
                return false;
            }
        }

        // Check PBO state
        const auto& pixelPackBufferObject =
            MG_State::pGLContext->GetBufferBindingSlot(BufferTarget::PixelPack).GetBoundObject();

        if (pixelPackBufferObject) {
            // Check if PBO is mapped
            if (pixelPackBufferObject->IsMapped()) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation, MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "ReadPixels_State",
                                                                              "Pixel pack buffer is currently mapped"));
                return false;
            }

            // Check alignment
            const SizeT typeSize = MG_Util::GetTexturePixelDataTypeSize(texturePixelDataType);
            if (reinterpret_cast<uintptr_t>(pixels) % typeSize != 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "ReadPixels_State",
                                                 "Pixel data not aligned for pixel pack buffer"));
                return false;
            }
        }

        // Check multisampling
        if (framebufferObject->GetAttachment(FramebufferAttachmentType::Color0).IsRenderbuffer()) {
            auto& rbo = framebufferObject->GetAttachment(FramebufferAttachmentType::Color0).GetRenderbuffer();
            if (rbo && rbo->GetSamples() > 1) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeUnique<GenericErrorInfo>("MG_Impl/GLImpl", "ReadPixels_State",
                                                 "ReadPixels not supported for multisampled framebuffers"));
                return false;
            }
        }

        return true;
    }

    void ReadPixels_Backend(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels) {
        MG_Backend::gBackendFunctionsTable.GL.ReadPixels(x, y, width, height, format, type, pixels);
    }

    /* @INSERTION_POINT:FUNCTION_IMPLEMENTATION@ */
    void ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels) {
        if (!ReadPixels_State(x, y, width, height, format, type, pixels)) return;
        ReadPixels_Backend(x, y, width, height, format, type, pixels);
    }

    void ClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
        ClearBufferfi_Backend(buffer, drawbuffer, depth, stencil);
    }

    void ClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value) {
        ClearBufferfv_Backend(buffer, drawbuffer, value);
    }

    void ClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value) {
        ClearBufferuiv_Backend(buffer, drawbuffer, value);
    }

    void ClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value) {
        ClearBufferiv_Backend(buffer, drawbuffer, value);
    }

    void SampleMaski(GLuint maskNumber, GLbitfield mask) {
        SampleMaski_State(maskNumber, mask);
    }

    void RenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
                                        GLsizei height) {
        RenderbufferStorageMultisample_State(target, samples, internalformat, width, height);
    }

    void RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
        RenderbufferStorage_State(target, internalformat, width, height);
    }

    GLboolean IsRenderbuffer(GLuint renderbuffer) {
        return IsRenderbuffer_State(renderbuffer);
    }

    GLboolean IsFramebuffer(GLuint framebuffer) {
        return IsFramebuffer_State(framebuffer);
    }

    void GetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params) {
        GetFramebufferAttachmentParameteriv_State(target, attachment, pname, params);
    }

    void GenRenderbuffers(GLsizei n, GLuint* renderbuffers) {
        GenRenderbuffers_State(n, renderbuffers);
    }

    void GenFramebuffers(GLsizei n, GLuint* framebuffers) {
        GenFramebuffers_State(n, framebuffers);
    }

    void FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
        FramebufferTextureLayer_State(target, attachment, texture, level, layer);
    }

    void FramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level,
                              GLint zoffset) {
        FramebufferTexture3D_State(target, attachment, textarget, texture, level, zoffset);
    }

    void FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
        FramebufferTexture2D_State(target, attachment, textarget, texture, level);
    }

    void FramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
        FramebufferTexture1D_State(target, attachment, textarget, texture, level);
    }

    void FramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level) {
        FramebufferTexture_State(target, attachment, texture, level);
    }

    void FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
        FramebufferRenderbuffer_State(target, attachment, renderbuffertarget, renderbuffer);
    }

    void DrawBuffer(GLenum buf) {
        DrawBuffer_State(buf);
    }

    void DrawBuffers(GLsizei n, const GLenum* bufs) {
        DrawBuffers_State(n, bufs);
    }

    void ReadBuffer(GLenum src) {
        ReadBuffer_State(src);
    }

    void DeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers) {
        DeleteRenderbuffers_State(n, renderbuffers);
    }

    void DeleteFramebuffers(GLsizei n, const GLuint* framebuffers) {
        DeleteFramebuffers_State(n, framebuffers);
    }

    GLenum CheckFramebufferStatus(GLenum target) {
        return CheckFramebufferStatus_State(target);
    }

    void BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1,
                         GLint dstY1, GLbitfield mask, GLenum filter) {
        BlitFramebuffer_Backend(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    }

    void BindRenderbuffer(GLenum target, GLuint renderbuffer) {
        BindRenderbuffer_State(target, renderbuffer);
    }

    void BindFramebuffer(GLenum target, GLuint framebuffer) {
        BindFramebuffer_State(target, framebuffer);
    }

    void GetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params) {
        GetRenderbufferParameteriv_State(target, pname, params);
    }

    namespace FramebufferImpl {
        UniquePtr<DefaultFramebufferInfo> pDefaultFramebufferInfo;
    } // namespace FramebufferImpl
} // namespace MobileGL::MG_Impl::GLImpl
