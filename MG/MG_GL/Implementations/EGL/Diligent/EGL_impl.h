//
// Created by Swung 0x48 on 2025-05-18.
//

#ifndef MOBILEGL_DILIGENT_EGL_IMPL_H
#define MOBILEGL_DILIGENT_EGL_IMPL_H

#include "../../../../Includes.h"

#if BACKEND_TYPE == BACKEND_DILIGENT

namespace MG_Diligent {
    extern Diligent::IRenderDevice*      g_pDevice;
    extern Diligent::IDeviceContext*     g_pContext;
    extern Diligent::ISwapChain*         g_pSwapChain;
    extern MG_Global::unordered_map<GLuint, Diligent::IBuffer*>         g_BufferMap;
    extern MG_Global::unordered_map<GLuint, Diligent::ITexture*>        g_TextureMap;
    extern MG_Global::unordered_map<GLuint, Diligent::ITextureView*>    g_TextureViewMap;
    
    struct GLFramebufferInfo
    {
        Diligent::IFramebuffer*                     pFramebuffer   = nullptr;
        Diligent::IRenderPass*                      pRenderPass   = nullptr;
        std::vector<Diligent::ITextureView*>        ColorRTVs;
        Diligent::ITextureView*                     pDepthStencilRTV = nullptr;
        Diligent::TEXTURE_FORMAT                    DepthStencilFormat = Diligent::TEX_FORMAT_UNKNOWN;
        bool                                        HasDepthStencil = false;
        std::vector<Diligent::OptimizedClearValue>  ClearValues;
        // ...
    };
    extern MG_Global::unordered_map<GLuint, GLFramebufferInfo>          g_FramebufferMap;
    extern MG_Global::unordered_map<GLuint, Diligent::IShader*>         g_ShaderMap;
    struct GLProgramInfo
    {
        GLuint id;
        std::vector<Diligent::IShader*> AttachedShaders;
        std::vector<GLuint> AttachedShadersID;
        Diligent::IPipelineState* pPipelineState = nullptr;
        Diligent::IShaderResourceBinding* pResourceBinding = nullptr;
        uint64_t psoStateHash = 0;
        bool psoDirty = true;
        std::vector<Diligent::LayoutElement> inputLayout;
        GLuint DefaultFBO = 0;
        ProgramObject programObj;

        // Uniform names, in the order of their index in uniform buffer
        std::vector<std::string> uniformBufferNames;

        Diligent::IBuffer* pDefaultUBO = nullptr;

        std::unordered_map<std::string, Diligent::SHADER_TYPE> uniformStages;
        std::unordered_map<std::string, size_t> uniformOffsets;
        // ...
    };
    extern MG_Global::unordered_map<GLuint, GLProgramInfo>              g_ProgramMap;
    
    extern MG_Global::unordered_map<GLuint, Diligent::ISampler*> g_SamplerMap;
    extern MG_Global::unordered_map<GLuint, Diligent::IBuffer*> g_UniformBufferMap;

    class PipelineStateManager {
    private:
        struct PSOKey {
            uint64_t programHash;
            uint64_t stateHash;

            bool operator==(const PSOKey &other) const {
                return programHash == other.programHash && stateHash == other.stateHash;
            }
        };

        struct PSOKeyHash {
            size_t operator()(const PSOKey &key) const {
                return std::hash<uint64_t>()(key.programHash) ^
                       std::hash<uint64_t>()(key.stateHash);
            }
        };

        std::unordered_map<PSOKey, Diligent::IPipelineState *, PSOKeyHash> psoCache;

    public:
        Diligent::IPipelineState *GetOrCreatePSO(
                GLuint program,
                GLProgramInfo &programInfo,
                CommonState &commonState,
                VertexArrayState &vaState,
                GLFramebufferInfo& fbInfo);

        void MarkPSODirty(GLuint program);

        void ReleasePSO(GLuint program);

    private:
        Diligent::FILTER_TYPE ConvertGLFilter(GLenum filter) {
            switch (filter) {
                case GL_NEAREST:
                case GL_NEAREST_MIPMAP_NEAREST:
                case GL_NEAREST_MIPMAP_LINEAR:
                    return Diligent::FILTER_TYPE_POINT;
                case GL_LINEAR:
                case GL_LINEAR_MIPMAP_NEAREST:
                case GL_LINEAR_MIPMAP_LINEAR:
                    return Diligent::FILTER_TYPE_LINEAR;
                default:
                    return Diligent::FILTER_TYPE_LINEAR;
            }
        }

        Diligent::FILTER_TYPE ConvertGLMipFilter(GLenum filter) {
            switch (filter) {
                case GL_NEAREST_MIPMAP_NEAREST:
                case GL_LINEAR_MIPMAP_NEAREST:
                    return Diligent::FILTER_TYPE_POINT;
                case GL_NEAREST_MIPMAP_LINEAR:
                case GL_LINEAR_MIPMAP_LINEAR:
                    return Diligent::FILTER_TYPE_LINEAR;
                case GL_NEAREST:
                case GL_LINEAR:
                    return Diligent::FILTER_TYPE_LINEAR;
                default:
                    return Diligent::FILTER_TYPE_LINEAR;
            }
        }

        Diligent::TEXTURE_ADDRESS_MODE ConvertGLWrapMode(GLenum wrap) {
            switch (wrap) {
                case GL_REPEAT:
                    return Diligent::TEXTURE_ADDRESS_WRAP;
                case GL_MIRRORED_REPEAT:
                    return Diligent::TEXTURE_ADDRESS_MIRROR;
                case GL_CLAMP_TO_EDGE:
                    return Diligent::TEXTURE_ADDRESS_CLAMP;
                case GL_CLAMP_TO_BORDER:
                    return Diligent::TEXTURE_ADDRESS_BORDER;
                case GL_MIRROR_CLAMP_TO_EDGE:
                    return Diligent::TEXTURE_ADDRESS_MIRROR_ONCE;
                default:
                    return Diligent::TEXTURE_ADDRESS_WRAP;
            }
        }

        Diligent::BLEND_FACTOR ConvertGLBlendFactor(GLenum factor) {
            switch (factor) {
                case GL_ZERO:
                    return Diligent::BLEND_FACTOR_ZERO;
                case GL_ONE:
                    return Diligent::BLEND_FACTOR_ONE;
                case GL_SRC_COLOR:
                    return Diligent::BLEND_FACTOR_SRC_COLOR;
                case GL_ONE_MINUS_SRC_COLOR:
                    return Diligent::BLEND_FACTOR_INV_SRC_COLOR;
                case GL_DST_COLOR:
                    return Diligent::BLEND_FACTOR_DEST_COLOR;
                case GL_ONE_MINUS_DST_COLOR:
                    return Diligent::BLEND_FACTOR_INV_DEST_COLOR;
                case GL_SRC_ALPHA:
                    return Diligent::BLEND_FACTOR_SRC_ALPHA;
                case GL_ONE_MINUS_SRC_ALPHA:
                    return Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
                case GL_DST_ALPHA:
                    return Diligent::BLEND_FACTOR_DEST_ALPHA;
                case GL_ONE_MINUS_DST_ALPHA:
                    return Diligent::BLEND_FACTOR_INV_DEST_ALPHA;
                default:
                    return Diligent::BLEND_FACTOR_ONE;
            }
        }

        Diligent::COMPARISON_FUNCTION ConvertGLDepthFunc(GLenum func) {
            switch (func) {
                case GL_LESS:
                    return Diligent::COMPARISON_FUNC_LESS;
                case GL_LEQUAL:
                    return Diligent::COMPARISON_FUNC_LESS_EQUAL;
                case GL_GREATER:
                    return Diligent::COMPARISON_FUNC_GREATER;
                case GL_GEQUAL:
                    return Diligent::COMPARISON_FUNC_GREATER_EQUAL;
                case GL_EQUAL:
                    return Diligent::COMPARISON_FUNC_EQUAL;
                case GL_NOTEQUAL:
                    return Diligent::COMPARISON_FUNC_NOT_EQUAL;
                case GL_ALWAYS:
                    return Diligent::COMPARISON_FUNC_ALWAYS;
                case GL_NEVER:
                    return Diligent::COMPARISON_FUNC_NEVER;
                default:
                    return Diligent::COMPARISON_FUNC_LESS;
            }
        }

        uint64_t CalculateStateHash(
                CommonState &commonState,
                VertexArrayState &vaState,
                GLFramebufferInfo& fbInfo);

        void ConfigurePSO(
                Diligent::GraphicsPipelineStateCreateInfo &PSOCreateInfo,
                GLProgramInfo &programInfo,
                CommonState &commonState,
                VertexArrayState &vaState,
                GLFramebufferInfo& fbInfo);

        void ConfigureResourceLayout(
                Diligent::GraphicsPipelineStateCreateInfo& PSOCreateInfo,
                GLProgramInfo& programInfo);
    };

    extern PipelineStateManager g_PSOManager;

    void BuildInputLayout(GLuint program, VertexArrayState& vaState, 
                          std::vector<Diligent::LayoutElement>& inputLayout);

    extern bool initialized;
    extern bool IsInRenderPass;
    extern GLuint g_NextResourceId;
}

namespace MG_EGL::Diligent {

    EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, NativeWindowType window, const EGLint* attrib_list);
    EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs, EGLint config_size, EGLint* num_config);
    EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext shareCtx, const EGLint* attrib_list);
    EGLBoolean eglInitialize(EGLDisplay dpy, EGLint* major, EGLint* minor);
    EGLDisplay eglGetDisplay(NativeDisplayType display);
    EGLint eglGetError();
    EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
    EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx);
    EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface);
    EGLBoolean eglTerminate(EGLDisplay dpy);
    EGLBoolean eglReleaseThread(void);
    EGLContext eglGetCurrentContext(void);
    EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
    EGLBoolean eglBindAPI(EGLenum api);
    EGLSurface eglGetCurrentSurface(EGLint readdraw);
    EGLBoolean eglQuerySurface(EGLDisplay display, EGLSurface surface, EGLint attribute, EGLint *value);
    EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval);
    EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface draw);
    EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
}

MG_EXPORT __eglMustCastToProperFunctionPointerType eglGetProcAddress(const char *procname);

#endif


#endif //MOBILEGL_DILIGENT_EGL_IMPL_H
