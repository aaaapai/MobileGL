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
    std::vector<TextureUnitState> textureUnits;
    GLuint activeTextureUnit = 0;

    GLuint lastUsedID = 0;
    std::set<GLuint> freeIDs;

    std::unordered_map<GLuint, TextureObject> textures;
    std::unordered_map<GLenum, TextureObject> proxyTextures;
public:
    TextureState();
    
    static TextureState& GetInstance();
    bool IsTextureGenerated(GLuint texture);
    bool IsTexture(GLuint texture);

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
    GLenum GetLevelPropertyIntVector(GLenum target, GLint level, GLenum pname, GLint* params);

private:
    static static size_t CalculatePixelDataSize(GLenum format, GLenum type, GLsizei width, GLsizei height);
    void InvalidateTextureInAllUnits(GLuint texture);
    static ComponentSizes GetComponentSizes(GLenum internalFormat);
    GLenum CheckUploadingTexture2DValidity(GLenum target, GLint level, GLint internalFormat,
                                           GLsizei width, GLsizei height, GLint border, GLenum format,
                                           GLenum type, const void* data);
};

#endif //MOBILEGL_TEXTURESTATE_H
