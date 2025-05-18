//
// Created by Swung 0x48 on 2025-05-18.
//

#include "EGL_impl.h"

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

// Pixel shader simply outputs interpolated vertex color
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

    EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, NativeWindowType window,
                                      const EGLint *attrib_list) {
        void *handle = dlopen("libGraphicsEngineOpenGL.so", RTLD_LAZY);
        auto Diligent_GetEngineFactoryOpenGL =
                (Diligent_GetEngineFactoryOpenGL_t) dlsym(handle,
                                                          "Diligent_GetEngineFactoryOpenGL");

        auto *pFactoryOpenGL = Diligent_GetEngineFactoryOpenGL();
        ::Diligent::EngineGLCreateInfo EngineCI;
        auto *nativeWindow = static_cast<ANativeWindow *>(window);
        EngineCI.Window.pAWindow = nativeWindow;

        ::Diligent::SwapChainDesc SCDesc;

        auto &ctx = MG_EGL::Diligent::DeviceContext::GetInstance();

        pFactoryOpenGL->CreateDeviceAndSwapChainGL(
                EngineCI, &ctx.pDevice, &ctx.pContext, SCDesc, &ctx.pSwapChain);

        // Pipeline state object encompasses configuration of all GPU stages

        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        PSOCreateInfo.PSODesc.Name = "Simple triangle PSO";

        // This is a graphics pipeline
        PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        // clang-format off
        // This tutorial will render to a single render target
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
        // Set render target format which is the format of the swap chain's color buffer
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = ctx.pSwapChain->GetDesc().ColorBufferFormat;
        // Use the depth buffer format from the swap chain
        PSOCreateInfo.GraphicsPipeline.DSVFormat = ctx.pSwapChain->GetDesc().DepthBufferFormat;
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // No back face culling for this tutorial
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        // Disable depth testing
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
        // clang-format on

        ShaderCreateInfo ShaderCI;
        // Tell the system that the shader source code is in HLSL.
        // For OpenGL, the engine will convert this into GLSL under the hood.
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        ShaderCI.Desc.UseCombinedTextureSamplers = true;
        // Create a vertex shader
        RefCntAutoPtr <IShader> pVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name = "Triangle vertex shader";
            ShaderCI.Source = VSSource;
            ctx.pDevice->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        RefCntAutoPtr <IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name = "Triangle pixel shader";
            ShaderCI.Source = PSSource;
            ctx.pDevice->CreateShader(ShaderCI, &pPS);
        }

        // Finally, create the pipeline state
        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;
        ctx.pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &ctx.pPSO);

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

    EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface draw) {
        auto &ctx = MG_EGL::Diligent::DeviceContext::GetInstance();

        // Clear the back buffer
        const float ClearColor[] = {0.350f, 0.350f, 0.350f, 1.0f};
        // Let the engine perform required state transitions
        ITextureView *pRTV = ctx.pSwapChain->GetCurrentBackBufferRTV();
        ITextureView *pDSV = ctx.pSwapChain->GetDepthBufferDSV();
        ctx.pContext->ClearRenderTarget(pRTV, ClearColor,
                                        RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        ctx.pContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0,
                                        RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Set the pipeline state in the immediate context
        ctx.pContext->SetPipelineState(ctx.pPSO);

        // Typically we should now call CommitShaderResources(), however shaders in this example don't
        // use any resources.

        DrawAttribs drawAttrs;
        drawAttrs.NumVertices = 3; // We will render 3 vertices
        ctx.pContext->Draw(drawAttrs);

        ctx.pSwapChain->Present();

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
