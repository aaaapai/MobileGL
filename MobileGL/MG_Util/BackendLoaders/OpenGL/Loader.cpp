#include "Loader.h"

#define GL_FUNC_DECL(name) name##_PTR name;
#define EGL_FUNC_DECL(name) name##_PTR name;

namespace MobileGL {
    namespace MG_External {
        namespace GLES {
            Caps::GLESCaps g_glesCaps;

            GL_FUNC_DECL(glActiveTexture)
            GL_FUNC_DECL(glAttachShader)
            GL_FUNC_DECL(glBindAttribLocation)
            GL_FUNC_DECL(glBindBuffer)
            GL_FUNC_DECL(glBindFramebuffer)
            GL_FUNC_DECL(glBindRenderbuffer)
            GL_FUNC_DECL(glBindTexture)
            GL_FUNC_DECL(glBlendColor)
            GL_FUNC_DECL(glBlendEquation)
            GL_FUNC_DECL(glBlendEquationSeparate)
            GL_FUNC_DECL(glBlendFunc)
            GL_FUNC_DECL(glBlendFuncSeparate)
            GL_FUNC_DECL(glBufferData)
            GL_FUNC_DECL(glBufferSubData)
            GL_FUNC_DECL(glCheckFramebufferStatus)
            GL_FUNC_DECL(glClear)
            GL_FUNC_DECL(glClearColor)
            GL_FUNC_DECL(glClearDepthf)
            GL_FUNC_DECL(glClearStencil)
            GL_FUNC_DECL(glColorMask)
            GL_FUNC_DECL(glCompileShader)
            GL_FUNC_DECL(glCompressedTexImage2D)
            GL_FUNC_DECL(glCompressedTexSubImage2D)
            GL_FUNC_DECL(glCopyTexImage2D)
            GL_FUNC_DECL(glCopyTexSubImage2D)
            GL_FUNC_DECL(glCreateProgram)
            GL_FUNC_DECL(glCreateShader)
            GL_FUNC_DECL(glCullFace)
            GL_FUNC_DECL(glDeleteBuffers)
            GL_FUNC_DECL(glDeleteFramebuffers)
            GL_FUNC_DECL(glDeleteProgram)
            GL_FUNC_DECL(glDeleteRenderbuffers)
            GL_FUNC_DECL(glDeleteShader)
            GL_FUNC_DECL(glDeleteTextures)
            GL_FUNC_DECL(glDepthFunc)
            GL_FUNC_DECL(glDepthMask)
            GL_FUNC_DECL(glDepthRangef)
            GL_FUNC_DECL(glDetachShader)
            GL_FUNC_DECL(glDisable)
            GL_FUNC_DECL(glDisableVertexAttribArray)
            GL_FUNC_DECL(glDrawArrays)
            GL_FUNC_DECL(glDrawElements)
            GL_FUNC_DECL(glEnable)
            GL_FUNC_DECL(glEnableVertexAttribArray)
            GL_FUNC_DECL(glFinish)
            GL_FUNC_DECL(glFlush)
            GL_FUNC_DECL(glFramebufferRenderbuffer)
            GL_FUNC_DECL(glFramebufferTexture2D)
            GL_FUNC_DECL(glFrontFace)
            GL_FUNC_DECL(glGenBuffers)
            GL_FUNC_DECL(glGenerateMipmap)
            GL_FUNC_DECL(glGenFramebuffers)
            GL_FUNC_DECL(glGenRenderbuffers)
            GL_FUNC_DECL(glGenTextures)
            GL_FUNC_DECL(glGetActiveAttrib)
            GL_FUNC_DECL(glGetActiveUniform)
            GL_FUNC_DECL(glGetAttachedShaders)
            GL_FUNC_DECL(glGetAttribLocation)
            GL_FUNC_DECL(glGetBooleanv)
            GL_FUNC_DECL(glGetBufferParameteriv)
            GL_FUNC_DECL(glGetError)
            GL_FUNC_DECL(glGetString)
            GL_FUNC_DECL(glGetStringi)
            GL_FUNC_DECL(glGetFloatv)
            GL_FUNC_DECL(glGetFramebufferAttachmentParameteriv)
            GL_FUNC_DECL(glGetIntegerv)
            GL_FUNC_DECL(glGetProgramiv)
            GL_FUNC_DECL(glGetProgramInfoLog)
            GL_FUNC_DECL(glGetRenderbufferParameteriv)
            GL_FUNC_DECL(glGetShaderiv)
            GL_FUNC_DECL(glGetShaderInfoLog)
            GL_FUNC_DECL(glGetShaderPrecisionFormat)
            GL_FUNC_DECL(glGetShaderSource)
            GL_FUNC_DECL(glGetTexParameterfv)
            GL_FUNC_DECL(glGetTexParameteriv)
            GL_FUNC_DECL(glGetUniformfv)
            GL_FUNC_DECL(glGetUniformiv)
            GL_FUNC_DECL(glGetUniformLocation)
            GL_FUNC_DECL(glGetVertexAttribfv)
            GL_FUNC_DECL(glGetVertexAttribiv)
            GL_FUNC_DECL(glGetVertexAttribPointerv)
            GL_FUNC_DECL(glHint)
            GL_FUNC_DECL(glIsBuffer)
            GL_FUNC_DECL(glIsEnabled)
            GL_FUNC_DECL(glIsFramebuffer)
            GL_FUNC_DECL(glIsProgram)
            GL_FUNC_DECL(glIsRenderbuffer)
            GL_FUNC_DECL(glIsShader)
            GL_FUNC_DECL(glIsTexture)
            GL_FUNC_DECL(glLineWidth)
            GL_FUNC_DECL(glLinkProgram)
            GL_FUNC_DECL(glPixelStorei)
            GL_FUNC_DECL(glPolygonOffset)
            GL_FUNC_DECL(glReadPixels)
            GL_FUNC_DECL(glReleaseShaderCompiler)
            GL_FUNC_DECL(glRenderbufferStorage)
            GL_FUNC_DECL(glSampleCoverage)
            GL_FUNC_DECL(glScissor)
            GL_FUNC_DECL(glShaderBinary)
            GL_FUNC_DECL(glShaderSource)
            GL_FUNC_DECL(glStencilFunc)
            GL_FUNC_DECL(glStencilFuncSeparate)
            GL_FUNC_DECL(glStencilMask)
            GL_FUNC_DECL(glStencilMaskSeparate)
            GL_FUNC_DECL(glStencilOp)
            GL_FUNC_DECL(glStencilOpSeparate)
            GL_FUNC_DECL(glTexImage2D)
            GL_FUNC_DECL(glTexParameterf)
            GL_FUNC_DECL(glTexParameterfv)
            GL_FUNC_DECL(glTexParameteri)
            GL_FUNC_DECL(glTexParameteriv)
            GL_FUNC_DECL(glTexSubImage2D)
            GL_FUNC_DECL(glUniform1f)
            GL_FUNC_DECL(glUniform1fv)
            GL_FUNC_DECL(glUniform1i)
            GL_FUNC_DECL(glUniform1iv)
            GL_FUNC_DECL(glUniform2f)
            GL_FUNC_DECL(glUniform2fv)
            GL_FUNC_DECL(glUniform2i)
            GL_FUNC_DECL(glUniform2iv)
            GL_FUNC_DECL(glUniform3f)
            GL_FUNC_DECL(glUniform3fv)
            GL_FUNC_DECL(glUniform3i)
            GL_FUNC_DECL(glUniform3iv)
            GL_FUNC_DECL(glUniform4f)
            GL_FUNC_DECL(glUniform4fv)
            GL_FUNC_DECL(glUniform4i)
            GL_FUNC_DECL(glUniform4iv)
            GL_FUNC_DECL(glUniformMatrix2fv)
            GL_FUNC_DECL(glUniformMatrix3fv)
            GL_FUNC_DECL(glUniformMatrix4fv)
            GL_FUNC_DECL(glUseProgram)
            GL_FUNC_DECL(glValidateProgram)
            GL_FUNC_DECL(glVertexAttrib1f)
            GL_FUNC_DECL(glVertexAttrib1fv)
            GL_FUNC_DECL(glVertexAttrib2f)
            GL_FUNC_DECL(glVertexAttrib2fv)
            GL_FUNC_DECL(glVertexAttrib3f)
            GL_FUNC_DECL(glVertexAttrib3fv)
            GL_FUNC_DECL(glVertexAttrib4f)
            GL_FUNC_DECL(glVertexAttrib4fv)
            GL_FUNC_DECL(glVertexAttribPointer)
            GL_FUNC_DECL(glViewport)
            GL_FUNC_DECL(glReadBuffer)
            GL_FUNC_DECL(glDrawRangeElements)
            GL_FUNC_DECL(glTexImage3D)
            GL_FUNC_DECL(glTexSubImage3D)
            GL_FUNC_DECL(glCopyTexSubImage3D)
            GL_FUNC_DECL(glCompressedTexImage3D)
            GL_FUNC_DECL(glCompressedTexSubImage3D)
            GL_FUNC_DECL(glGenQueries)
            GL_FUNC_DECL(glDeleteQueries)
            GL_FUNC_DECL(glIsQuery)
            GL_FUNC_DECL(glBeginQuery)
            GL_FUNC_DECL(glEndQuery)
            GL_FUNC_DECL(glGetQueryiv)
            GL_FUNC_DECL(glGetQueryObjectuiv)
            GL_FUNC_DECL(glUnmapBuffer)
            GL_FUNC_DECL(glGetBufferPointerv)
            GL_FUNC_DECL(glDrawBuffers)
            GL_FUNC_DECL(glUniformMatrix2x3fv)
            GL_FUNC_DECL(glUniformMatrix3x2fv)
            GL_FUNC_DECL(glUniformMatrix2x4fv)
            GL_FUNC_DECL(glUniformMatrix4x2fv)
            GL_FUNC_DECL(glUniformMatrix3x4fv)
            GL_FUNC_DECL(glUniformMatrix4x3fv)
            GL_FUNC_DECL(glBlitFramebuffer)
            GL_FUNC_DECL(glRenderbufferStorageMultisample)
            GL_FUNC_DECL(glFramebufferTextureLayer)
            GL_FUNC_DECL(glFlushMappedBufferRange)
            GL_FUNC_DECL(glBindVertexArray)
            GL_FUNC_DECL(glDeleteVertexArrays)
            GL_FUNC_DECL(glGenVertexArrays)
            GL_FUNC_DECL(glIsVertexArray)
            GL_FUNC_DECL(glGetIntegeri_v)
            GL_FUNC_DECL(glBeginTransformFeedback)
            GL_FUNC_DECL(glEndTransformFeedback)
            GL_FUNC_DECL(glBindBufferRange)
            GL_FUNC_DECL(glBindBufferBase)
            GL_FUNC_DECL(glTransformFeedbackVaryings)
            GL_FUNC_DECL(glGetTransformFeedbackVarying)
            GL_FUNC_DECL(glVertexAttribIPointer)
            GL_FUNC_DECL(glGetVertexAttribIiv)
            GL_FUNC_DECL(glGetVertexAttribIuiv)
            GL_FUNC_DECL(glVertexAttribI4i)
            GL_FUNC_DECL(glVertexAttribI4ui)
            GL_FUNC_DECL(glVertexAttribI4iv)
            GL_FUNC_DECL(glVertexAttribI4uiv)
            GL_FUNC_DECL(glGetUniformuiv)
            GL_FUNC_DECL(glGetFragDataLocation)
            GL_FUNC_DECL(glUniform1ui)
            GL_FUNC_DECL(glUniform2ui)
            GL_FUNC_DECL(glUniform3ui)
            GL_FUNC_DECL(glUniform4ui)
            GL_FUNC_DECL(glUniform1uiv)
            GL_FUNC_DECL(glUniform2uiv)
            GL_FUNC_DECL(glUniform3uiv)
            GL_FUNC_DECL(glUniform4uiv)
            GL_FUNC_DECL(glClearBufferiv)
            GL_FUNC_DECL(glClearBufferuiv)
            GL_FUNC_DECL(glClearBufferfv)
            GL_FUNC_DECL(glClearBufferfi)
            GL_FUNC_DECL(glCopyBufferSubData)
            GL_FUNC_DECL(glGetUniformIndices)
            GL_FUNC_DECL(glGetActiveUniformsiv)
            GL_FUNC_DECL(glGetUniformBlockIndex)
            GL_FUNC_DECL(glGetActiveUniformBlockiv)
            GL_FUNC_DECL(glGetActiveUniformBlockName)
            GL_FUNC_DECL(glUniformBlockBinding)
            GL_FUNC_DECL(glDrawArraysInstanced)
            GL_FUNC_DECL(glDrawElementsInstanced)
            GL_FUNC_DECL(glFenceSync)
            GL_FUNC_DECL(glIsSync)
            GL_FUNC_DECL(glDeleteSync)
            GL_FUNC_DECL(glClientWaitSync)
            GL_FUNC_DECL(glWaitSync)
            GL_FUNC_DECL(glGetInteger64v)
            GL_FUNC_DECL(glGetSynciv)
            GL_FUNC_DECL(glGetInteger64i_v)
            GL_FUNC_DECL(glGetBufferParameteri64v)
            GL_FUNC_DECL(glGenSamplers)
            GL_FUNC_DECL(glDeleteSamplers)
            GL_FUNC_DECL(glIsSampler)
            GL_FUNC_DECL(glBindSampler)
            GL_FUNC_DECL(glSamplerParameteri)
            GL_FUNC_DECL(glSamplerParameteriv)
            GL_FUNC_DECL(glSamplerParameterf)
            GL_FUNC_DECL(glSamplerParameterfv)
            GL_FUNC_DECL(glGetSamplerParameteriv)
            GL_FUNC_DECL(glGetSamplerParameterfv)
            GL_FUNC_DECL(glVertexAttribDivisor)
            GL_FUNC_DECL(glBindTransformFeedback)
            GL_FUNC_DECL(glDeleteTransformFeedbacks)
            GL_FUNC_DECL(glGenTransformFeedbacks)
            GL_FUNC_DECL(glIsTransformFeedback)
            GL_FUNC_DECL(glPauseTransformFeedback)
            GL_FUNC_DECL(glResumeTransformFeedback)
            GL_FUNC_DECL(glGetProgramBinary)
            GL_FUNC_DECL(glProgramBinary)
            GL_FUNC_DECL(glProgramParameteri)
            GL_FUNC_DECL(glInvalidateFramebuffer)
            GL_FUNC_DECL(glInvalidateSubFramebuffer)
            GL_FUNC_DECL(glTexStorage2D)
            GL_FUNC_DECL(glTexStorage3D)
            GL_FUNC_DECL(glGetInternalformativ)
            GL_FUNC_DECL(glDispatchCompute)
            GL_FUNC_DECL(glDispatchComputeIndirect)
            GL_FUNC_DECL(glDrawArraysIndirect)
            GL_FUNC_DECL(glDrawElementsIndirect)
            GL_FUNC_DECL(glFramebufferParameteri)
            GL_FUNC_DECL(glGetFramebufferParameteriv)
            GL_FUNC_DECL(glGetProgramInterfaceiv)
            GL_FUNC_DECL(glGetProgramResourceIndex)
            GL_FUNC_DECL(glGetProgramResourceName)
            GL_FUNC_DECL(glGetProgramResourceiv)
            GL_FUNC_DECL(glGetProgramResourceLocation)
            GL_FUNC_DECL(glUseProgramStages)
            GL_FUNC_DECL(glActiveShaderProgram)
            GL_FUNC_DECL(glCreateShaderProgramv)
            GL_FUNC_DECL(glBindProgramPipeline)
            GL_FUNC_DECL(glDeleteProgramPipelines)
            GL_FUNC_DECL(glGenProgramPipelines)
            GL_FUNC_DECL(glIsProgramPipeline)
            GL_FUNC_DECL(glGetProgramPipelineiv)
            GL_FUNC_DECL(glProgramUniform1i)
            GL_FUNC_DECL(glProgramUniform2i)
            GL_FUNC_DECL(glProgramUniform3i)
            GL_FUNC_DECL(glProgramUniform4i)
            GL_FUNC_DECL(glProgramUniform1ui)
            GL_FUNC_DECL(glProgramUniform2ui)
            GL_FUNC_DECL(glProgramUniform3ui)
            GL_FUNC_DECL(glProgramUniform4ui)
            GL_FUNC_DECL(glProgramUniform1f)
            GL_FUNC_DECL(glProgramUniform2f)
            GL_FUNC_DECL(glProgramUniform3f)
            GL_FUNC_DECL(glProgramUniform4f)
            GL_FUNC_DECL(glProgramUniform1iv)
            GL_FUNC_DECL(glProgramUniform2iv)
            GL_FUNC_DECL(glProgramUniform3iv)
            GL_FUNC_DECL(glProgramUniform4iv)
            GL_FUNC_DECL(glProgramUniform1uiv)
            GL_FUNC_DECL(glProgramUniform2uiv)
            GL_FUNC_DECL(glProgramUniform3uiv)
            GL_FUNC_DECL(glProgramUniform4uiv)
            GL_FUNC_DECL(glProgramUniform1fv)
            GL_FUNC_DECL(glProgramUniform2fv)
            GL_FUNC_DECL(glProgramUniform3fv)
            GL_FUNC_DECL(glProgramUniform4fv)
            GL_FUNC_DECL(glProgramUniformMatrix2fv)
            GL_FUNC_DECL(glProgramUniformMatrix3fv)
            GL_FUNC_DECL(glProgramUniformMatrix4fv)
            GL_FUNC_DECL(glProgramUniformMatrix2x3fv)
            GL_FUNC_DECL(glProgramUniformMatrix3x2fv)
            GL_FUNC_DECL(glProgramUniformMatrix2x4fv)
            GL_FUNC_DECL(glProgramUniformMatrix4x2fv)
            GL_FUNC_DECL(glProgramUniformMatrix3x4fv)
            GL_FUNC_DECL(glProgramUniformMatrix4x3fv)
            GL_FUNC_DECL(glValidateProgramPipeline)
            GL_FUNC_DECL(glGetProgramPipelineInfoLog)
            GL_FUNC_DECL(glBindImageTexture)
            GL_FUNC_DECL(glGetBooleani_v)
            GL_FUNC_DECL(glMemoryBarrier)
            GL_FUNC_DECL(glMemoryBarrierByRegion)
            GL_FUNC_DECL(glTexStorage2DMultisample)
            GL_FUNC_DECL(glGetMultisamplefv)
            GL_FUNC_DECL(glSampleMaski)
            GL_FUNC_DECL(glGetTexLevelParameteriv)
            GL_FUNC_DECL(glGetTexLevelParameterfv)
            GL_FUNC_DECL(glBindVertexBuffer)
            GL_FUNC_DECL(glVertexAttribFormat)
            GL_FUNC_DECL(glVertexAttribIFormat)
            GL_FUNC_DECL(glVertexAttribBinding)
            GL_FUNC_DECL(glVertexBindingDivisor)
            GL_FUNC_DECL(glBlendBarrier)
            GL_FUNC_DECL(glCopyImageSubData)
            GL_FUNC_DECL(glDebugMessageControl)
            GL_FUNC_DECL(glDebugMessageInsert)
            GL_FUNC_DECL(glDebugMessageCallback)
            GL_FUNC_DECL(glGetDebugMessageLog)
            GL_FUNC_DECL(glPushDebugGroup)
            GL_FUNC_DECL(glPopDebugGroup)
            GL_FUNC_DECL(glObjectLabel)
            GL_FUNC_DECL(glGetObjectLabel)
            GL_FUNC_DECL(glObjectPtrLabel)
            GL_FUNC_DECL(glGetObjectPtrLabel)
            GL_FUNC_DECL(glGetPointerv)
            GL_FUNC_DECL(glEnablei)
            GL_FUNC_DECL(glDisablei)
            GL_FUNC_DECL(glBlendEquationi)
            GL_FUNC_DECL(glBlendEquationSeparatei)
            GL_FUNC_DECL(glBlendFunci)
            GL_FUNC_DECL(glBlendFuncSeparatei)
            GL_FUNC_DECL(glColorMaski)
            GL_FUNC_DECL(glIsEnabledi)
            GL_FUNC_DECL(glDrawElementsBaseVertex)
            GL_FUNC_DECL(glDrawRangeElementsBaseVertex)
            GL_FUNC_DECL(glDrawElementsInstancedBaseVertex)
            GL_FUNC_DECL(glFramebufferTexture)
            GL_FUNC_DECL(glPrimitiveBoundingBox)
            GL_FUNC_DECL(glGetGraphicsResetStatus)
            GL_FUNC_DECL(glReadnPixels)
            GL_FUNC_DECL(glGetnUniformfv)
            GL_FUNC_DECL(glGetnUniformiv)
            GL_FUNC_DECL(glGetnUniformuiv)
            GL_FUNC_DECL(glMinSampleShading)
            GL_FUNC_DECL(glPatchParameteri)
            GL_FUNC_DECL(glTexParameterIiv)
            GL_FUNC_DECL(glTexParameterIuiv)
            GL_FUNC_DECL(glGetTexParameterIiv)
            GL_FUNC_DECL(glGetTexParameterIuiv)
            GL_FUNC_DECL(glSamplerParameterIiv)
            GL_FUNC_DECL(glSamplerParameterIuiv)
            GL_FUNC_DECL(glGetSamplerParameterIiv)
            GL_FUNC_DECL(glGetSamplerParameterIuiv)
            GL_FUNC_DECL(glTexBuffer)
            GL_FUNC_DECL(glTexBufferRange)
            GL_FUNC_DECL(glTexStorage3DMultisample)
            GL_FUNC_DECL(glMapBufferRange)
            GL_FUNC_DECL(glBufferStorageEXT)
            GL_FUNC_DECL(glGetQueryObjectivEXT)
            GL_FUNC_DECL(glGetQueryObjecti64vEXT)
            GL_FUNC_DECL(glBindFragDataLocationEXT)
            GL_FUNC_DECL(glMapBufferOES)

            GL_FUNC_DECL(glMultiDrawArraysIndirectEXT)
            GL_FUNC_DECL(glMultiDrawElementsIndirectEXT)
            GL_FUNC_DECL(glMultiDrawElementsBaseVertexEXT)

            GL_FUNC_DECL(glBruh)
        } // namespace GLES

        namespace EGL {
            EGL_FUNC_DECL(eglGetProcAddress)
            EGL_FUNC_DECL(eglBindAPI)
            EGL_FUNC_DECL(eglBindTexImage)
            EGL_FUNC_DECL(eglChooseConfig)
            EGL_FUNC_DECL(eglCopyBuffers)
            EGL_FUNC_DECL(eglCreateContext)
            EGL_FUNC_DECL(eglCreatePbufferFromClientBuffer)
            EGL_FUNC_DECL(eglCreatePbufferSurface)
            EGL_FUNC_DECL(eglCreatePixmapSurface)
            EGL_FUNC_DECL(eglCreatePlatformWindowSurface)
            EGL_FUNC_DECL(eglCreateWindowSurface)
            EGL_FUNC_DECL(eglDestroyContext)
            EGL_FUNC_DECL(eglDestroySurface)
            EGL_FUNC_DECL(eglGetConfigAttrib)
            EGL_FUNC_DECL(eglGetConfigs)
            EGL_FUNC_DECL(eglGetCurrentContext)
            EGL_FUNC_DECL(eglGetCurrentDisplay)
            EGL_FUNC_DECL(eglGetCurrentSurface)
            EGL_FUNC_DECL(eglGetDisplay)
            EGL_FUNC_DECL(eglGetPlatformDisplay)
            EGL_FUNC_DECL(eglGetError)
            EGL_FUNC_DECL(eglInitialize)
            EGL_FUNC_DECL(eglMakeCurrent)
            EGL_FUNC_DECL(eglQueryAPI)
            EGL_FUNC_DECL(eglQueryContext)
            EGL_FUNC_DECL(eglQueryString)
            EGL_FUNC_DECL(eglQuerySurface)
            EGL_FUNC_DECL(eglReleaseTexImage)
            EGL_FUNC_DECL(eglReleaseThread)
            EGL_FUNC_DECL(eglSurfaceAttrib)
            EGL_FUNC_DECL(eglSwapBuffers)
            EGL_FUNC_DECL(eglSwapBuffersWithDamageEXT)
            EGL_FUNC_DECL(eglSwapInterval)
            EGL_FUNC_DECL(eglTerminate)
            EGL_FUNC_DECL(eglUnlockSurfaceKHR)
            EGL_FUNC_DECL(eglWaitClient)
            EGL_FUNC_DECL(eglWaitGL)
            EGL_FUNC_DECL(eglWaitNative)
        } // namespace EGL
    } // namespace MG_External

#undef GL_FUNC_DECL
#undef EGL_FUNC_DECL

    namespace MG_Util {
        namespace BackendLoader::GLES {
            void Init() {
                LoadLibs();
                InitEGL();
                InitGLES();
                DestroyTempEGLCtx();
            }

            void *libEGL = nullptr;

            static const char* LibPathPrefixes[] = {"", "/opt/vc/lib/", "/usr/local/lib/", "/usr/lib/", nullptr};
            static const char* LibExts[] = {"so", "so.1", "so.2", "dylib", "dll", nullptr};
            static const char* EGLLibs[] = {"libEGL", "libEGL_angle", nullptr};
            static const char* EGLANGLELibs[] = {"libEGL_angle", nullptr};


            void* OpenLib(const char** names, const char* override) {
#if !defined(__WIN32) && !defined(_WIN32) && !defined(__APPLE__)
                void* lib = nullptr;

                char path_name[PATH_MAX + 1];
                int flags = RTLD_LOCAL | RTLD_NOW;
                if (override) {
                    if ((lib = dlopen(override, flags))) {
                        strncpy(path_name, override, PATH_MAX);
                        return lib;
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
#else
                return nullptr;
#endif
            }

            void LoadLibs() {
                // 只加载EGL库，GL函数将通过EGL获取
                if (std::getenv("LIBGL_ANGLE")) {
                    libEGL = OpenLib(EGLANGLELibs, nullptr);
                } else {
                    libEGL = OpenLib(EGLLibs, nullptr);
                }
            }

            void* ProcAddress(const char* name) {
                // 优先使用EGL的GetProcAddress
                if (MG_External::EGL::eglGetProcAddress) {
                    void* func = (void*)MG_External::EGL::eglGetProcAddress(name);
                    if (func) return func;
                }
                
                // 回退到dlsym
                if (libEGL) {
                    return dlsym(libEGL, name);
                }
                return nullptr;
            }

            void InitGLESCapabilities() {
                MG_External::GLES::glGetIntegerv(GL_MAJOR_VERSION, &MG_External::GLES::g_glesCaps.version.Major);
                MG_External::GLES::glGetIntegerv(GL_MINOR_VERSION, &MG_External::GLES::g_glesCaps.version.Minor);

                GLint extCount = 0;
                MG_External::GLES::glGetIntegerv(GL_NUM_EXTENSIONS, &extCount);
                for (GLint i = 0; i < extCount; ++i) {
                    const char* extension = (const char*)MG_External::GLES::glGetStringi(GL_EXTENSIONS, i);
                    if (extension) {
                        if (std::strcmp(extension, "GL_EXT_buffer_storage") == 0) {
                            MG_External::GLES::g_glesCaps.hasPersistentMapping = true;
                        }
                    }
                }
            }

            void InitGLES() {
                INIT_GLES_FUNC(glActiveTexture)
                INIT_GLES_FUNC(glAttachShader)
                INIT_GLES_FUNC(glBindAttribLocation)
                INIT_GLES_FUNC(glBindBuffer)
                INIT_GLES_FUNC(glBindFramebuffer)
                INIT_GLES_FUNC(glBindRenderbuffer)
                INIT_GLES_FUNC(glBindTexture)
                INIT_GLES_FUNC(glBlendColor)
                INIT_GLES_FUNC(glBlendEquation)
                INIT_GLES_FUNC(glBlendEquationSeparate)
                INIT_GLES_FUNC(glBlendFunc)
                INIT_GLES_FUNC(glBlendFuncSeparate)
                INIT_GLES_FUNC(glBufferData)
                INIT_GLES_FUNC(glBufferSubData)
                INIT_GLES_FUNC(glCheckFramebufferStatus)
                INIT_GLES_FUNC(glClear)
                INIT_GLES_FUNC(glClearColor)
                INIT_GLES_FUNC(glClearDepthf)
                INIT_GLES_FUNC(glClearStencil)
                INIT_GLES_FUNC(glColorMask)
                INIT_GLES_FUNC(glCompileShader)
                INIT_GLES_FUNC(glCompressedTexImage2D)
                INIT_GLES_FUNC(glCompressedTexSubImage2D)
                //    INIT_GLES_FUNC(glCopyTexImage1D)
                INIT_GLES_FUNC(glCopyTexImage2D)
                INIT_GLES_FUNC(glCopyTexSubImage2D)
                INIT_GLES_FUNC(glCreateProgram)
                INIT_GLES_FUNC(glCreateShader)
                INIT_GLES_FUNC(glCullFace)
                INIT_GLES_FUNC(glDeleteBuffers)
                INIT_GLES_FUNC(glDeleteFramebuffers)
                INIT_GLES_FUNC(glDeleteProgram)
                INIT_GLES_FUNC(glDeleteRenderbuffers)
                INIT_GLES_FUNC(glDeleteShader)
                INIT_GLES_FUNC(glDeleteTextures)
                INIT_GLES_FUNC(glDepthFunc)
                INIT_GLES_FUNC(glDepthMask)
                INIT_GLES_FUNC(glDepthRangef)
                INIT_GLES_FUNC(glDetachShader)
                INIT_GLES_FUNC(glDisable)
                INIT_GLES_FUNC(glDisableVertexAttribArray)
                INIT_GLES_FUNC(glDrawArrays)
                INIT_GLES_FUNC(glDrawElements)
                INIT_GLES_FUNC(glEnable)
                INIT_GLES_FUNC(glEnableVertexAttribArray)
                INIT_GLES_FUNC(glFinish)
                INIT_GLES_FUNC(glFlush)
                INIT_GLES_FUNC(glFramebufferRenderbuffer)
                INIT_GLES_FUNC(glFramebufferTexture2D)
                INIT_GLES_FUNC(glFrontFace)
                INIT_GLES_FUNC(glGenBuffers)
                INIT_GLES_FUNC(glGenerateMipmap)
                INIT_GLES_FUNC(glGenFramebuffers)
                INIT_GLES_FUNC(glGenRenderbuffers)
                INIT_GLES_FUNC(glGenTextures)
                INIT_GLES_FUNC(glGetActiveAttrib)
                INIT_GLES_FUNC(glGetActiveUniform)
                INIT_GLES_FUNC(glGetAttachedShaders)
                INIT_GLES_FUNC(glGetAttribLocation)
                INIT_GLES_FUNC(glGetBooleanv)
                INIT_GLES_FUNC(glGetBufferParameteriv)
                INIT_GLES_FUNC(glGetError)
                INIT_GLES_FUNC(glGetString)
                INIT_GLES_FUNC(glGetStringi)
                INIT_GLES_FUNC(glGetFloatv)
                INIT_GLES_FUNC(glGetFramebufferAttachmentParameteriv)
                INIT_GLES_FUNC(glGetIntegerv)
                INIT_GLES_FUNC(glGetProgramiv)
                INIT_GLES_FUNC(glGetProgramInfoLog)
                INIT_GLES_FUNC(glGetRenderbufferParameteriv)
                INIT_GLES_FUNC(glGetShaderiv)
                INIT_GLES_FUNC(glGetShaderInfoLog)
                INIT_GLES_FUNC(glGetShaderPrecisionFormat)
                INIT_GLES_FUNC(glGetShaderSource)
                INIT_GLES_FUNC(glGetTexParameterfv)
                INIT_GLES_FUNC(glGetTexParameteriv)
                INIT_GLES_FUNC(glGetUniformfv)
                INIT_GLES_FUNC(glGetUniformiv)
                INIT_GLES_FUNC(glGetUniformLocation)
                INIT_GLES_FUNC(glGetVertexAttribfv)
                INIT_GLES_FUNC(glGetVertexAttribiv)
                INIT_GLES_FUNC(glGetVertexAttribPointerv)
                INIT_GLES_FUNC(glHint)
                INIT_GLES_FUNC(glIsBuffer)
                INIT_GLES_FUNC(glIsEnabled)
                INIT_GLES_FUNC(glIsFramebuffer)
                INIT_GLES_FUNC(glIsProgram)
                INIT_GLES_FUNC(glIsRenderbuffer)
                INIT_GLES_FUNC(glIsShader)
                INIT_GLES_FUNC(glIsTexture)
                INIT_GLES_FUNC(glLineWidth)
                INIT_GLES_FUNC(glLinkProgram)
                INIT_GLES_FUNC(glPixelStorei)
                INIT_GLES_FUNC(glPolygonOffset)
                INIT_GLES_FUNC(glReadPixels)
                INIT_GLES_FUNC(glReleaseShaderCompiler)
                INIT_GLES_FUNC(glRenderbufferStorage)
                INIT_GLES_FUNC(glSampleCoverage)
                INIT_GLES_FUNC(glScissor)
                INIT_GLES_FUNC(glShaderBinary)
                INIT_GLES_FUNC(glShaderSource)
                INIT_GLES_FUNC(glStencilFunc)
                INIT_GLES_FUNC(glStencilFuncSeparate)
                INIT_GLES_FUNC(glStencilMask)
                INIT_GLES_FUNC(glStencilMaskSeparate)
                INIT_GLES_FUNC(glStencilOp)
                INIT_GLES_FUNC(glStencilOpSeparate)
                //    INIT_GLES_FUNC(glTexImage1D)
                INIT_GLES_FUNC(glTexImage2D)
                //    INIT_GLES_FUNC(glTexStorage1D)
                INIT_GLES_FUNC(glTexParameterf)
                INIT_GLES_FUNC(glTexParameterfv)
                INIT_GLES_FUNC(glTexParameteri)
                INIT_GLES_FUNC(glTexParameteriv)
                INIT_GLES_FUNC(glTexSubImage2D)
                INIT_GLES_FUNC(glUniform1f)
                INIT_GLES_FUNC(glUniform1fv)
                INIT_GLES_FUNC(glUniform1i)
                INIT_GLES_FUNC(glUniform1iv)
                INIT_GLES_FUNC(glUniform2f)
                INIT_GLES_FUNC(glUniform2fv)
                INIT_GLES_FUNC(glUniform2i)
                INIT_GLES_FUNC(glUniform2iv)
                INIT_GLES_FUNC(glUniform3f)
                INIT_GLES_FUNC(glUniform3fv)
                INIT_GLES_FUNC(glUniform3i)
                INIT_GLES_FUNC(glUniform3iv)
                INIT_GLES_FUNC(glUniform4f)
                INIT_GLES_FUNC(glUniform4fv)
                INIT_GLES_FUNC(glUniform4i)
                INIT_GLES_FUNC(glUniform4iv)
                INIT_GLES_FUNC(glUniformMatrix2fv)
                INIT_GLES_FUNC(glUniformMatrix3fv)
                INIT_GLES_FUNC(glUniformMatrix4fv)
                INIT_GLES_FUNC(glUseProgram)
                INIT_GLES_FUNC(glValidateProgram)
                INIT_GLES_FUNC(glVertexAttrib1f)
                INIT_GLES_FUNC(glVertexAttrib1fv)
                INIT_GLES_FUNC(glVertexAttrib2f)
                INIT_GLES_FUNC(glVertexAttrib2fv)
                INIT_GLES_FUNC(glVertexAttrib3f)
                INIT_GLES_FUNC(glVertexAttrib3fv)
                INIT_GLES_FUNC(glVertexAttrib4f)
                INIT_GLES_FUNC(glVertexAttrib4fv)
                INIT_GLES_FUNC(glVertexAttribPointer)
                INIT_GLES_FUNC(glViewport)
                INIT_GLES_FUNC(glReadBuffer)
                INIT_GLES_FUNC(glDrawRangeElements)
                INIT_GLES_FUNC(glTexImage3D)
                INIT_GLES_FUNC(glTexSubImage3D)
                INIT_GLES_FUNC(glCopyTexSubImage3D)
                INIT_GLES_FUNC(glCompressedTexImage3D)
                INIT_GLES_FUNC(glCompressedTexSubImage3D)
                INIT_GLES_FUNC(glGenQueries)
                INIT_GLES_FUNC(glDeleteQueries)
                INIT_GLES_FUNC(glIsQuery)
                INIT_GLES_FUNC(glBeginQuery)
                INIT_GLES_FUNC(glEndQuery)
                INIT_GLES_FUNC(glGetQueryiv)
                INIT_GLES_FUNC(glGetQueryObjectuiv)
                INIT_GLES_FUNC(glUnmapBuffer)
                INIT_GLES_FUNC(glGetBufferPointerv)
                INIT_GLES_FUNC(glDrawBuffers)
                INIT_GLES_FUNC(glUniformMatrix2x3fv)
                INIT_GLES_FUNC(glUniformMatrix3x2fv)
                INIT_GLES_FUNC(glUniformMatrix2x4fv)
                INIT_GLES_FUNC(glUniformMatrix4x2fv)
                INIT_GLES_FUNC(glUniformMatrix3x4fv)
                INIT_GLES_FUNC(glUniformMatrix4x3fv)
                INIT_GLES_FUNC(glBlitFramebuffer)
                INIT_GLES_FUNC(glRenderbufferStorageMultisample)
                INIT_GLES_FUNC(glFramebufferTextureLayer)
                INIT_GLES_FUNC(glFlushMappedBufferRange)
                INIT_GLES_FUNC(glBindVertexArray)
                INIT_GLES_FUNC(glDeleteVertexArrays)
                INIT_GLES_FUNC(glGenVertexArrays)
                INIT_GLES_FUNC(glIsVertexArray)
                INIT_GLES_FUNC(glGetIntegeri_v)
                INIT_GLES_FUNC(glBeginTransformFeedback)
                INIT_GLES_FUNC(glEndTransformFeedback)
                INIT_GLES_FUNC(glBindBufferRange)
                INIT_GLES_FUNC(glBindBufferBase)
                INIT_GLES_FUNC(glTransformFeedbackVaryings)
                INIT_GLES_FUNC(glGetTransformFeedbackVarying)
                INIT_GLES_FUNC(glVertexAttribIPointer)
                INIT_GLES_FUNC(glGetVertexAttribIiv)
                INIT_GLES_FUNC(glGetVertexAttribIuiv)
                INIT_GLES_FUNC(glVertexAttribI4i)
                INIT_GLES_FUNC(glVertexAttribI4ui)
                INIT_GLES_FUNC(glVertexAttribI4iv)
                INIT_GLES_FUNC(glVertexAttribI4uiv)
                INIT_GLES_FUNC(glGetUniformuiv)
                INIT_GLES_FUNC(glGetFragDataLocation)
                INIT_GLES_FUNC(glUniform1ui)
                INIT_GLES_FUNC(glUniform2ui)
                INIT_GLES_FUNC(glUniform3ui)
                INIT_GLES_FUNC(glUniform4ui)
                INIT_GLES_FUNC(glUniform1uiv)
                INIT_GLES_FUNC(glUniform2uiv)
                INIT_GLES_FUNC(glUniform3uiv)
                INIT_GLES_FUNC(glUniform4uiv)
                INIT_GLES_FUNC(glClearBufferiv)
                INIT_GLES_FUNC(glClearBufferuiv)
                INIT_GLES_FUNC(glClearBufferfv)
                INIT_GLES_FUNC(glClearBufferfi)
                INIT_GLES_FUNC(glCopyBufferSubData)
                INIT_GLES_FUNC(glGetUniformIndices)
                INIT_GLES_FUNC(glGetActiveUniformsiv)
                INIT_GLES_FUNC(glGetUniformBlockIndex)
                INIT_GLES_FUNC(glGetActiveUniformBlockiv)
                INIT_GLES_FUNC(glGetActiveUniformBlockName)
                INIT_GLES_FUNC(glUniformBlockBinding)
                INIT_GLES_FUNC(glDrawArraysInstanced)
                INIT_GLES_FUNC(glDrawElementsInstanced)
                INIT_GLES_FUNC(glFenceSync)
                INIT_GLES_FUNC(glIsSync)
                INIT_GLES_FUNC(glDeleteSync)
                INIT_GLES_FUNC(glClientWaitSync)
                INIT_GLES_FUNC(glWaitSync)
                INIT_GLES_FUNC(glGetInteger64v)
                INIT_GLES_FUNC(glGetSynciv)
                INIT_GLES_FUNC(glGetInteger64i_v)
                INIT_GLES_FUNC(glGetBufferParameteri64v)
                INIT_GLES_FUNC(glGenSamplers)
                INIT_GLES_FUNC(glDeleteSamplers)
                INIT_GLES_FUNC(glIsSampler)
                INIT_GLES_FUNC(glBindSampler)
                INIT_GLES_FUNC(glSamplerParameteri)
                INIT_GLES_FUNC(glSamplerParameteriv)
                INIT_GLES_FUNC(glSamplerParameterf)
                INIT_GLES_FUNC(glSamplerParameterfv)
                INIT_GLES_FUNC(glGetSamplerParameteriv)
                INIT_GLES_FUNC(glGetSamplerParameterfv)
                INIT_GLES_FUNC(glVertexAttribDivisor)
                INIT_GLES_FUNC(glBindTransformFeedback)
                INIT_GLES_FUNC(glDeleteTransformFeedbacks)
                INIT_GLES_FUNC(glGenTransformFeedbacks)
                INIT_GLES_FUNC(glIsTransformFeedback)
                INIT_GLES_FUNC(glPauseTransformFeedback)
                INIT_GLES_FUNC(glResumeTransformFeedback)
                INIT_GLES_FUNC(glGetProgramBinary)
                INIT_GLES_FUNC(glProgramBinary)
                INIT_GLES_FUNC(glProgramParameteri)
                INIT_GLES_FUNC(glInvalidateFramebuffer)
                INIT_GLES_FUNC(glInvalidateSubFramebuffer)
                INIT_GLES_FUNC(glTexStorage2D)
                INIT_GLES_FUNC(glTexStorage3D)
                INIT_GLES_FUNC(glGetInternalformativ)
                INIT_GLES_FUNC(glDispatchCompute)
                INIT_GLES_FUNC(glDispatchComputeIndirect)
                INIT_GLES_FUNC(glDrawArraysIndirect)
                INIT_GLES_FUNC(glDrawElementsIndirect)
                INIT_GLES_FUNC(glFramebufferParameteri)
                INIT_GLES_FUNC(glGetFramebufferParameteriv)
                INIT_GLES_FUNC(glGetProgramInterfaceiv)
                INIT_GLES_FUNC(glGetProgramResourceIndex)
                INIT_GLES_FUNC(glGetProgramResourceName)
                INIT_GLES_FUNC(glGetProgramResourceiv)
                INIT_GLES_FUNC(glGetProgramResourceLocation)
                INIT_GLES_FUNC(glUseProgramStages)
                INIT_GLES_FUNC(glActiveShaderProgram)
                INIT_GLES_FUNC(glCreateShaderProgramv)
                INIT_GLES_FUNC(glBindProgramPipeline)
                INIT_GLES_FUNC(glDeleteProgramPipelines)
                INIT_GLES_FUNC(glGenProgramPipelines)
                INIT_GLES_FUNC(glIsProgramPipeline)
                INIT_GLES_FUNC(glGetProgramPipelineiv)
                INIT_GLES_FUNC(glProgramUniform1i)
                INIT_GLES_FUNC(glProgramUniform2i)
                INIT_GLES_FUNC(glProgramUniform3i)
                INIT_GLES_FUNC(glProgramUniform4i)
                INIT_GLES_FUNC(glProgramUniform1ui)
                INIT_GLES_FUNC(glProgramUniform2ui)
                INIT_GLES_FUNC(glProgramUniform3ui)
                INIT_GLES_FUNC(glProgramUniform4ui)
                INIT_GLES_FUNC(glProgramUniform1f)
                INIT_GLES_FUNC(glProgramUniform2f)
                INIT_GLES_FUNC(glProgramUniform3f)
                INIT_GLES_FUNC(glProgramUniform4f)
                INIT_GLES_FUNC(glProgramUniform1iv)
                INIT_GLES_FUNC(glProgramUniform2iv)
                INIT_GLES_FUNC(glProgramUniform3iv)
                INIT_GLES_FUNC(glProgramUniform4iv)
                INIT_GLES_FUNC(glProgramUniform1uiv)
                INIT_GLES_FUNC(glProgramUniform2uiv)
                INIT_GLES_FUNC(glProgramUniform3uiv)
                INIT_GLES_FUNC(glProgramUniform4uiv)
                INIT_GLES_FUNC(glProgramUniform1fv)
                INIT_GLES_FUNC(glProgramUniform2fv)
                INIT_GLES_FUNC(glProgramUniform3fv)
                INIT_GLES_FUNC(glProgramUniform4fv)
                INIT_GLES_FUNC(glProgramUniformMatrix2fv)
                INIT_GLES_FUNC(glProgramUniformMatrix3fv)
                INIT_GLES_FUNC(glProgramUniformMatrix4fv)
                INIT_GLES_FUNC(glProgramUniformMatrix2x3fv)
                INIT_GLES_FUNC(glProgramUniformMatrix3x2fv)
                INIT_GLES_FUNC(glProgramUniformMatrix2x4fv)
                INIT_GLES_FUNC(glProgramUniformMatrix4x2fv)
                INIT_GLES_FUNC(glProgramUniformMatrix3x4fv)
                INIT_GLES_FUNC(glProgramUniformMatrix4x3fv)
                INIT_GLES_FUNC(glValidateProgramPipeline)
                INIT_GLES_FUNC(glGetProgramPipelineInfoLog)
                INIT_GLES_FUNC(glBindImageTexture)
                INIT_GLES_FUNC(glGetBooleani_v)
                INIT_GLES_FUNC(glMemoryBarrier)
                INIT_GLES_FUNC(glMemoryBarrierByRegion)
                INIT_GLES_FUNC(glTexStorage2DMultisample)
                INIT_GLES_FUNC(glGetMultisamplefv)
                INIT_GLES_FUNC(glSampleMaski)
                INIT_GLES_FUNC(glGetTexLevelParameteriv)
                INIT_GLES_FUNC(glGetTexLevelParameterfv)
                INIT_GLES_FUNC(glBindVertexBuffer)
                INIT_GLES_FUNC(glVertexAttribFormat)
                INIT_GLES_FUNC(glVertexAttribIFormat)
                INIT_GLES_FUNC(glVertexAttribBinding)
                INIT_GLES_FUNC(glVertexBindingDivisor)
                INIT_GLES_FUNC(glBlendBarrier)
                INIT_GLES_FUNC(glCopyImageSubData)
                INIT_GLES_FUNC(glDebugMessageControl)
                INIT_GLES_FUNC(glDebugMessageInsert)
                INIT_GLES_FUNC(glDebugMessageCallback)
                INIT_GLES_FUNC(glGetDebugMessageLog)
                INIT_GLES_FUNC(glPushDebugGroup)
                INIT_GLES_FUNC(glPopDebugGroup)
                INIT_GLES_FUNC(glObjectLabel)
                INIT_GLES_FUNC(glGetObjectLabel)
                INIT_GLES_FUNC(glObjectPtrLabel)
                INIT_GLES_FUNC(glGetObjectPtrLabel)
                INIT_GLES_FUNC(glGetPointerv)
                INIT_GLES_FUNC(glEnablei)
                INIT_GLES_FUNC(glDisablei)
                INIT_GLES_FUNC(glBlendEquationi)
                INIT_GLES_FUNC(glBlendEquationSeparatei)
                INIT_GLES_FUNC(glBlendFunci)
                INIT_GLES_FUNC(glBlendFuncSeparatei)
                INIT_GLES_FUNC(glColorMaski)
                INIT_GLES_FUNC(glIsEnabledi)
                INIT_GLES_FUNC(glDrawElementsBaseVertex)
                INIT_GLES_FUNC(glDrawRangeElementsBaseVertex)
                INIT_GLES_FUNC(glDrawElementsInstancedBaseVertex)
                INIT_GLES_FUNC(glFramebufferTexture)
                INIT_GLES_FUNC(glPrimitiveBoundingBox)
                INIT_GLES_FUNC(glGetGraphicsResetStatus)
                INIT_GLES_FUNC(glReadnPixels)
                INIT_GLES_FUNC(glGetnUniformfv)
                INIT_GLES_FUNC(glGetnUniformiv)
                INIT_GLES_FUNC(glGetnUniformuiv)
                INIT_GLES_FUNC(glMinSampleShading)
                INIT_GLES_FUNC(glPatchParameteri)
                INIT_GLES_FUNC(glTexParameterIiv)
                INIT_GLES_FUNC(glTexParameterIuiv)
                INIT_GLES_FUNC(glGetTexParameterIiv)
                INIT_GLES_FUNC(glGetTexParameterIuiv)
                INIT_GLES_FUNC(glSamplerParameterIiv)
                INIT_GLES_FUNC(glSamplerParameterIuiv)
                INIT_GLES_FUNC(glGetSamplerParameterIiv)
                INIT_GLES_FUNC(glGetSamplerParameterIuiv)
                INIT_GLES_FUNC(glTexBuffer)
                INIT_GLES_FUNC(glTexBufferRange)
                INIT_GLES_FUNC(glTexStorage3DMultisample)
                INIT_GLES_FUNC(glMapBufferRange)
                INIT_GLES_FUNC(glBufferStorageEXT)
                INIT_GLES_FUNC(glGetQueryObjectivEXT)
                INIT_GLES_FUNC(glGetQueryObjecti64vEXT)
                INIT_GLES_FUNC(glBindFragDataLocationEXT)
                INIT_GLES_FUNC(glMapBufferOES)
                INIT_GLES_FUNC(glMultiDrawArraysIndirectEXT)
                INIT_GLES_FUNC(glMultiDrawElementsIndirectEXT)
                INIT_GLES_FUNC(glMultiDrawElementsBaseVertexEXT)

                InitGLESCapabilities();
            }

            static EGLDisplay eglDisplay = EGL_NO_DISPLAY;
            static EGLSurface eglSurface = EGL_NO_SURFACE;
            static EGLContext eglContext = EGL_NO_CONTEXT;

#define INIT_EGL_FUNC(name)                                                                                            \
    if (libEGL != NULL) {                                                                                              \
        MG_External::EGL::name = (MG_External::EGL::name##_PTR)ProcAddress(#name);                             \
    }
            void InitEGL() {
                INIT_EGL_FUNC(eglGetProcAddress)
                INIT_EGL_FUNC(eglBindAPI)
                INIT_EGL_FUNC(eglBindTexImage)
                INIT_EGL_FUNC(eglChooseConfig)
                INIT_EGL_FUNC(eglCopyBuffers)
                INIT_EGL_FUNC(eglCreateContext)
                INIT_EGL_FUNC(eglCreatePbufferFromClientBuffer)
                INIT_EGL_FUNC(eglCreatePbufferSurface)
                INIT_EGL_FUNC(eglCreatePixmapSurface)
                INIT_EGL_FUNC(eglCreatePlatformWindowSurface)
                INIT_EGL_FUNC(eglCreateWindowSurface)
                INIT_EGL_FUNC(eglDestroyContext)
                INIT_EGL_FUNC(eglDestroySurface)
                INIT_EGL_FUNC(eglGetConfigAttrib)
                INIT_EGL_FUNC(eglGetConfigs)
                INIT_EGL_FUNC(eglGetCurrentContext)
                INIT_EGL_FUNC(eglGetCurrentDisplay)
                INIT_EGL_FUNC(eglGetCurrentSurface)
                INIT_EGL_FUNC(eglGetDisplay)
                INIT_EGL_FUNC(eglGetPlatformDisplay)
                INIT_EGL_FUNC(eglGetError)
                INIT_EGL_FUNC(eglInitialize)
                INIT_EGL_FUNC(eglMakeCurrent)
                INIT_EGL_FUNC(eglQueryAPI)
                INIT_EGL_FUNC(eglQueryContext)
                INIT_EGL_FUNC(eglQueryString)
                INIT_EGL_FUNC(eglQuerySurface)
                INIT_EGL_FUNC(eglReleaseTexImage)
                INIT_EGL_FUNC(eglReleaseThread)
                INIT_EGL_FUNC(eglSurfaceAttrib)
                INIT_EGL_FUNC(eglSwapBuffers)
                INIT_EGL_FUNC(eglSwapBuffersWithDamageEXT)
                INIT_EGL_FUNC(eglSwapInterval)
                INIT_EGL_FUNC(eglTerminate)
                INIT_EGL_FUNC(eglUnlockSurfaceKHR)
                INIT_EGL_FUNC(eglWaitClient)
                INIT_EGL_FUNC(eglWaitGL)
                INIT_EGL_FUNC(eglWaitNative)

                EGLint configAttribs[] = {EGL_RED_SIZE, 8,
                                          EGL_GREEN_SIZE, 8,
                                          EGL_BLUE_SIZE, 8,
                                          EGL_ALPHA_SIZE, 8,
                                          EGL_DEPTH_SIZE, 24,
                                          EGL_SURFACE_TYPE, EGL_WINDOW_BIT|EGL_PBUFFER_BIT,
                                          EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
                                          EGL_NONE};

                EGLint ctxAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};

                EGLint pbAttribs[] = {EGL_WIDTH, 10, EGL_HEIGHT, 10, EGL_NONE};

                EGLConfig pbufConfig;
                EGLint configsFound = 0;

                eglDisplay = MG_External::EGL::eglGetDisplay(EGL_DEFAULT_DISPLAY);
                if (eglDisplay == EGL_NO_DISPLAY) {
                    goto cleanup;
                }

                if (MG_External::EGL::eglInitialize(eglDisplay, NULL, NULL) != EGL_TRUE) {
                    goto cleanup;
                }

                if (MG_External::EGL::eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE) {
                    goto cleanup;
                }

                if (MG_External::EGL::eglChooseConfig(eglDisplay, configAttribs, &pbufConfig, 1, &configsFound) !=
                    EGL_TRUE) {
                    goto cleanup;
                }

                if (configsFound == 0) {
                    configAttribs[6] = 0;
                    if (MG_External::EGL::eglChooseConfig(eglDisplay, configAttribs, &pbufConfig, 1, &configsFound) !=
                        EGL_TRUE) {
                        goto cleanup;
                    }
                    if (!configsFound) {
                        goto cleanup;
                    }
                }

                eglContext = MG_External::EGL::eglCreateContext(eglDisplay, pbufConfig, EGL_NO_CONTEXT, ctxAttribs);
                if (eglContext == EGL_NO_CONTEXT) {
                    goto cleanup;
                }

                eglSurface = MG_External::EGL::eglCreatePbufferSurface(eglDisplay, pbufConfig, pbAttribs);
                if (eglSurface == EGL_NO_SURFACE) {
                    goto cleanup;
                }

                if (MG_External::EGL::eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) != EGL_TRUE) {
                    goto cleanup;
                }

                return;

            cleanup:
                if (eglSurface != EGL_NO_SURFACE) {
                    MG_External::EGL::eglDestroySurface(eglDisplay, eglSurface);
                }
                if (eglContext != EGL_NO_CONTEXT) {
                    MG_External::EGL::eglDestroyContext(eglDisplay, eglContext);
                }
                if (eglDisplay != EGL_NO_DISPLAY) {
                    MG_External::EGL::eglTerminate(eglDisplay);
                }
            }

            void DestroyTempEGLCtx() {
                INIT_EGL_FUNC(eglDestroySurface)
                INIT_EGL_FUNC(eglDestroyContext)
                INIT_EGL_FUNC(eglMakeCurrent)
                INIT_EGL_FUNC(eglTerminate)

                MG_External::EGL::eglMakeCurrent(eglDisplay, 0, 0, EGL_NO_CONTEXT);
                MG_External::EGL::eglDestroySurface(eglDisplay, eglSurface);
                MG_External::EGL::eglDestroyContext(eglDisplay, eglContext);

                MG_External::EGL::eglTerminate(eglDisplay);
            }

#undef LOAD_EGL

        } // namespace BackendLoader::GLES
    } // namespace MG_Util
} // namespace MobileGL
