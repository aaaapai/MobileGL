//
// Created by BZLZHH on 2025/4/4.
//

#include "GLState.h"

std::queue<GLenum> MG_State_T::glErrorQueue;
TextureState* MG_State_T::textureState = nullptr;
CommonState* MG_State_T::commonState = nullptr;
BufferState* MG_State_T::bufferState = nullptr;
VertexArrayState* MG_State_T::vertexArrayState = nullptr;
ProgramState* MG_State_T::programState = nullptr;
FramebufferState* MG_State_T::framebufferState = nullptr;

namespace MG_State {
    void Init() {
        MG_State_T::textureState = new TextureState();
        MG_State_T::commonState = new CommonState();
        MG_State_T::bufferState = new BufferState();
        MG_State_T::vertexArrayState = new VertexArrayState();
        MG_State_T::programState = new ProgramState();
        MG_State_T::framebufferState = new FramebufferState();
    }
    
    void Destroy() {
        delete MG_State_T::textureState;
        delete MG_State_T::commonState;
        delete MG_State_T::bufferState;
        delete MG_State_T::vertexArrayState;
        delete MG_State_T::programState;
        delete MG_State_T::framebufferState;
    }
    
    void SetError(GLenum error) {
        if (error == GL_NO_ERROR) {
            return;
        }
        MG_State_T::glErrorQueue.push(error);
    }

    GLenum GetError() {
        if (MG_State_T::glErrorQueue.empty()) {
            return GL_NO_ERROR;
        }
        GLenum error = MG_State_T::glErrorQueue.front();
        MG_State_T::glErrorQueue.pop();
        return error;
    }

// TODO: Take these functions apart into multiple files.

    // Texture
    GLenum BindTextureUnit(GLenum textureUnit) {
        return MG_State_T::textureState->BindUnit(textureUnit);
    }
    
    GLenum CreateTexture(GLuint* texture) {
        return MG_State_T::textureState->Create(texture);
    }
    
    GLenum CreateTextures(GLsizei n, GLuint* textures) {
        return MG_State_T::textureState->CreateN(n, textures);
    }
    
    GLenum BindTexture(GLenum target, GLuint texture) {
        return MG_State_T::textureState->Bind(target, texture);
    }
    
    GLenum UploadTexture2D(GLenum target, GLint level, GLint internalFormat,
                           GLsizei width, GLsizei height, GLint border, GLenum format,
                           GLenum type, const void* data) {
        return MG_State_T::textureState->Upload2D(target, level, internalFormat, width, height, border, format, type, data);
    }

    GLenum UpdateTextureRegion2D(GLenum target, GLint level, GLint xoffset,
                                 GLint yoffset, GLsizei width, GLsizei height, GLenum format,
                                 GLenum type, const GLvoid* data) {
        return MG_State_T::textureState->UpdateRegion2D(target, level, xoffset, yoffset, width, height, format, type, data);
    }
    
    GLenum SetTexturePropertyInt(GLenum target, GLenum pname, GLint param) {
        return MG_State_T::textureState->SetTexturePropertyInt(target, pname, param);
    }
    
    GLenum SetTexturePropertyFloat(GLenum target, GLenum pname, GLfloat param) {
        return MG_State_T::textureState->SetTexturePropertyFloat(target, pname, param);
    }
    
    GLenum DeleteTexture(GLuint texture) {
        return MG_State_T::textureState->Delete(texture);
    }
    
    GLenum DeleteTextures(GLsizei n, const GLuint* textures) { 
        return MG_State_T::textureState->DeleteN(n, textures);
    }
    
    GLenum QueryTextureLevelPropertyIntVector(GLenum target, GLint level, GLenum pname, GLint* params) {
        return MG_State_T::textureState->QueryLevelPropertyIntVector(target, level, pname, params);
    }
    
    // Common
    GLenum SetPixelStoreInt(GLenum pname, GLint param) {
        return MG_State_T::commonState->SetPixelStoreInt(pname, param);
    }

    GLint QueryPixelStoreInt(GLenum pname) {
        return MG_State_T::commonState->QueryPixelStoreInt(pname);
    }
    
    GLenum glEnable(GLenum cap) {
        return MG_State_T::commonState->Enable(cap);
    }

    GLenum glDisable(GLenum cap) {
        return MG_State_T::commonState->Disable(cap);
    }

    GLenum glBlendFunc(GLenum sfactor, GLenum dfactor) {
        return MG_State_T::commonState->BlendFunc(sfactor, dfactor);
    }

    GLenum glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
        return MG_State_T::commonState->BlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
    }

    GLenum glClear(GLbitfield mask) {
        return MG_State_T::commonState->Clear(mask);
    }

    GLenum glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
        return MG_State_T::commonState->ClearColor(red, green, blue, alpha);
    }

    GLenum glClearDepth(GLdouble depth) {
        return MG_State_T::commonState->ClearDepth(static_cast<GLfloat>(depth));
    }

    GLenum glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
        return MG_State_T::commonState->ColorMask(red, green, blue, alpha);
    }

    GLenum glDepthFunc(GLenum func) {
        return MG_State_T::commonState->DepthFunc(func);
    }

    GLenum glDepthMask(GLboolean flag) {
        return MG_State_T::commonState->DepthMask(flag);
    }

    GLenum glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
        return MG_State_T::commonState->Viewport(x, y, width, height);
    }

    // Buffer
    GLenum AcquireBufferMemory(GLenum target, GLenum access, void** mappedPtr) {
        return MG_State_T::bufferState->AcquireBufferMemory(target, access, mappedPtr);
    }
    
    GLenum AcquireBufferMemoryRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access, void** mappedPointer) {
        return MG_State_T::bufferState->AcquireBufferMemoryRange(target, offset, length, access, mappedPointer);
    }

    GLenum SyncBufferMemory(GLenum target, GLintptr offset, GLsizeiptr length) {
        return MG_State_T::bufferState->SyncBufferMemory(target, offset, length);
    }

    GLenum CopyBufferRange(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) {
        return MG_State_T::bufferState->CopyBufferRange(readTarget, writeTarget, readOffset,
                                                        writeOffset, size);
    }

    GLenum ReleaseBufferMemory(GLenum target) {
        return MG_State_T::bufferState->ReleaseBufferMemory(target);
    }

    GLenum CreateBuffer(GLuint buffer) {
        return MG_State_T::bufferState->Create(buffer);
    }

    GLenum GenBufferNames(GLsizei n, GLuint* buffers) {
        return MG_State_T::bufferState->GenNameN(n, buffers);
    }

    GLenum BindBuffer(GLenum target, GLuint buffer) {
        if (target == GL_ELEMENT_ARRAY_BUFFER) {
            auto* vao = MG_State_T::vertexArrayState->GetCurrentVAO();
            if (!vao) return GL_INVALID_OPERATION;
            vao->elementBuffer = (GLuint)buffer;
            vao->eboDirty = true;
        }
        return MG_State_T::bufferState->Bind(target, buffer);
    }

    GLenum CommitBufferStorage(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
        return MG_State_T::bufferState->CommitStorage(target, size, data, usage);
    }

    GLenum CommitBufferStorageRegion(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) {
        return MG_State_T::bufferState->CommitStorageRegion(target, offset, size, data);
    }

    bool ValidateAllocatedBufferHandle(GLuint buffer) {
        return MG_State_T::bufferState->ValidateAllocatedHandle(buffer);
    }

    bool ValidateGeneratedName(GLuint buffer) {
        return MG_State_T::bufferState->ValidateGeneratedName(buffer);
    }

    void DeleteBuffer(GLuint buffer) {
        return MG_State_T::bufferState->Delete(buffer);
    }

    GLenum DeleteBuffers(GLsizei n, const GLuint* buffers) {
        return MG_State_T::bufferState->DeleteN(n, buffers);
    }

    GLenum QueryBufferPropertyIntVector(GLenum target, GLenum pname, GLint* params) {
        return MG_State_T::bufferState->QueryPropertyIntVector(target, pname, params);
    }
    
    // VertexArray
    GLenum BindVertexArray(GLuint array) {
        return MG_State_T::vertexArrayState->Bind(array);
    }

    GLenum CreateVertexArray(GLuint* array) {
        return MG_State_T::vertexArrayState->Create(array);
    }

    GLenum CreateVertexArrays(GLsizei n, GLuint* arrays) {
        return MG_State_T::vertexArrayState->CreateN(n, arrays);
    }

    GLenum GenVertexArraysNames(GLsizei n, GLuint* arrays) {
        return MG_State_T::vertexArrayState->GenNameN(n, arrays);
    }

    GLenum EnableVertexAttribArray(GLuint index) {
        return MG_State_T::vertexArrayState->EnableAttrib(index);
    }

    GLenum DisableVertexAttribArray(GLuint index) {
        return MG_State_T::vertexArrayState->DisableAttrib(index);
    }

    GLenum SetVertexAttributeLayout(GLuint index, GLint size, GLenum type,
                               GLboolean normalized, GLsizei stride,
                               const void* pointer) {
        GLuint currentBuffer = MG_State_T::bufferState->GetCurrentBinding(GL_ARRAY_BUFFER);
        return MG_State_T::vertexArrayState->SetAttribPointer(
                index, size, type, normalized, stride, pointer, false, currentBuffer
        );
    }

    GLenum SetVertexAttributeLayoutInt(GLuint index, GLint size, GLenum type,
                                GLsizei stride, const void* pointer) {
        GLuint currentBuffer = MG_State_T::bufferState->GetCurrentBinding(GL_ARRAY_BUFFER);
        return MG_State_T::vertexArrayState->SetAttribPointer(
                index, size, type, GL_FALSE, stride, pointer, true, currentBuffer
        );
    }
    
    // Program
    GLenum CreateShader(GLenum type, GLuint* shader) {
        return MG_State_T::programState->CreateShader(type, shader);
    }

    GLenum CreateProgram(GLuint* program) {
        return MG_State_T::programState->CreateProgram(program);
    }

    GLenum DeleteShader(GLuint shader) {
        return MG_State_T::programState->DeleteShader(shader);
    }

    GLenum DeleteProgram(GLuint program) {
        return MG_State_T::programState->DeleteProgram(program);
    }

    GLenum LinkShaderToProgram(GLuint program, GLuint shader) {
        return MG_State_T::programState->LinkShaderToProgram(program, shader);
    }

    GLenum UploadShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length) {
        return MG_State_T::programState->UploadShaderSource(shader, count, string, length);
    }

    GLenum BuildShaderStage(GLuint shader) {
        return MG_State_T::programState->BuildShaderStage(shader);
    }

    GLenum FinalizeProgramPipeline(GLuint program) {
        return MG_State_T::programState->FinalizeProgramPipeline(program);
    }

    GLenum ActivateRenderProgram(GLuint program) {
        return MG_State_T::programState->ActivateRenderProgram(program);
    }

    GLenum DefineProgramAttributeLocation(GLuint program, GLuint index, const GLchar* name) {
        return MG_State_T::programState->DefineProgramAttributeLocation(program, index, name);
    }

    GLint QueryProgramAttributeLocation(GLuint program, const GLchar* name) {
        return MG_State_T::programState->QueryProgramAttributeLocation(program, name);
    }

    GLint QueryProgramUniformLocation(GLuint program, const GLchar* name) {
        return MG_State_T::programState->QueryProgramUniformLocation(program, name);
    }

    GLenum QueryProgramStateIntVector(GLuint program, GLenum pname, GLint* params) {
        return MG_State_T::programState->QueryProgramStateIntVector(program, pname, params);
    }

    GLenum QueryShaderStateIntVector(GLuint shader, GLenum pname, GLint* params) {
        return MG_State_T::programState->QueryShaderStateIntVector(shader, pname, params);
    }

#define IMPLEMENT_UNIFORM_FUNCTIONS(type, suffix, vecType)                               \
void UpdateProgramUniform##suffix##1(GLint location, type v0) {                   \
    MG_State_T::programState->UpdateUniform##suffix##1(location, v0); \
}                                                                                        \
void UpdateProgramUniform##suffix##2(GLint location, type v0, type v1) {          \
    MG_State_T::programState->UpdateUniform##suffix##2(location, v0, v1); \
}                                                                                        \
void UpdateProgramUniform##suffix##3(GLint location, type v0, type v1, type v2) { \
    MG_State_T::programState->UpdateUniform##suffix##3(location, v0, v1, v2); \
}                                                                                        \
void UpdateProgramUniform##suffix##4(GLint location, type v0, type v1, type v2, type v3) { \
    MG_State_T::programState->UpdateUniform##suffix##4(location, v0, v1, v2, v3); \
    } \
void UpdateProgramUniform##suffix##Vector1(GLint location, GLsizei count, const type* value) { \
    MG_State_T::programState->UpdateUniform##suffix##Vector1(location, count,value); \
    } \
void UpdateProgramUniform##suffix##Vector2(GLint location, GLsizei count, const type* value) { \
    MG_State_T::programState->UpdateUniform##suffix##Vector2(location, count, value); \
    } \
void UpdateProgramUniform##suffix##Vector3(GLint location, GLsizei count, const type* value) { \
    MG_State_T::programState->UpdateUniform##suffix##Vector3(location, count, value); \
    } \
void UpdateProgramUniform##suffix##Vector4(GLint location, GLsizei count, const type* value) { \
    MG_State_T::programState->UpdateUniform##suffix##Vector4(location, count, value); \
    }

    IMPLEMENT_UNIFORM_FUNCTIONS(GLfloat, Float, GL_FLOAT_VEC4)
    IMPLEMENT_UNIFORM_FUNCTIONS(GLint, Int, GL_INT_VEC4)
    IMPLEMENT_UNIFORM_FUNCTIONS(GLuint, UInt, GL_UNSIGNED_INT_VEC4)
    IMPLEMENT_UNIFORM_FUNCTIONS(GLboolean, Bool, GL_BOOL_VEC4)
#undef IMPLEMENT_UNIFORM_FUNCTIONS

#define IMPLEMENT_MATRIX_FUNCTIONS(suffix, matrixType) \
void UpdateProgramUniformMatrix##suffix##Vector(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { \
    MG_State_T::programState->UpdateUniformMatrix##suffix##Vector(location, count, transpose, value); \
}

    IMPLEMENT_MATRIX_FUNCTIONS(2x2, GL_FLOAT_MAT2)
    IMPLEMENT_MATRIX_FUNCTIONS(3x3, GL_FLOAT_MAT3)
    IMPLEMENT_MATRIX_FUNCTIONS(4x4, GL_FLOAT_MAT4)
    IMPLEMENT_MATRIX_FUNCTIONS(2x3, GL_FLOAT_MAT2x3)
    IMPLEMENT_MATRIX_FUNCTIONS(2x4, GL_FLOAT_MAT2x4)
    IMPLEMENT_MATRIX_FUNCTIONS(3x2, GL_FLOAT_MAT3x2)
    IMPLEMENT_MATRIX_FUNCTIONS(3x4, GL_FLOAT_MAT3x4)
    IMPLEMENT_MATRIX_FUNCTIONS(4x2, GL_FLOAT_MAT4x2)
    IMPLEMENT_MATRIX_FUNCTIONS(4x3, GL_FLOAT_MAT4x3)
#undef IMPLEMENT_MATRIX_FUNCTIONS
    
    GLuint GetCurrentProgram() {
        return MG_State_T::programState->GetCurrentProgram();
    }
    
    bool SetProgramStatus(GLuint program, GLboolean status) {
        return MG_State_T::programState->SetProgramStatus(program, status);
    }
    
    bool SetShaderStatus(GLuint shader, GLboolean status) {
        return MG_State_T::programState->SetShaderStatus(shader, status);
    }
    
    // Framebuffer
    GLenum MG_State::CreateFramebuffer(GLuint* framebuffer) {
        return MG_State_T::framebufferState->Create(framebuffer);
    }
    
    GLenum MG_State::CreateFramebuffers(GLsizei n, GLuint* framebuffers) {
        return MG_State_T::framebufferState->CreateN(n, framebuffers);
    }
    
    GLenum MG_State::DeleteFramebuffer(GLuint framebuffer) {
        return MG_State_T::framebufferState->Delete(framebuffer);
    }
    
    GLenum MG_State::BindFramebuffer(GLenum target, GLuint framebuffer) {
        return MG_State_T::framebufferState->Bind(target, framebuffer);
    }
    
    GLenum MG_State::AttachTexture2DToFramebuffer(GLenum target, GLenum attachment, GLenum textarget,
                                                  GLuint texture, GLint level) {
        return MG_State_T::framebufferState->AttachTexture2D(target, attachment, textarget,
                                                             texture, level);
    }
    
    GLenum MG_State::ValidateFramebufferCompleteness(GLenum target) {
        return MG_State_T::framebufferState->ValidateCompleteness(target);
    }
}