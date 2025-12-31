// MobileGL - MobileGL/MG_Impl/Init.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "Init.h"
#include "GLImpl/Texture/ProxyTexture.h"
#include "GLImpl/Framebuffer/GL_Framebuffer.h"

#include <MG_State/GLState/TextureState/TextureObject1D.h>
#include <MG_State/GLState/TextureState/TextureObject2D.h>
#include <MG_State/GLState/TextureState/TextureObject3D.h>

namespace MobileGL {
    namespace MG_Impl {
        void Init() {
            MGLOG_D("Initializing MobileGL Implementation...");
            GLImpl::TextureImpl::pProxyTextureManager = new GLImpl::TextureImpl::ProxyTextureManager();

            // TODO: get real info in EGL
            auto fbo0 = MG_State::pGLContext->CreateFramebufferObject(0);
            auto colorTex = MakeShared<MG_State::GLState::TextureObject2D>(0);
            colorTex->SetInternalFormat(TextureInternalFormat::RGBA8);
            colorTex->AllocateStorage(TextureUploadTarget::Texture2D, 0, {{512, 512, 1}, 0});
            // colorTex->SetMipmapLevel({{512, 512, 1}, 0, false, 0, {nullptr, 0}});
            auto depthTex = MakeShared<MG_State::GLState::TextureObject2D>(0);
            depthTex->SetInternalFormat(TextureInternalFormat::Depth32FStencil8);
            depthTex->AllocateStorage(TextureUploadTarget::Texture2D, 0, {{512, 512, 1}, 0});
            // depthTex->SetMipmapLevel({{512, 512, 1}, 0, false, 0, {nullptr, 0}});
            auto stencilTex = MakeShared<MG_State::GLState::TextureObject2D>(0);
            stencilTex->SetInternalFormat(TextureInternalFormat::Depth32FStencil8);
            stencilTex->AllocateStorage(TextureUploadTarget::Texture2D, 0, {{512, 512, 1}, 0});
            // stencilTex->SetMipmapLevel({{512, 512, 1}, 0, false, 0, {nullptr, 0}});
            fbo0->AttachTexture(FramebufferAttachmentType::Color0, colorTex);
            fbo0->AttachTexture(FramebufferAttachmentType::Depth, depthTex);
            fbo0->AttachTexture(FramebufferAttachmentType::Stencil, stencilTex);
            GLImpl::FramebufferImpl::pDefaultFramebufferInfo =
                new GLImpl::FramebufferImpl::DefaultFramebufferInfo(fbo0, colorTex, depthTex, stencilTex);
        }
    } // namespace MG_Impl
} // namespace MobileGL