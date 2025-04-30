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
    GLenum SetPixelStoreInt(GLenum pname, GLint param);
    GLint GetPixelStoreInt(GLenum pname);
};

#endif //MOBILEGL_COMMONSTATE_H
