//
// Created by Swung 0x48 on 2025-05-18.
//

#include "EGL_impl.h"

typedef Diligent::IEngineFactoryVk* (*Diligent_GetEngineFactoryVk_t)();
typedef Diligent::IEngineFactoryOpenGL* (*Diligent_GetEngineFactoryOpenGL_t)();

using namespace Diligent;

namespace MG_EGL::Diligent {
    static const char *VSSource = R"(
struct PSInput
{
    float4 Pos   : SV_POSITION;
    float3 Color : COLOR;
};

void main(in  uint    VertId : SV_VertexID,
          out PSInput PSIn)
{
    float4 Pos[3];
    Pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
    Pos[1] = float4( 0.0, +0.5, 0.0, 1.0);
    Pos[2] = float4(+0.5, -0.5, 0.0, 1.0);

    float3 Col[3];
    Col[0] = float3(1.0, 0.0, 0.0); // red
    Col[1] = float3(0.0, 1.0, 0.0); // green
    Col[2] = float3(0.0, 0.0, 1.0); // blue

    PSIn.Pos   = Pos[VertId];
    PSIn.Color = Col[VertId];
}
)";

    static const char *PSSource = R"(
struct PSInput
{
    float4 Pos   : SV_POSITION;
    float3 Color : COLOR;
};

struct PSOutput
{
    float4 Color : SV_TARGET;
};

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    PSOut.Color = float4(PSIn.Color.rgb, 1.0);
}
)";

    void LoadDiligentCoreOpenGL(NativeWindowType window) {
        void *handle = dlopen("libGraphicsEngineOpenGL.so", RTLD_LAZY);
        auto Diligent_GetEngineFactoryOpenGL =
                (Diligent_GetEngineFactoryOpenGL_t) dlsym(handle,
                                                          "Diligent_GetEngineFactoryOpenGL");

        auto *pFactoryOpenGL = Diligent_GetEngineFactoryOpenGL();
        ::Diligent::EngineGLCreateInfo EngineCI;
        auto *nativeWindow = static_cast<ANativeWindow *>(window);
        EngineCI.Window.pAWindow = nativeWindow;

        ::Diligent::SwapChainDesc SCDesc;

        pFactoryOpenGL->CreateDeviceAndSwapChainGL(
                EngineCI, &ctx.pDevice, &ctx.pContext, SCDesc, &ctx.pSwapChain);
    }

    void LoadDiligentCoreVulkan(NativeWindowType window) {
        void *handle = dlopen("libGraphicsEngineVk.so", RTLD_LAZY);
        auto Diligent_GetEngineFactoryVk =
                (Diligent_GetEngineFactoryVk_t) dlsym(handle,
                                                      "Diligent_GetEngineFactoryVk");

        auto *pFactoryVk = Diligent_GetEngineFactoryVk();
        ::Diligent::EngineVkCreateInfo EngineCI;

        ::Diligent::SwapChainDesc SCDesc;

        pFactoryVk->CreateDeviceAndContextsVk(
                EngineCI, &ctx.pDevice, &ctx.pContext);
        AndroidNativeWindow nativeWindow{window};
        pFactoryVk->CreateSwapChainVk(
                ctx.pDevice, ctx.pContext, SCDesc, nativeWindow, &ctx.pSwapChain);
    }

    void LoadDiligentCore(NativeWindowType window) {
        switch (DILIGENT_BACKEND_TYPE) {
            case MG_Constants::Backend::BACKEND_DILIGENT_VULKAN:
                LoadDiligentCoreVulkan(window);
                break;
            case MG_Constants::Backend::BACKEND_DILIGENT_OPENGL:
            default:
                LoadDiligentCoreOpenGL(window);
                break;
        }

    }

    EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, NativeWindowType window,
                                      const EGLint *attrib_list) {
        LoadDiligentCore(window);

        RenderPassAttachmentDesc RPAttachmentDescs[2];
        RPAttachmentDescs[0].Format = ctx.pSwapChain->GetDesc().ColorBufferFormat;
        RPAttachmentDescs[0].InitialState = RESOURCE_STATE_RENDER_TARGET;
        RPAttachmentDescs[0].FinalState = RESOURCE_STATE_RENDER_TARGET;
        RPAttachmentDescs[0].LoadOp = ATTACHMENT_LOAD_OP_CLEAR;
        RPAttachmentDescs[0].StoreOp = ATTACHMENT_STORE_OP_STORE;
        RPAttachmentDescs[1].Format = ctx.pSwapChain->GetDesc().DepthBufferFormat;
        RPAttachmentDescs[1].InitialState = RESOURCE_STATE_DEPTH_WRITE;
        RPAttachmentDescs[1].FinalState = RESOURCE_STATE_DEPTH_WRITE;
        RPAttachmentDescs[1].LoadOp = ATTACHMENT_LOAD_OP_CLEAR;
        RPAttachmentDescs[1].StoreOp = ATTACHMENT_STORE_OP_DISCARD;

        SubpassDesc Subpass;
        Subpass.InputAttachmentCount = 0;
        Subpass.RenderTargetAttachmentCount = 1;
        AttachmentReference RTAttachmentRef = {0, RESOURCE_STATE_RENDER_TARGET};
        Subpass.pRenderTargetAttachments = &RTAttachmentRef;
        AttachmentReference DSAttachmentRef = {1, RESOURCE_STATE_DEPTH_WRITE};
        Subpass.pDepthStencilAttachment = &DSAttachmentRef;

        RenderPassDesc RPDesc;
        RPDesc.Name = "Main render pass";
        RPDesc.AttachmentCount = 2;
        RPDesc.pAttachments = RPAttachmentDescs;
        RPDesc.SubpassCount = 1;
        RPDesc.pSubpasses = &Subpass;

        ctx.pDevice->CreateRenderPass(RPDesc, &ctx.pRenderPass);

        GraphicsPipelineStateCreateInfo PSOCreateInfo;
        PSOCreateInfo.PSODesc.Name = "Simple triangle PSO";

        PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 0;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = TEX_FORMAT_UNKNOWN;
        PSOCreateInfo.GraphicsPipeline.DSVFormat = TEX_FORMAT_UNKNOWN;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
        PSOCreateInfo.GraphicsPipeline.pRenderPass = ctx.pRenderPass;

        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        ShaderCI.Desc.UseCombinedTextureSamplers = true;
        RefCntAutoPtr <IShader> pVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name = "Triangle vertex shader";
            ShaderCI.Source = VSSource;
            ctx.pDevice->CreateShader(ShaderCI, &pVS);
        }

        RefCntAutoPtr <IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name = "Triangle pixel shader";
            ShaderCI.Source = PSSource;
            ctx.pDevice->CreateShader(ShaderCI, &pPS);
        }

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;
        ctx.pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &ctx.pPSO);

        const auto& SCDesc = ctx.pSwapChain->GetDesc();
        ctx.pFramebuffers.resize(SCDesc.BufferCount);

        ctx.pContext->SetRenderTargets(0, nullptr, nullptr, RESOURCE_STATE_TRANSITION_MODE_NONE);
        return (EGLSurface) 1;
    }

    EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs,
                               EGLint config_size, EGLint *num_config) {
        *num_config = 1;
        return EGL_TRUE;
    }

    EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext shareCtx,
                                const EGLint *attrib_list) {
        return (EGLContext) 1;
    }

    EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor) {
        return EGL_TRUE;
    }

    EGLDisplay eglGetDisplay(NativeDisplayType display) {
        return (EGLDisplay) 1;
    }

    EGLint eglGetError() {
        return EGL_SUCCESS;
    }

    EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
        return EGL_TRUE;
    }

    EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx) {
        return EGL_TRUE;
    }

    EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface) {
        return EGL_TRUE;
    }

    EGLBoolean eglTerminate(EGLDisplay dpy) {
        return EGL_TRUE;
    }

    EGLBoolean eglReleaseThread(void) {
        return EGL_TRUE;
    }

    EGLContext eglGetCurrentContext(void) {
        return (EGLContext) 1;
    }

    EGLBoolean
    eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value) {
        if (attribute == EGL_NATIVE_VISUAL_ID)
            *value = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
        return EGL_TRUE;
    }

    EGLBoolean eglBindAPI(EGLenum api) {
        return EGL_TRUE;
    }

    EGLSurface eglGetCurrentSurface(EGLint readdraw) {
        return (EGLSurface) 1;
    }

    EGLBoolean
    eglQuerySurface(EGLDisplay display, EGLSurface surface, EGLint attribute, EGLint *value) {
        return EGL_TRUE;
    }

    EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval) {
        return EGL_TRUE;
    }

    static int swapBuffersCount = 0;
    static int bufferCount = 0;
    EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface draw) {
        const float ClearColor[] = {0.350f, 0.350f, 0.350f, 1.0f};

        // Framebuffer Creation
        if (!bufferCount) bufferCount = ctx.pSwapChain->GetDesc().BufferCount;
        ITextureView* pDSV = ctx.pSwapChain->GetDepthBufferDSV();
        if (swapBuffersCount < bufferCount) {
            ITextureView *pRTV = ctx.pSwapChain->GetCurrentBackBufferRTV();
            ITextureView *attachments[2];
            attachments[0] = pRTV;
            attachments[1] = pDSV;

            FramebufferDesc FBDesc;
            FBDesc.Name = ("Main framebuffer " + std::to_string(swapBuffersCount)).c_str();
            FBDesc.pRenderPass = ctx.pRenderPass;
            FBDesc.AttachmentCount = 2;
            FBDesc.ppAttachments = attachments;
            ctx.pDevice->CreateFramebuffer(FBDesc, &ctx.pFramebuffers[swapBuffersCount]);
        }

        BeginRenderPassAttribs beginRenderPassAttribs;

        beginRenderPassAttribs.pFramebuffer = ctx.pFramebuffers[swapBuffersCount % bufferCount];
        beginRenderPassAttribs.pRenderPass = ctx.pRenderPass;
        beginRenderPassAttribs.ClearValueCount = 2;
        OptimizedClearValue optimizedClearValues[2];
        optimizedClearValues[0].Color[0] = ClearColor[0];
        optimizedClearValues[0].Color[1] = ClearColor[1];
        optimizedClearValues[0].Color[2] = ClearColor[2];
        optimizedClearValues[0].Color[3] = ClearColor[3];
        optimizedClearValues[1].DepthStencil.Depth = 1.f;
        optimizedClearValues[1].DepthStencil.Stencil = 0;

        beginRenderPassAttribs.pClearValues = optimizedClearValues;
        beginRenderPassAttribs.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        ctx.pContext->BeginRenderPass(beginRenderPassAttribs);

        Viewport vp = {0, 0, (float)ctx.pSwapChain->GetDesc().Width, (float)ctx.pSwapChain->GetDesc().Height, 0, 1};
        ctx.pContext->SetViewports(1, &vp, ctx.pSwapChain->GetDesc().Width, ctx.pSwapChain->GetDesc().Height);

        ctx.pContext->SetPipelineState(ctx.pPSO);

        DrawAttribs drawAttrs;
        drawAttrs.NumVertices = 3;
        drawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
        ctx.pContext->Draw(drawAttrs);

        ctx.pContext->EndRenderPass();

        ctx.pContext->Flush();

        ctx.pSwapChain->Present();

        swapBuffersCount++;
        return EGL_TRUE;
    }

    EGLSurface
    eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) {
        return (EGLSurface) 1;
    }

}

void *libGLES = nullptr, *libEGL = nullptr;

static const char *LibPathPrefixes[] = {
        "",
        "/opt/vc/lib/",
        "/usr/local/lib/",
        "/usr/lib/",
        nullptr
};

static const char *LibExts[] = {
        "so",
        "so.1",
        "so.2",
        "dylib",
        "dll",
        nullptr
};

static const char *GLES3Libs[] = {
        "libGLESv3_CM",
        "libGLESv3",
        nullptr
};

static const char *EGLLibs[] = {
        "libEGL",
        nullptr
};

void *OpenLib(const char **names, const char *override) {
    void *lib = nullptr;

    char path_name[PATH_MAX + 1];
    int flags = RTLD_LOCAL | RTLD_NOW;
    if (override) {
        if ((lib = dlopen(override, flags))) {
            strncpy(path_name, override, PATH_MAX);
            return lib;
        } else {
            MG_Util::Debug::LogE("LIBGL_GLES override failed: %s\n", dlerror());
        }
    }
    for (int p = 0; LibPathPrefixes[p]; p++) {
        for (int i = 0; names[i]; i++) {
            for (int e = 0; LibExts[e]; e++) {
                snprintf(path_name, PATH_MAX, "%s%s.%s", LibPathPrefixes[p], names[i], LibExts[e]);
                if ((lib = dlopen(path_name, flags))) {
                    return lib;
                }
            }
        }
    }
    return lib;
}

typedef __eglMustCastToProperFunctionPointerType (* eglGetProcAddress_t)(const char *procname);
eglGetProcAddress_t real_eglGetProcAddress = nullptr;

__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char *procname) {
    if (strncmp(procname, "eglGetProcAddress", strlen("eglGetProcAddress")) == 0) {
        return (__eglMustCastToProperFunctionPointerType)(&eglGetProcAddress);
    }

    MG_EGL_PROC_EXPORT(eglCreateWindowSurface);
    MG_EGL_PROC_EXPORT(eglChooseConfig);
    MG_EGL_PROC_EXPORT(eglCreateContext);
    MG_EGL_PROC_EXPORT(eglInitialize);
    MG_EGL_PROC_EXPORT(eglGetDisplay);
    MG_EGL_PROC_EXPORT(eglGetError);
    MG_EGL_PROC_EXPORT(eglMakeCurrent);
    MG_EGL_PROC_EXPORT(eglDestroyContext);
    MG_EGL_PROC_EXPORT(eglDestroySurface);
    MG_EGL_PROC_EXPORT(eglTerminate);
    MG_EGL_PROC_EXPORT(eglReleaseThread);
    MG_EGL_PROC_EXPORT(eglGetCurrentContext);
    MG_EGL_PROC_EXPORT(eglGetConfigAttrib);
    MG_EGL_PROC_EXPORT(eglBindAPI);
    MG_EGL_PROC_EXPORT(eglGetCurrentSurface);
    MG_EGL_PROC_EXPORT(eglQuerySurface);
    MG_EGL_PROC_EXPORT(eglSwapInterval);
    MG_EGL_PROC_EXPORT(eglSwapBuffers);
    MG_EGL_PROC_EXPORT(eglCreatePbufferSurface);

    /* TODO: Call real eglGetProcAddress() to get the rest (should be real native GLES functions) */
    if (!libEGL) {
        libEGL = OpenLib(EGLLibs, nullptr);
        real_eglGetProcAddress = (eglGetProcAddress_t)dlsym(libEGL, "eglGetProcAddress");
    }
    if (real_eglGetProcAddress)
        return real_eglGetProcAddress(procname);

    return nullptr;
}
