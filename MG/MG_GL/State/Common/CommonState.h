//
// Created by BZLZHH on 2025/4/30.
//

#ifndef MOBILEGL_COMMONSTATE_H
#define MOBILEGL_COMMONSTATE_H
#define MOBILEGL_GLSTATE_H

#include "../../../Includes.h"

class CommonState {
public: // TODO
    // Pixel storage parameters
    std::unordered_map<GLenum, GLint> pixelStoreParams;

    // Blend state
    GLenum blendSrcRGB = GL_ONE;
    GLenum blendDstRGB = GL_ZERO;
    GLenum blendSrcAlpha = GL_ONE;
    GLenum blendDstAlpha = GL_ZERO;
    
    // Clear state
    GLfloat clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    GLfloat clearDepth = 1.0f;
    
    // Color mask
    GLboolean colorMask[4] = {GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE};
    
    // Depth state
    GLenum depthFunc = GL_LESS;
    GLboolean depthMask = GL_TRUE;
    
    // Viewport
    GLint viewport[4] = {0, 0, 0, 0};
    
    // Capability enables
    std::unordered_map<GLenum, bool> capabilities;
    
public:
    CommonState();
    
    // Return: the validity of the operation, according to OpenGL 3 standard
    GLenum SetPixelStoreInt(GLenum pname, GLint param);

    GLenum Enable(GLenum cap);
    GLenum Disable(GLenum cap);
    GLenum BlendFunc(GLenum sfactor, GLenum dfactor);
    GLenum BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB,
                             GLenum srcAlpha, GLenum dstAlpha);
    GLenum Clear(GLbitfield mask);
    GLenum ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    GLenum ClearDepth(GLfloat depth);
    GLenum ColorMask(GLboolean red, GLboolean green,
                     GLboolean blue, GLboolean alpha);
    GLenum DepthFunc(GLenum func);
    GLenum DepthMask(GLboolean flag);
    GLenum Viewport(GLint x, GLint y, GLsizei width, GLsizei height);
    
    GLint QueryPixelStoreInt(GLenum pname);
};

#endif //MOBILEGL_COMMONSTATE_H
