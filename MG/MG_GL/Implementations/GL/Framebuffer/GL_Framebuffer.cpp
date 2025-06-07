//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_Framebuffer.h"

namespace MG_GL::GL {
    void ReleaseFramebufferResources(MG_Diligent::GLFramebufferInfo& fbInfo) {
        if (fbInfo.pFramebuffer) {
            fbInfo.pFramebuffer->Release();
            fbInfo.pFramebuffer = nullptr;
        }

        if (fbInfo.pRenderPass) {
            fbInfo.pRenderPass->Release();
            fbInfo.pRenderPass = nullptr;
        }

        for (auto& rtv : fbInfo.ColorRTVs) {
            if (rtv) {
                rtv->Release();
                rtv = nullptr;
            }
        }
        fbInfo.ColorRTVs.clear();

        if (fbInfo.pDepthStencilRTV) {
            fbInfo.pDepthStencilRTV->Release();
            fbInfo.pDepthStencilRTV = nullptr;
        }
    }

    void CreateRenderPassAndFramebuffer(GLuint framebuffer,
                                        const FramebufferObject& fbo,
                                        MG_Diligent::GLFramebufferInfo& fbInfo) {
        Diligent::Uint32 width = 0;
        Diligent::Uint32 height = 0;

        if (framebuffer == 0) {
            if (MG_Diligent::g_pSwapChain) {
                width = MG_Diligent::g_pSwapChain->GetDesc().Width;
                height = MG_Diligent::g_pSwapChain->GetDesc().Height;
                MG_Util::Debug::LogD("CreateRenderPassAndFramebuffer: Using swap chain dimensions: %ux%u", width, height);
            } else {
                MG_Util::Debug::LogE("CreateRenderPassAndFramebuffer: Swap chain not initialized for default framebuffer");
                return;
            }
        }
        else {
            if (!fbInfo.ColorRTVs.empty() && fbInfo.ColorRTVs[0]) {
                const auto& desc = fbInfo.ColorRTVs[0]->GetTexture()->GetDesc();
                width = desc.Width;
                height = desc.Height;
                MG_Util::Debug::LogD("CreateRenderPassAndFramebuffer: Using color attachment 0 for dimensions: %ux%u", width, height);
            } else if (fbInfo.pDepthStencilRTV) {
                const auto& desc = fbInfo.pDepthStencilRTV->GetTexture()->GetDesc();
                width = desc.Width;
                height = desc.Height;
                MG_Util::Debug::LogD("CreateRenderPassAndFramebuffer: Using depth/stencil attachment for dimensions: %ux%u", width, height);
            } else {
                MG_Util::Debug::LogE("CreateRenderPassAndFramebuffer: Failed to determine framebuffer dimensions. No attachments.");
                return;
            }
        }

        if (width == 0 || height == 0) {
            MG_Util::Debug::LogE("CreateRenderPassAndFramebuffer: Failed to determine framebuffer dimensions. Width or Height is 0.");
            return;
        }

        Diligent::RenderPassDesc RPDesc;
        std::vector<Diligent::RenderPassAttachmentDesc> Attachments;
        std::vector<Diligent::SubpassDesc> Subpasses;
        std::vector<Diligent::AttachmentReference> ColorRefs;

        for (size_t i = 0; i < fbInfo.ColorRTVs.size(); ++i) {
            if (fbInfo.ColorRTVs[i]) {
                Diligent::RenderPassAttachmentDesc ColorAttachment;
                ColorAttachment.Format = fbInfo.ColorRTVs[i]->GetDesc().Format;
                ColorAttachment.SampleCount = 1;
                ColorAttachment.InitialState = Diligent::RESOURCE_STATE_RENDER_TARGET;
                ColorAttachment.FinalState = Diligent::RESOURCE_STATE_RENDER_TARGET;
                ColorAttachment.LoadOp = Diligent::ATTACHMENT_LOAD_OP_LOAD;
                ColorAttachment.StoreOp = Diligent::ATTACHMENT_STORE_OP_STORE;
                Attachments.push_back(ColorAttachment);

                Diligent::AttachmentReference ColorRef;
                ColorRef.AttachmentIndex = static_cast<Diligent::Uint32>(Attachments.size() - 1);
                ColorRef.State = Diligent::RESOURCE_STATE_RENDER_TARGET;
                ColorRefs.push_back(ColorRef);
                MG_Util::Debug::LogD("CreateRenderPassAndFramebuffer: Added color attachment %zu", i);
            }
        }

        Diligent::AttachmentReference DepthRef;
        if (fbInfo.pDepthStencilRTV) {
            Diligent::RenderPassAttachmentDesc DepthAttachment;
            DepthAttachment.Format = fbInfo.DepthStencilFormat;
            DepthAttachment.SampleCount = 1;
            DepthAttachment.InitialState = Diligent::RESOURCE_STATE_DEPTH_WRITE;
            DepthAttachment.FinalState = Diligent::RESOURCE_STATE_DEPTH_WRITE;
            DepthAttachment.LoadOp = Diligent::ATTACHMENT_LOAD_OP_LOAD;
            DepthAttachment.StoreOp = Diligent::ATTACHMENT_STORE_OP_STORE;
            DepthAttachment.StencilLoadOp = Diligent::ATTACHMENT_LOAD_OP_LOAD;
            DepthAttachment.StencilStoreOp = Diligent::ATTACHMENT_STORE_OP_STORE;
            Attachments.push_back(DepthAttachment);

            DepthRef.AttachmentIndex = static_cast<Diligent::Uint32>(Attachments.size() - 1);
            DepthRef.State = Diligent::RESOURCE_STATE_DEPTH_WRITE;
            MG_Util::Debug::LogD("CreateRenderPassAndFramebuffer: Added depth/stencil attachment");
        }

        Diligent::SubpassDesc Subpass;
        Subpass.RenderTargetAttachmentCount = static_cast<Diligent::Uint32>(ColorRefs.size());
        Subpass.pRenderTargetAttachments = ColorRefs.empty() ? nullptr : ColorRefs.data();
        Subpass.pDepthStencilAttachment = fbInfo.pDepthStencilRTV ? &DepthRef : nullptr;
        Subpasses.push_back(Subpass);

        RPDesc.AttachmentCount = static_cast<Diligent::Uint32>(Attachments.size());
        RPDesc.pAttachments = Attachments.empty() ? nullptr : Attachments.data();
        RPDesc.SubpassCount = static_cast<Diligent::Uint32>(Subpasses.size());
        RPDesc.pSubpasses = Subpasses.data();

        MG_Util::Debug::LogD("CreateRenderPassAndFramebuffer: Creating RenderPass with %u attachments and %u subpasses.",
                             RPDesc.AttachmentCount, RPDesc.SubpassCount);

        MG_Diligent::g_pDevice->CreateRenderPass(RPDesc, &fbInfo.pRenderPass);
        if (!fbInfo.pRenderPass) {
            MG_Util::Debug::LogE("CreateRenderPassAndFramebuffer: Failed to create RenderPass.");
            return;
        }
        MG_Util::Debug::LogD("CreateRenderPassAndFramebuffer: RenderPass created successfully.");

        Diligent::FramebufferDesc FBDesc;
        FBDesc.Name = "Framebuffer";
        FBDesc.pRenderPass = fbInfo.pRenderPass;
        FBDesc.AttachmentCount = static_cast<Diligent::Uint32>(Attachments.size());

        std::vector<Diligent::ITextureView *> FBAtachments(Attachments.size());
        for (size_t i = 0; i < ColorRefs.size(); ++i) {
            if (i < fbInfo.ColorRTVs.size() && fbInfo.ColorRTVs[i]) {
                FBAtachments[i] = fbInfo.ColorRTVs[i];
            }
        }

        if (fbInfo.pDepthStencilRTV) {
            FBAtachments[ColorRefs.size()] = fbInfo.pDepthStencilRTV;
        }

        FBDesc.ppAttachments = FBAtachments.data();
        FBDesc.Width = width;
        FBDesc.Height = height;
        FBDesc.NumArraySlices = 1;

        MG_Util::Debug::LogD("CreateRenderPassAndFramebuffer: Creating Framebuffer '%s' with %u attachments, Dimensions: %ux%u.",
                             FBDesc.Name, FBDesc.AttachmentCount, FBDesc.Width, FBDesc.Height);

        fbInfo.ClearValues.clear();
        for (size_t i = 0; i < fbInfo.ColorRTVs.size(); ++i) {
            Diligent::OptimizedClearValue clearValue;
            clearValue.Color[0] = 0.0f;
            clearValue.Color[1] = 0.0f;
            clearValue.Color[2] = 0.0f;
            clearValue.Color[3] = 1.0f;
            fbInfo.ClearValues.push_back(clearValue);
        }

        if (fbInfo.pDepthStencilRTV) {
            Diligent::OptimizedClearValue clearValue;
            clearValue.DepthStencil.Depth = 1.0f;
            clearValue.DepthStencil.Stencil = 0;
            fbInfo.ClearValues.push_back(clearValue);
        }

        MG_Diligent::g_pDevice->CreateFramebuffer(FBDesc, &fbInfo.pFramebuffer);
        if (!fbInfo.pFramebuffer) {
            MG_Util::Debug::LogE("CreateRenderPassAndFramebuffer: Failed to create Framebuffer.");
            return;
        }
        MG_Util::Debug::LogD("CreateRenderPassAndFramebuffer: Framebuffer created successfully.");
    }

    void UpdateDefaultFramebuffer() {
        auto it = MG_Diligent::g_FramebufferMap.find(0);
        if (it == MG_Diligent::g_FramebufferMap.end()) {
            MG_Diligent::g_FramebufferMap[0] = MG_Diligent::GLFramebufferInfo();
        }

        MG_Diligent::GLFramebufferInfo& fbInfo = MG_Diligent::g_FramebufferMap[0];

        ReleaseFramebufferResources(fbInfo);

        if (MG_Diligent::g_pSwapChain) {
            Diligent::ITextureView* pRTV = MG_Diligent::g_pSwapChain->GetCurrentBackBufferRTV();
            Diligent::ITextureView* pDSV = MG_Diligent::g_pSwapChain->GetDepthBufferDSV();

            if (pRTV) {
                fbInfo.ColorRTVs = {pRTV};
            } else {
                MG_Util::Debug::LogE("Failed to get swap chain back buffer RTV");
            }

            if (pDSV) {
                fbInfo.pDepthStencilRTV = pDSV;
                fbInfo.DepthStencilFormat = MG_Diligent::g_pSwapChain->GetDesc().DepthBufferFormat;
                fbInfo.HasDepthStencil = true;
            } else {
                MG_Util::Debug::LogE("Failed to get swap chain depth buffer DSV");
            }

            CreateRenderPassAndFramebuffer(0, FramebufferObject(), fbInfo);
        } else {
            MG_Util::Debug::LogE("Swap chain not initialized for default framebuffer");
        }
    }

    void GenFramebuffers(GLsizei n, GLuint* framebuffers) {
        MG_Util::Debug::LogD("glGenFramebuffers, n: %d, framebuffers: %p", n, framebuffers);
        GLenum result = MG_State::CreateFramebuffers(n, framebuffers);
        if (result == GL_NO_ERROR) {
            for (GLsizei i = 0; i < n; ++i) {
                MG_Diligent::GLFramebufferInfo fbInfo;
                MG_Diligent::g_FramebufferMap[framebuffers[i]] = fbInfo;
            }
            return;
        };
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Framebuffer generation failed: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void DeleteFramebuffers(GLsizei n, const GLuint* framebuffers) {
        MG_Util::Debug::LogD("glDeleteFramebuffers, n: %d, framebuffers: %p", n, framebuffers);
        for (GLsizei i = 0; i < n; ++i) {
            GLenum result = MG_State::DeleteFramebuffer(framebuffers[i]);
            if (result != GL_NO_ERROR) {
                MG_State::SetError(result);
                MG_Util::Debug::LogE("Failed to delete framebuffer %u: %s",
                                     framebuffers[i], MG_Util::Debug::GLEnumToString(result));
            } else {
                GLuint fb = framebuffers[i];
                auto it = MG_Diligent::g_FramebufferMap.find(fb);
                if (it != MG_Diligent::g_FramebufferMap.end()) {
                    MG_Diligent::GLFramebufferInfo& fbInfo = it->second;

                    if (fbInfo.pFramebuffer) {
                        fbInfo.pFramebuffer->Release();
                    }

                    if (fbInfo.pRenderPass) {
                        fbInfo.pRenderPass->Release();
                    }

                    for (auto& rtv : fbInfo.ColorRTVs) {
                        if (rtv) rtv->Release();
                    }

                    if (fbInfo.pDepthStencilRTV) {
                        fbInfo.pDepthStencilRTV->Release();
                    }

                    MG_Diligent::g_FramebufferMap.erase(it);
                }
            }
        }
    }

    void BindFramebuffer(GLenum target, GLuint framebuffer) {
        MG_Util::Debug::LogD("glBindFramebuffer, target: %s, fb: %u",
                             MG_Util::Debug::GLEnumToString(target), framebuffer);
        GLenum result = MG_State::BindFramebuffer(target, framebuffer);
        if (result == GL_NO_ERROR) {
            if (MG_Diligent::IsInRenderPass) {
                MG_Diligent::g_pContext->EndRenderPass();
                MG_Diligent::IsInRenderPass = false;
            }

            if (framebuffer == 0) {
                UpdateDefaultFramebuffer();
            }

            FramebufferObject* pFBO = MG_State_T::framebufferState->GetCurrentFBO(target);
            auto it = MG_Diligent::g_FramebufferMap.find(framebuffer);

            if (it == MG_Diligent::g_FramebufferMap.end()) {
                MG_Util::Debug::LogE("Framebuffer %u not found in map", framebuffer);
                return;
            }

            MG_Diligent::GLFramebufferInfo& fbInfo = it->second;

            if (!fbInfo.pRenderPass || !fbInfo.pFramebuffer) {
                CreateRenderPassAndFramebuffer(framebuffer, pFBO ? *pFBO : FramebufferObject(), fbInfo);
            }

            MG_Diligent::g_pContext->SetRenderTargets(
                    static_cast<Diligent::Uint32>(fbInfo.ColorRTVs.size()),
                    fbInfo.ColorRTVs.empty() ? nullptr : fbInfo.ColorRTVs.data(),
                    fbInfo.pDepthStencilRTV,
                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
            );

            if (fbInfo.pRenderPass && fbInfo.pFramebuffer) {
                Diligent::BeginRenderPassAttribs beginRenderPassAttribs;

                beginRenderPassAttribs.pFramebuffer = fbInfo.pFramebuffer;
                beginRenderPassAttribs.pRenderPass = fbInfo.pRenderPass;
                beginRenderPassAttribs.ClearValueCount = fbInfo.ClearValues.size();
                beginRenderPassAttribs.pClearValues = fbInfo.ClearValues.data();
                beginRenderPassAttribs.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

                MG_Diligent::g_pContext->BeginRenderPass(beginRenderPassAttribs);
                MG_Diligent::IsInRenderPass = true;
            }
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Framebuffer bind error: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget,
                              GLuint texture, GLint level) {
        MG_Util::Debug::LogD("glFramebufferTexture2D, target: %s, attach: %s, textarget: %s, tex: %u, level: %d",
                             MG_Util::Debug::GLEnumToString(target),
                             MG_Util::Debug::GLEnumToString(attachment),
                             MG_Util::Debug::GLEnumToString(textarget),
                             texture, level);

        GLuint currentFB = MG_State_T::framebufferState->currentBindings_[target];
        
        GLenum result = MG_State::AttachTexture2DToFramebuffer(
                target, attachment, textarget, texture, level
        );

        if (result == GL_NO_ERROR) {
            auto it = MG_Diligent::g_FramebufferMap.find(currentFB);
            if (it == MG_Diligent::g_FramebufferMap.end()) {
                MG_Util::Debug::LogE("Framebuffer %u not found in map", currentFB);
                return;
            }

            MG_Diligent::GLFramebufferInfo& fbInfo = it->second;

            if (fbInfo.pFramebuffer) {
                fbInfo.pFramebuffer->Release();
                fbInfo.pFramebuffer = nullptr;
                MG_Util::Debug::LogD("Released existing framebuffer");
            }
            if (fbInfo.pRenderPass) {
                fbInfo.pRenderPass->Release();
                fbInfo.pRenderPass = nullptr;
                MG_Util::Debug::LogD("Released existing render pass");
            }

            if (texture == 0) {
                MG_Util::Debug::LogD("Unbinding attachment: %s", MG_Util::Debug::GLEnumToString(attachment));

                if (attachment == GL_DEPTH_ATTACHMENT ||
                    attachment == GL_STENCIL_ATTACHMENT ||
                    attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
                    if (fbInfo.pDepthStencilRTV) {
                        fbInfo.pDepthStencilRTV->Release();
                        fbInfo.pDepthStencilRTV = nullptr;
                        MG_Util::Debug::LogD("Released depth/stencil attachment");
                    }
                } else {
                    size_t index = attachment - GL_COLOR_ATTACHMENT0;
                    if (index < fbInfo.ColorRTVs.size() && fbInfo.ColorRTVs[index]) {
                        fbInfo.ColorRTVs[index]->Release();
                        fbInfo.ColorRTVs[index] = nullptr;
                        MG_Util::Debug::LogD("Released color attachment %zu", index);
                    }
                }
            } else {
                Diligent::ITexture* pTexture = MG_Diligent::g_TextureMap[texture];
                if (!pTexture) {
                    MG_Util::Debug::LogE("Texture %u exists in map but pointer is null", texture);
                    return;
                }
                const auto& texDesc = pTexture->GetDesc();
                MG_Util::Debug::LogD("Attaching texture: %s (size: %ux%u)",
                                     texDesc.Name,
                                     texDesc.Width, texDesc.Height);
                Diligent::TextureViewDesc ViewDesc;
                ViewDesc.TextureDim = Diligent::RESOURCE_DIM_TEX_2D;
                ViewDesc.MostDetailedMip = level;
                ViewDesc.NumMipLevels = 1;

                if (attachment == GL_DEPTH_ATTACHMENT ||
                    attachment == GL_STENCIL_ATTACHMENT ||
                    attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
                    MG_Util::Debug::LogD("Creating depth/stencil attachment");

                    if (fbInfo.pDepthStencilRTV) {
                        fbInfo.pDepthStencilRTV->Release();
                        MG_Util::Debug::LogD("Released existing depth/stencil attachment");
                    }

                    ViewDesc.ViewType = Diligent::TEXTURE_VIEW_DEPTH_STENCIL;
                    fbInfo.DepthStencilFormat = texDesc.Format;
                    pTexture->CreateView(ViewDesc, &fbInfo.pDepthStencilRTV);

                    if (fbInfo.pDepthStencilRTV) {
                        fbInfo.HasDepthStencil = true;
                        MG_Util::Debug::LogD("Created new depth/stencil RTV");
                    } else {
                        MG_Util::Debug::LogE("Failed to create depth/stencil RTV");
                    }
                } else {
                    size_t index = attachment - GL_COLOR_ATTACHMENT0;
                    MG_Util::Debug::LogD("Creating color attachment at index %zu", index);

                    if (index >= fbInfo.ColorRTVs.size()) {
                        fbInfo.ColorRTVs.resize(index + 1, nullptr);
                        MG_Util::Debug::LogD("Resized color attachments to %zu", fbInfo.ColorRTVs.size());
                    }

                    if (fbInfo.ColorRTVs[index]) {
                        fbInfo.ColorRTVs[index]->Release();
                        MG_Util::Debug::LogD("Released existing color attachment %zu", index);
                    }

                    ViewDesc.ViewType = Diligent::TEXTURE_VIEW_RENDER_TARGET;
                    pTexture->CreateView(ViewDesc, &fbInfo.ColorRTVs[index]);

                    if (fbInfo.ColorRTVs[index]) {
                        MG_Util::Debug::LogD("Created new color RTV at index %zu", index);
                    } else {
                        MG_Util::Debug::LogE("Failed to create color RTV at index %zu", index);
                    }
                }
            }

            MG_Util::Debug::LogD("FramebufferTexture2D completed successfully");
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Texture attachment failed: %s", MG_Util::Debug::GLEnumToString(result));
    }

    GLenum CheckFramebufferStatus(GLenum target) {
        MG_Util::Debug::LogD("glCheckFramebufferStatus, target: %s",
                             MG_Util::Debug::GLEnumToString(target));
        GLenum result = MG_State::ValidateFramebufferCompleteness(target);
        if (result >= GL_FRAMEBUFFER_COMPLETE) {
            MG_Util::Debug::LogD("Framebuffer status: %s", MG_Util::Debug::GLEnumToString(result));
            return result;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Framebuffer incomplete: %s",
                             MG_Util::Debug::GLEnumToString(result));
        return result;
    }

    void BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                         GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
        GLuint readFB = MG_State_T::framebufferState->currentBindings_[GL_READ_FRAMEBUFFER];
        GLuint drawFB = MG_State_T::framebufferState->currentBindings_[GL_DRAW_FRAMEBUFFER];

        MG_Diligent::GLFramebufferInfo* pSrcFB = nullptr;
        MG_Diligent::GLFramebufferInfo* pDstFB = nullptr;

        if (readFB != 0) pSrcFB = &MG_Diligent::g_FramebufferMap[readFB];
        if (drawFB != 0) pDstFB = &MG_Diligent::g_FramebufferMap[drawFB];

        if (!pSrcFB) pSrcFB = &MG_Diligent::g_FramebufferMap[0];
        if (!pDstFB) pDstFB = &MG_Diligent::g_FramebufferMap[0];

        bool copyColor = (mask & GL_COLOR_BUFFER_BIT) != 0;
        bool copyDepth = (mask & GL_DEPTH_BUFFER_BIT) != 0;
        bool copyStencil = (mask & GL_STENCIL_BUFFER_BIT) != 0;

        if (copyColor && !pSrcFB->ColorRTVs.empty() && !pDstFB->ColorRTVs.empty()) {
            for (size_t i = 0; i < pSrcFB->ColorRTVs.size() && i < pDstFB->ColorRTVs.size(); ++i) {
                if (pSrcFB->ColorRTVs[i] && pDstFB->ColorRTVs[i]) {
                    Diligent::CopyTextureAttribs CopyAttribs;
                    CopyAttribs.pSrcTexture = pSrcFB->ColorRTVs[i]->GetTexture();
                    CopyAttribs.pDstTexture = pDstFB->ColorRTVs[i]->GetTexture();
                    CopyAttribs.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
                    CopyAttribs.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

                    Diligent::Box SrcBox;
                    SrcBox.MinX = srcX0;
                    SrcBox.MinY = srcY0;
                    SrcBox.MaxX = srcX1;
                    SrcBox.MaxY = srcY1;
                    CopyAttribs.pSrcBox = &SrcBox;

                    CopyAttribs.DstX = dstX0;
                    CopyAttribs.DstY = dstY0;

                    MG_Diligent::g_pContext->CopyTexture(CopyAttribs);
                }
            }
        }

        if ((copyDepth || copyStencil) && pSrcFB->pDepthStencilRTV && pDstFB->pDepthStencilRTV) {
            Diligent::CopyTextureAttribs CopyAttribs;
            CopyAttribs.pSrcTexture = pSrcFB->pDepthStencilRTV->GetTexture();
            CopyAttribs.pDstTexture = pDstFB->pDepthStencilRTV->GetTexture();
            CopyAttribs.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
            CopyAttribs.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

            Diligent::Box SrcBox;
            SrcBox.MinX = srcX0;
            SrcBox.MinY = srcY0;
            SrcBox.MaxX = srcX1;
            SrcBox.MaxY = srcY1;
            CopyAttribs.pSrcBox = &SrcBox;

            CopyAttribs.DstX = dstX0;
            CopyAttribs.DstY = dstY0;

            MG_Diligent::g_pContext->CopyTexture(CopyAttribs);
        }
    }
}
