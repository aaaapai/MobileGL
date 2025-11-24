#include "Init.h"
#include "GLImpl/Texture/ProxyTexture.h"
#include "GLImpl/Framebuffer/GL_Framebuffer.h"

#include "MG_State/GLState/TextureState/TextureObject1D.h"
#include "MG_State/GLState/TextureState/TextureObject2D.h"
#include "MG_State/GLState/TextureState/TextureObject3D.h"

namespace MobileGL {
    namespace MG_Impl {
        void Init() {
            MGLOG_D("Initializing MobileGL Implementation...");
            GLImpl::TextureImpl::pProxyTextureManager = new GLImpl::TextureImpl::ProxyTextureManager();

            // TODO: get real info in EGL
            auto fbo0 = MG_State::pGLContext->CreateFramebufferObject(0);
            auto colorTex = MakeShared<MG_State::GLState::TextureObject2D>(0);
            colorTex->SetInternalFormat(TextureInternalFormat::RGBA8);
            colorTex->SetMipmapLevel({{512, 512, 1}, 0, false, 0, {nullptr, 0}});
            auto depthTex = MakeShared<MG_State::GLState::TextureObject2D>(0);
            depthTex->SetInternalFormat(TextureInternalFormat::Depth32FStencil8);
            depthTex->SetMipmapLevel({{512, 512, 1}, 0, false, 0, {nullptr, 0}});
            auto stencilTex = MakeShared<MG_State::GLState::TextureObject2D>(0);
            stencilTex->SetInternalFormat(TextureInternalFormat::Depth32FStencil8);
            stencilTex->SetMipmapLevel({{512, 512, 1}, 0, false, 0, {nullptr, 0}});
            fbo0->AttachTexture(FramebufferAttachmentType::Color0, colorTex);
            fbo0->AttachTexture(FramebufferAttachmentType::Depth, depthTex);
            fbo0->AttachTexture(FramebufferAttachmentType::Depth, depthTex);
            fbo0->AttachTexture(FramebufferAttachmentType::Stencil, stencilTex);
            GLImpl::FramebufferImpl::pDefaultFramebufferInfo =
                new GLImpl::FramebufferImpl::DefaultFramebufferInfo(fbo0, colorTex, depthTex, stencilTex);
        }
    } // namespace MG_Impl
} // namespace MobileGL