//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_Common.h"

namespace MG_GL::GL {
    void ClearDepth(GLdouble depth) {
        
    }
    
    void ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
        
    }

    void Clear(GLbitfield mask) {
        
    }

    void PixelStorei(GLenum pname, GLint param) {
        MG_Util::Debug::LogD("glPixelStorei, pname: %d, param: %d", pname, param);
        GLenum result = MG_State::SetPixelStoreInt(pname,param);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }   
}