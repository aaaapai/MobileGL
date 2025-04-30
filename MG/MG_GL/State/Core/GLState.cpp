//
// Created by BZLZHH on 2025/4/4.
//

#include "GLState.h"

std::queue<GLenum> MG_State_T::glErrorQueue;
TextureState* MG_State_T::textureState = nullptr;
CommonState* MG_State_T::commonState = nullptr;

namespace MG_State {
    void Init() {
        MG_State_T::textureState = new TextureState();
        MG_State_T::commonState = new CommonState();
    }
    
    void Destroy() {
        delete MG_State_T::textureState;
        delete MG_State_T::commonState;
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
    
    GLenum GetTextureLevelPropertyIntVector(GLenum target, GLint level, GLenum pname, GLint* params) {
        return MG_State_T::textureState->GetLevelPropertyIntVector(target, level, pname, params);
    }

    GLenum MG_State::SetPixelStoreInt(GLenum pname, GLint param) {
        return MG_State_T::commonState->SetPixelStoreInt(pname, param);
    }

    GLint MG_State::GetPixelStoreInt(GLenum pname) {
        return MG_State_T::commonState->GetPixelStoreInt(pname);
    }

}