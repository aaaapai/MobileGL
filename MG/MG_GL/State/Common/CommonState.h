//
// Created by BZLZHH on 2025/4/30.
//

#ifndef MOBILEGL_COMMONSTATE_H
#define MOBILEGL_COMMONSTATE_H
#define MOBILEGL_GLSTATE_H

#include "../../../Includes.h"

class CommonState {
private:
    std::unordered_map<GLenum, GLint> pixelStoreParams;
    
public:
    // Return: the validity of the operation, according to OpenGL 3 standard
    GLenum SetPixelStoreInt(GLenum pname, GLint param);
    
    GLint QueryPixelStoreInt(GLenum pname);
};

#endif //MOBILEGL_COMMONSTATE_H
