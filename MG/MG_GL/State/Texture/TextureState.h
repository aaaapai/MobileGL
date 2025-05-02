//
// Created by BZLZHH on 2025/4/4.
//

#ifndef MOBILEGL_TEXTURESTATE_H
#define MOBILEGL_TEXTURESTATE_H
#define MOBILEGL_GLSTATE_H

#include "../../../Includes.h"

struct ComponentSizes {
    GLint red = 0;  
    GLint green = 0;
    GLint blue = 0; 
    GLint alpha = 0;
    GLint depth = 0;
    GLint stencil = 0;
    bool isCompressed = false;
};

struct TextureParams {
    std::unordered_map<GLenum, GLfloat> texPropertiesFloat;
    std::unordered_map<GLenum, GLint> texPropertiesInt;
    struct MipmapLevel {
        GLsizei width = 0;
        GLsizei height = 0;
        GLenum format = GL_RGBA;
        GLenum internalFormat = GL_RGBA;
        GLenum type = GL_UNSIGNED_BYTE;
        std::vector<GLubyte> pixelData;
        bool hasData = false;
    };
    std::unordered_map<GLint, MipmapLevel> mipmapData;
};

class TextureObject {
public:
    bool generated = false;
    GLenum target = GL_NONE;
    TextureParams params;
    const void* data = nullptr;
    bool IsImmutable() const;
    uint64_t createTimestamp{};
};

class TextureUnitState {
private:
    GLenum activeTarget = GL_TEXTURE_2D;
    
public:
    std::unordered_map<GLenum, GLuint> boundTextures;
    void Bind(GLenum target, GLuint texture);
    GLuint GetBoundTexture(GLenum target);
};

class TextureState {
private:
    std::vector<TextureUnitState> textureUnits_;
    GLuint activeTextureUnit_ = 0;

    GLuint lastUsedID_ = 0;
    std::set<GLuint> freeIDs_;

    std::unordered_map<GLenum, TextureObject> proxyTextures_;
    
    static GLint GetUnpackParam_(GLenum pname);
public:
    TextureState();
    
    bool IsTextureGenerated(GLuint texture);
    bool IsTexture(GLuint texture);
    std::unordered_map<GLuint, TextureObject> textures;

    // Return: the validity of the operation, according to OpenGL 3 standard
    GLenum BindUnit(GLenum textureUnit);
    GLenum Create(GLuint* texture);
    GLenum CreateN(GLsizei n, GLuint* textures);
    GLenum Bind(GLenum target, GLuint texture);
    GLenum Upload2D(GLenum target, GLint level, GLint internalFormat,
                    GLsizei width, GLsizei height, GLint border, GLenum format,
                    GLenum type, const void* data);
    GLenum UpdateRegion2D(GLenum target, GLint level, GLint xoffset,
                          GLint yoffset, GLsizei width, GLsizei height, GLenum format,
                          GLenum type, const GLvoid* data);
    GLenum SetTexturePropertyInt(GLenum target, GLenum pname, GLint param);
    GLenum SetTexturePropertyFloat(GLenum target, GLenum pname, GLfloat param);
    GLenum Delete(GLuint texture);
    GLenum DeleteN(GLsizei n, const GLuint* textures);
    GLenum QueryLevelPropertyIntVector(GLenum target, GLint level, GLenum pname, GLint* params);

private:
    size_t CalculatePixelDataSize_(GLenum format, GLenum type, GLsizei width, GLsizei height);
    void InvalidateTextureInAllUnits_(GLuint texture);
    static ComponentSizes GetComponentSize_s_(GLenum internalFormat);
    size_t CalculateBytesPerPixel_(GLenum format, GLenum type);
    static size_t GetComponentSize_(GLenum type);
    void SwapBytesForTexture_(GLenum format, GLenum type, const GLubyte* src, GLubyte* dst, GLsizei width);
    static void ReverseBitOrder_(const GLubyte* src, GLubyte* dst, size_t size);
    static GLubyte ReverseBits_(GLubyte b);
    void SwapPixelBytes_(GLenum format, GLenum type, const GLubyte* src, GLubyte* dst);
    GLenum CheckUploadingTexture2DValidity_(GLenum target, GLint level, GLint internalFormat,
                                            GLsizei width, GLsizei height, GLint border, GLenum format,
                                            GLenum type, const void* data);
    GLenum CheckUpdatingTextureRegion2DValidity_(GLenum target, GLint level, GLint xoffset,
                                                 GLint yoffset, GLsizei width, GLsizei height, GLenum format,
                                                 GLenum type, const GLvoid* data);
};

#endif //MOBILEGL_TEXTURESTATE_H
