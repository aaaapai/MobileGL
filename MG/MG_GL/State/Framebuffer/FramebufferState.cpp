//
// Created by BZLZHH on 2025/5/3.
//

// TODO: Add more gl error check for framebuffer state manager.
// TODO: Check if the state machine really meets up to OpenGL 3 standard.

#include "FramebufferState.h"

// Re-Include Includes.h, cuz of the absence of GLState.h
#undef MOBILEGL_GLSTATE_H
#include "../../../Includes.h"

GLenum FramebufferState::Create(GLuint* framebuffer) {
    if (!framebuffer) return GL_INVALID_VALUE;
    GLuint id = freeIds_.empty() ? ++lastId_ : *freeIds_.begin();
    if (!freeIds_.empty()) freeIds_.erase(freeIds_.begin());
    *framebuffer = id;
    framebuffers_[id].generated = true;
    return GL_NO_ERROR;
}

GLenum FramebufferState::CreateN(GLsizei n, GLuint* framebuffers) {
    if (n < 0) return GL_INVALID_VALUE;
    for (GLsizei i = 0; i < n; ++i) {
        if (GLenum err = Create(&framebuffers[i])) return err;
    }
    return GL_NO_ERROR;
}

GLenum FramebufferState::Delete(GLuint framebuffer) {
    if (framebuffer == 0) return GL_INVALID_VALUE;
    
    if (framebuffers_.erase(framebuffer)) {
        freeIds_.insert(framebuffer);
        for (auto& [target, id] : currentBindings_) {
            if (id == framebuffer) id = 0;
        }
    }
    return GL_NO_ERROR;
}

GLenum FramebufferState::Bind(GLenum target, GLuint framebuffer) {
    if (MG_Constants::Framebuffer::VALID_TARGETS.find(target) == MG_Constants::Framebuffer::VALID_TARGETS.end()) 
        return GL_INVALID_ENUM;
    if (framebuffer != 0 && !ValidateHandle(framebuffer))
        return GL_INVALID_OPERATION;
    if (target == GL_FRAMEBUFFER) {
        currentBindings_[GL_READ_FRAMEBUFFER] = framebuffer;
        currentBindings_[GL_DRAW_FRAMEBUFFER] = framebuffer;
    } else {
        currentBindings_[target] = framebuffer;
    }
    return GL_NO_ERROR;
}

GLenum FramebufferState::glAttachTexture2D(GLenum target, GLenum attachment,
                                           GLenum textarget, GLuint texture, GLint level) {
    if (MG_Constants::Framebuffer::VALID_ATTACHMENTS.find(attachment) == MG_Constants::Framebuffer::VALID_ATTACHMENTS.end() ||
        MG_Constants::Texture::VALID_TARGETS.find(textarget) == MG_Constants::Texture::VALID_TARGETS.end())
        return GL_INVALID_ENUM;
    if (target == GL_FRAMEBUFFER) target = GL_DRAW_FRAMEBUFFER;
    FramebufferObject* fbo = GetCurrentFBO(target);
    if (!fbo) return GL_INVALID_OPERATION;
    if (texture != 0 && !MG_State_T::textureState->IsTexture(texture))
        return GL_INVALID_VALUE;
    FramebufferAttachment att;
    att.type = GL_TEXTURE_2D;
    att.handle = texture;
    att.mipLevel = level;
    fbo->attachments[attachment] = att;
    
    if (texture != 0) {
        GLint params = 0;
        MG_State_T::textureState->QueryLevelPropertyIntVector(
            textarget, level, GL_TEXTURE_WIDTH, &params);
        fbo->width = params;
        MG_State_T::textureState->QueryLevelPropertyIntVector(
            textarget, level, GL_TEXTURE_HEIGHT, &params);
        fbo->height = params;
    }
    UpdateCompleteness(*fbo);
    return GL_NO_ERROR;
}

GLenum FramebufferState::glValidateCompleteness(GLenum target) {
    if (target == GL_FRAMEBUFFER) target = GL_DRAW_FRAMEBUFFER;
    FramebufferObject* fbo = GetCurrentFBO(target);
    if (!fbo) return GL_INVALID_OPERATION;
    return fbo->status;
}

void FramebufferState::UpdateCompleteness(FramebufferObject& fbo) {
    if (fbo.attachments.empty()) {
        fbo.status = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
        return;
    }
    bool hasColor = false;
    bool hasDepth = false;
    bool hasStencil = false;
    GLsizei width = 0, height = 0;
    for (const auto& [attach, att] : fbo.attachments) {
        GLsizei attWidth = 0, attHeight = 0;
        if (att.handle != 0) {
            GLint params = 0;
            MG_State_T::textureState->QueryLevelPropertyIntVector(
                GL_TEXTURE_2D, att.mipLevel, GL_TEXTURE_WIDTH, &params);
            attWidth = params;
            MG_State_T::textureState->QueryLevelPropertyIntVector(
                GL_TEXTURE_2D, att.mipLevel, GL_TEXTURE_HEIGHT, &params);
            attHeight = params;
        }
        if (width == 0) {
            width = attWidth;
            height = attHeight;
        } else if (attWidth != width || attHeight != height) {
            fbo.status = GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS;
            return;
        }
        
        if (attach == GL_DEPTH_ATTACHMENT) hasDepth = true;
        if (attach == GL_STENCIL_ATTACHMENT) hasStencil = true;
        if (attach >= GL_COLOR_ATTACHMENT0 && 
            attach <= GL_COLOR_ATTACHMENT31) hasColor = true;
    }
    
    if (!ValidateAttachmentCombination(fbo)) {
        fbo.status = GL_FRAMEBUFFER_UNSUPPORTED;
        return;
    }
    fbo.status = GL_FRAMEBUFFER_COMPLETE;
}

bool FramebufferState::ValidateAttachmentCombination(const FramebufferObject& fbo) {
    bool depthStencilConflict = false;
    for (const auto& [attach, att] : fbo.attachments) {
        if (attach == GL_DEPTH_STENCIL_ATTACHMENT) {
            GLint internalFormat = 0;
            MG_State_T::textureState->QueryLevelPropertyIntVector(
                GL_TEXTURE_2D, att.mipLevel, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
            if (internalFormat != GL_DEPTH24_STENCIL8 && 
                internalFormat != GL_DEPTH32F_STENCIL8) {
                return false;
            }
        }
    }
    return true;
}

FramebufferObject* FramebufferState::GetCurrentFBO(GLenum target) {
    auto it = currentBindings_.find(target);
    if (it == currentBindings_.end()) return nullptr;
    
    GLuint id = it->second;
    if (id == 0) return nullptr;
    
    auto fboIt = framebuffers_.find(id);
    return (fboIt != framebuffers_.end()) ? &fboIt->second : nullptr;
}

bool FramebufferState::ValidateHandle(GLuint framebuffer) {
    return framebuffers_.count(framebuffer) && 
           framebuffers_[framebuffer].generated;
}