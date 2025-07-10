//
// Created by BZLZHH on 2025/3/15.
//

#include "../../../../Includes.h"

namespace MG_GL::GL {
    void CreateRenderPassAndFramebuffer(GLuint framebuffer,
                                        const FramebufferObject& fbo,
                                        MG_Diligent::GLFramebufferInfo& fbInfo);
    
    void Clear(GLbitfield mask) {
        GLuint drawFramebuffer = MG_State_T::framebufferState->currentBindings_[GL_DRAW_FRAMEBUFFER];
        if (drawFramebuffer == 0) {
            drawFramebuffer = 0;
        }

        auto it = MG_Diligent::g_FramebufferMap.find(drawFramebuffer);
        if (it == MG_Diligent::g_FramebufferMap.end()) {
            MG_Util::Debug::LogE("Framebuffer not found: %u", drawFramebuffer);
            return;
        }

        MG_Diligent::GLFramebufferInfo& fbInfo = it->second;
        auto& commonState = *MG_State_T::commonState;

        MG_Util::Debug::LogD("Setting render targets: %zu color attachments", fbInfo.ColorRTVs.size());
        MG_Diligent::g_pContext->SetRenderTargets(
                static_cast<Diligent::Uint32>(fbInfo.ColorRTVs.size()),
                fbInfo.ColorRTVs.data(),
                fbInfo.pDepthStencilRTV,
                ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
        );

//        if (MG_Diligent::IsInRenderPass) {
//            MG_Util::Debug::LogD("Ending render pass before clear operation");
//            MG_Diligent::g_pContext->EndRenderPass();
//            MG_Diligent::IsInRenderPass = false;
//        }

//        MG_Util::Debug::LogD("Beginning render pass for clear operation");

//        if (!fbInfo.pRenderPass) {
//            MG_GL::GL::CreateRenderPassAndFramebuffer(drawFramebuffer,
//                                                      *MG_State_T::framebufferState->GetCurrentFBO(GL_DRAW_FRAMEBUFFER), fbInfo);
//        }
//        Diligent::BeginRenderPassAttribs beginAttribs;
//        beginAttribs.pRenderPass = fbInfo.pRenderPass;
//        beginAttribs.pFramebuffer = fbInfo.pFramebuffer;
//        beginAttribs.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
//        beginAttribs.ClearValueCount = 0;
//        beginAttribs.pClearValues = nullptr;

//        MG_Diligent::g_pContext->BeginRenderPass(beginAttribs);
//        MG_Diligent::IsInRenderPass = true;
        
        if (mask & GL_COLOR_BUFFER_BIT) {
            for (size_t i = 0; i < fbInfo.ColorRTVs.size(); ++i) {
                if (fbInfo.ColorRTVs[i]) {
                    MG_Util::Debug::LogD("Clearing color attachment %zu", i);
                    MG_Diligent::g_pContext->ClearRenderTarget(
                            fbInfo.ColorRTVs[i],
                            commonState.clearColor,
                            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
                    );
                }
            }
        }

        if (mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) {
            if (fbInfo.pDepthStencilRTV) {
                Diligent::Uint32 clearFlags = 0;
                if (mask & GL_DEPTH_BUFFER_BIT) clearFlags |= Diligent::CLEAR_DEPTH_FLAG;
                if (mask & GL_STENCIL_BUFFER_BIT) clearFlags |= Diligent::CLEAR_STENCIL_FLAG;

                MG_Util::Debug::LogD("Clearing depth/stencil (flags: %u)", clearFlags);
                MG_Diligent::g_pContext->ClearDepthStencil(
                        fbInfo.pDepthStencilRTV,
                        static_cast<Diligent::CLEAR_DEPTH_STENCIL_FLAGS>(clearFlags),
                        commonState.clearDepth,
                        0,
                        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
                );
            }
        }

//        MG_Util::Debug::LogD("Ending render pass after clear operation");
//        MG_Diligent::g_pContext->EndRenderPass();
//        MG_Diligent::IsInRenderPass = false;
    }

    void Enable(GLenum cap) {
        MG_Util::Debug::LogD("glEnable, cap: %s", MG_Util::Debug::GLEnumToString(cap));
        GLenum result = MG_State::glEnable(cap);
        if (result == GL_NO_ERROR) {
            MG_Diligent::g_PSOManager.MarkPSODirty(MG_State_T::programState->currentProgram_);
            return; 
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error enabling capability: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void Disable(GLenum cap) {
        MG_Util::Debug::LogD("glDisable, cap: %s", MG_Util::Debug::GLEnumToString(cap));
        GLenum result = MG_State::glDisable(cap);
        if (result == GL_NO_ERROR) {
            MG_Diligent::g_PSOManager.MarkPSODirty(MG_State_T::programState->currentProgram_);
            return; 
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error disabling capability: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void BlendFunc(GLenum sfactor, GLenum dfactor) {
        MG_Util::Debug::LogD("glBlendFunc, sfactor: %s, dfactor: %s",
                             MG_Util::Debug::GLEnumToString(sfactor),
                             MG_Util::Debug::GLEnumToString(dfactor));
        GLenum result = MG_State::glBlendFunc(sfactor, dfactor);
        if (result == GL_NO_ERROR) {
            MG_Diligent::g_PSOManager.MarkPSODirty(MG_State_T::programState->currentProgram_);
            return; 
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting blend func: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
        MG_Util::Debug::LogD("glBlendFuncSeparate, srcRGB: %s, dstRGB: %s, srcAlpha: %s, dstAlpha: %s",
                             MG_Util::Debug::GLEnumToString(srcRGB),
                             MG_Util::Debug::GLEnumToString(dstRGB),
                             MG_Util::Debug::GLEnumToString(srcAlpha),
                             MG_Util::Debug::GLEnumToString(dstAlpha));
        GLenum result = MG_State::glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
        if (result == GL_NO_ERROR) {
            MG_Diligent::g_PSOManager.MarkPSODirty(MG_State_T::programState->currentProgram_);
            return; 
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting separate blend func: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
        MG_Util::Debug::LogD("glClearColor, rgba: [%.2f, %.2f, %.2f, %.2f]", red, green, blue, alpha);
        GLenum result = MG_State::glClearColor(red, green, blue, alpha);
        if (result == GL_NO_ERROR) {
            MG_Diligent::g_PSOManager.MarkPSODirty(MG_State_T::programState->currentProgram_);
            return; 
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting clear color: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void ClearDepth(GLdouble depth) {
        MG_Util::Debug::LogD("glClearDepth, depth: %.3f", depth);
        GLenum result = MG_State::glClearDepth(depth);
        if (result == GL_NO_ERROR) {
            MG_Diligent::g_PSOManager.MarkPSODirty(MG_State_T::programState->currentProgram_);
            return; 
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting clear depth: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
        MG_Util::Debug::LogD("glColorMask, rgba: [%d, %d, %d, %d]", red, green, blue, alpha);
        GLenum result = MG_State::glColorMask(red, green, blue, alpha);
        if (result == GL_NO_ERROR) {
            MG_Diligent::g_PSOManager.MarkPSODirty(MG_State_T::programState->currentProgram_);
            return; 
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting color mask: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void DepthFunc(GLenum func) {
        MG_Util::Debug::LogD("glDepthFunc, func: %s", MG_Util::Debug::GLEnumToString(func));
        GLenum result = MG_State::glDepthFunc(func);
        if (result == GL_NO_ERROR) {
            MG_Diligent::g_PSOManager.MarkPSODirty(MG_State_T::programState->currentProgram_);
            return; 
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting depth func: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void DepthMask(GLboolean flag) {
        MG_Util::Debug::LogD("glDepthMask, flag: %d", flag);
        GLenum result = MG_State::glDepthMask(flag);
        if (result == GL_NO_ERROR) {
            MG_Diligent::g_PSOManager.MarkPSODirty(MG_State_T::programState->currentProgram_);
            return; 
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting depth mask: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void Viewport(GLint x, GLint y, GLsizei width, GLsizei height) {
        MG_Util::Debug::LogD("glViewport, x: %d, y: %d, width: %d, height: %d",
                             x, y, width, height);
        GLenum result = MG_State::glViewport(x, y, width, height);
        if (result == GL_NO_ERROR) {
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error setting viewport: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void PixelStorei(GLenum pname, GLint param) {
        MG_Util::Debug::LogD("glPixelStorei, pname: %s, param: %d", MG_Util::Debug::GLEnumToString(pname), param);
        GLenum result = MG_State::SetPixelStoreInt(pname,param);
        if (result == GL_NO_ERROR) {
            MG_Diligent::g_PSOManager.MarkPSODirty(MG_State_T::programState->currentProgram_);
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }   
}