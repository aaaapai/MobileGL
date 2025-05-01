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
    GLenum GetTextureLevelPropertyIntVector(GLenum target, GLint level, GLenum pname, GLint* params);
    
    // Common
    GLenum SetPixelStoreInt(GLenum pname, GLint param);
    GLint GetPixelStoreInt(GLenum pname);
    
    // Buffer
    GLenum AcquireBufferMemory(GLenum target, GLenum access, void** mappedPtr);
    GLenum ReleaseBufferMemory(GLenum target);
    GLenum CreateBuffer(GLuint* buffer);
    GLenum CreateBuffers(GLsizei n, GLuint* buffers);
    GLenum BindBuffer(GLenum target, GLuint buffer);
    GLenum CommitBufferStorage(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
    bool ValidateBufferHandle(GLuint buffer);
    void DeleteBuffer(GLuint buffer);
    GLenum QueryBufferPropertyIntVector(GLenum target, GLenum pname, GLint* params);
    
    // VertexArray

    GLenum CreateVertexArray(GLuint* array);
    GLenum CreateVertexArrays(GLsizei n, GLuint* arrays);
    GLenum BindVertexArray(GLuint array);
    GLenum EnableVertexAttribArray(GLuint index);
    GLenum DisableVertexAttribArray(GLuint index);
    GLenum SetVertexAttributeLayout(GLuint index, GLint size, GLenum type,
                                    GLboolean normalized, GLsizei stride,
                                    const void* pointer);
    GLenum SetVertexAttributeLayoutInt(GLuint index, GLint size, GLenum type,
                                       GLsizei stride, const void* pointer);
}


#endif //MOBILEGL_GLSTATE_H
