//
// Created by BZLZHH on 2025/4/4.
//

#ifndef MOBILEGL_GLSTATE_H
#define MOBILEGL_GLSTATE_H

#include "../../../Includes.h"

struct MG_State_T {
    static std::queue<GLenum> glErrorQueue;
    static TextureState* textureState;
    static CommonState* commonState;
    static BufferState* bufferState;
    static VertexArrayState* vertexArrayState;
    static ProgramState* programState;
    static FramebufferState* framebufferState;
};

namespace MG_State {
    void Init();
    void Destroy();
    void SetError(GLenum error);
    GLenum GetError();
    
    //Texture
    GLenum BindTextureUnit(GLenum textureUnit);
    GLenum CreateTexture(GLuint* texture);
    GLenum CreateTextures(GLsizei n, GLuint* textures);
    GLenum BindTexture(GLenum target, GLuint texture);
    GLenum UploadTexture2D(GLenum target, GLint level, GLint internalFormat,
                           GLsizei width, GLsizei height, GLint border, GLenum format,
                           GLenum type, const void* data);
    GLenum UpdateTextureRegion2D(GLenum target, GLint level, GLint xoffset,
                                 GLint yoffset, GLsizei width, GLsizei height, GLenum format,
                                 GLenum type, const GLvoid* data);
    GLenum SetTexturePropertyInt(GLenum target, GLenum pname, GLint param);
    GLenum SetTexturePropertyFloat(GLenum target, GLenum pname, GLfloat param);
    GLenum DeleteTexture(GLuint texture);
    GLenum DeleteTextures(GLsizei n, const GLuint* textures);
    GLenum QueryTextureLevelPropertyIntVector(GLenum target, GLint level, GLenum pname, GLint* params);
    
    // Common
    GLenum SetPixelStoreInt(GLenum pname, GLint param);
    GLint QueryPixelStoreInt(GLenum pname);
    GLenum glEnable(GLenum cap);
    GLenum glDisable(GLenum cap);
    GLenum glBlendFunc(GLenum sfactor, GLenum dfactor);
    GLenum glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
    GLenum glClear(GLbitfield mask);
    GLenum glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    GLenum glClearDepth(GLdouble depth);
    GLenum glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    GLenum glDepthFunc(GLenum func);
    GLenum glDepthMask(GLboolean flag);
    GLenum glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
    
    // Buffer
    GLenum AcquireBufferMemory(GLenum target, GLenum access, void** mappedPtr);
    GLenum AcquireBufferMemoryRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access, void** mappedPointer);
    GLenum SyncBufferMemory(GLenum target, GLintptr offset, GLsizeiptr length);
    GLenum CopyBufferRange(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    GLenum ReleaseBufferMemory(GLenum target);
    GLenum CreateBuffer(GLuint buffer);
    GLenum GenBufferNames(GLsizei n, GLuint* buffers);
    GLenum BindBuffer(GLenum target, GLuint buffer);
    GLenum CommitBufferStorage(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
    GLenum CommitBufferStorageRegion(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
    bool ValidateAllocatedBufferHandle(GLuint buffer);
    bool ValidateGeneratedName(GLuint buffer);
    void DeleteBuffer(GLuint buffer);
    GLenum DeleteBuffers(GLsizei n, const GLuint* buffers);
    GLenum QueryBufferPropertyIntVector(GLenum target, GLenum pname, GLint* params);
    
    // VertexArray
    GLenum CreateVertexArray(GLuint* array);
    GLenum CreateVertexArrays(GLsizei n, GLuint* arrays);
    GLenum GenVertexArraysNames(GLsizei n, GLuint* arrays);
    GLenum BindVertexArray(GLuint array);
    GLenum EnableVertexAttribArray(GLuint index);
    GLenum DisableVertexAttribArray(GLuint index);
    GLenum SetVertexAttributeLayout(GLuint index, GLint size, GLenum type,
                                    GLboolean normalized, GLsizei stride,
                                    const void* pointer);
    GLenum SetVertexAttributeLayoutInt(GLuint index, GLint size, GLenum type,
                                       GLsizei stride, const void* pointer);
    
    // Program
    GLenum CreateShader(GLenum type, GLuint* shader);
    GLenum CreateProgram(GLuint* program);
    GLenum DeleteShader(GLuint shader);
    GLenum DeleteProgram(GLuint program);
    GLenum LinkShaderToProgram(GLuint program, GLuint shader);
    GLenum UploadShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
    GLenum BuildShaderStage(GLuint shader);
    GLenum FinalizeProgramPipeline(GLuint program);
    GLenum ActivateRenderProgram(GLuint program);
    GLenum DefineProgramAttributeBinding(GLuint program, GLuint index, const GLchar* name);
    GLint QueryProgramAttributeBinding(GLuint program, const GLchar* name);
    GLint QueryProgramUniformLocation(GLuint program, const GLchar* name);
    GLenum QueryProgramStateIntVector(GLuint program, GLenum pname, GLint* params);
    GLenum QueryShaderStateIntVector(GLuint shader, GLenum pname, GLint* params);
    template<typename T>
    GLenum SetUniform(GLuint program, GLint location, GLsizei count, const T* value, GLenum type);

    template<typename T>
    GLenum SetUniformMatrix(GLuint program, GLint location, GLsizei count, GLboolean transpose, const T* value, GLenum matrixType);

#define DECLARE_UNIFORM_FUNCTIONS(type, suffix, vecType) \
    void UpdateProgramUniform##suffix##1(GLint location, type v0); \
    void UpdateProgramUniform##suffix##2(GLint location, type v0, type v1); \
    void UpdateProgramUniform##suffix##3(GLint location, type v0, type v1, type v2); \
    void UpdateProgramUniform##suffix##4(GLint location, type v0, type v1, type v2, type v3); \
    void UpdateProgramUniform##suffix##Vector1(GLint location, GLsizei count, const type* value); \
    void UpdateProgramUniform##suffix##Vector2(GLint location, GLsizei count, const type* value); \
    void UpdateProgramUniform##suffix##Vector3(GLint location, GLsizei count, const type* value); \
    void UpdateProgramUniform##suffix##Vector4(GLint location, GLsizei count, const type* value);
    DECLARE_UNIFORM_FUNCTIONS(GLfloat, Float, GL_FLOAT)
    DECLARE_UNIFORM_FUNCTIONS(GLint, Int, GL_INT)
    DECLARE_UNIFORM_FUNCTIONS(GLuint, UInt, GL_UNSIGNED_INT)
    DECLARE_UNIFORM_FUNCTIONS(GLboolean, Bool, GL_BOOL)
#undef DECLARE_UNIFORM_FUNCTIONS
#define DECLARE_MATRIX_FUNCTIONS(suffix, matrixType) \
    void UpdateProgramUniformMatrix##suffix##Vector(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    DECLARE_MATRIX_FUNCTIONS(2x2, GL_FLOAT_MAT2)
    DECLARE_MATRIX_FUNCTIONS(3x3, GL_FLOAT_MAT3)
    DECLARE_MATRIX_FUNCTIONS(4x4, GL_FLOAT_MAT4)
    DECLARE_MATRIX_FUNCTIONS(2x3, GL_FLOAT_MAT2x3)
    DECLARE_MATRIX_FUNCTIONS(2x4, GL_FLOAT_MAT2x4)
    DECLARE_MATRIX_FUNCTIONS(3x2, GL_FLOAT_MAT3x2)
    DECLARE_MATRIX_FUNCTIONS(3x4, GL_FLOAT_MAT3x4)
    DECLARE_MATRIX_FUNCTIONS(4x2, GL_FLOAT_MAT4x2)
    DECLARE_MATRIX_FUNCTIONS(4x3, GL_FLOAT_MAT4x3)
#undef DECLARE_MATRIX_FUNCTIONS
    
    GLuint GetCurrentProgram();
    bool SetProgramStatus(GLuint program, GLboolean status);
    bool SetShaderStatus(GLuint shader, GLboolean status);
    
    // Framebuffer
    GLenum CreateFramebuffer(GLuint* framebuffer);
    GLenum CreateFramebuffers(GLsizei n, GLuint* framebuffers);
    GLenum DeleteFramebuffer(GLuint framebuffer);
    GLenum BindFramebuffer(GLenum target, GLuint framebuffer);
    GLenum AttachTexture2DToFramebuffer(GLenum target, GLenum attachment, GLenum textarget,
                                        GLuint texture, GLint level);
    GLenum ValidateFramebufferCompleteness(GLenum target);
}


#endif //MOBILEGL_GLSTATE_H
