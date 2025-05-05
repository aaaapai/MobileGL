//
// Created by BZLZHH on 2025/5/1.
//

#ifndef MOBILEGL_BUFFERSTATE_H
#define MOBILEGL_BUFFERSTATE_H
#define MOBILEGL_GLSTATE_H

#include "../../../Includes.h"

class BufferState {
public:
    struct BufferObject {
        GLenum target = 0;
        GLenum usage = GL_STATIC_DRAW;
        std::vector<GLubyte> data;
        bool isMapped = false;
        bool generated = false;
        GLenum accessMode = GL_READ_WRITE;
    };

    // Return: the validity of the operation, according to OpenGL 3 standard
    GLenum Create(GLuint* buffer);
    GLenum CreateN(GLsizei n, GLuint* buffers);
    GLenum Bind(GLenum target, GLuint buffer);
    GLenum CommitStorage(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
    GLenum AcquireBufferMemory(GLenum target, GLenum access, void** mappedPointer);
    GLenum ReleaseBufferMemory(GLenum target);
    GLenum QueryPropertyIntVector(GLenum target, GLenum pname, GLint* params) const;
    
    bool ValidateHandle(GLuint buffer);
    void Delete(GLuint buffer);
    GLuint GetCurrentBinding(GLenum target) const;

    std::unordered_map<GLenum, GLuint> currentBindings_;
    std::unordered_map<GLuint, BufferObject> buffers_;
private:
    std::set<GLuint> freeIds_;
    GLuint lastId_ = 0;

    static bool IsValidTarget_(GLenum target);
};

#endif //MOBILEGL_BUFFERSTATE_H
