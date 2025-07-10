//
// Created by BZLZHH on 2025/5/1.
//

#ifndef MOBILEGL_BUFFERSTATE_H
#define MOBILEGL_BUFFERSTATE_H

class BufferState {
public:
    struct BufferObject {
        GLenum usage = GL_STATIC_DRAW;
        std::vector<GLubyte> data;
        bool dataValid = false;
        bool dirty = false; // TODO: encapsulate this with an public API to RHI
        bool isMapped = false;
        bool generated = false;
        bool isDynamic = false;
        GLenum accessMode = GL_READ_WRITE;
        GLintptr mapOffset = 0;
        GLsizeiptr mapLength = 0;
        GLbitfield mapAccessFlags = 0;
    };

    // Return: the validity of the operation, according to OpenGL 3 standard
//    GLenum GenName(GLuint* buffer);
    GLenum GenNameN(GLsizei n, GLuint* buffers);
    GLenum Create(GLuint buffer);
    GLenum Bind(GLenum target, GLuint buffer);
    GLenum CommitStorage(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
    GLenum CommitStorageRegion(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
    GLenum AcquireBufferMemoryRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access, void** mappedPointer);
    GLenum SyncBufferMemory(GLenum target, GLintptr offset, GLsizeiptr length);
    GLenum CopyBufferRange(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    GLenum AcquireBufferMemory(GLenum target, GLenum access, void** mappedPointer);
    GLenum ReleaseBufferMemory(GLenum target);
    GLenum QueryPropertyIntVector(GLenum target, GLenum pname, GLint* params) const;
    GLenum DeleteN(GLsizei n, const GLuint* buffers);
    
    bool ValidateAllocatedHandle(GLuint buffer) const;
    bool ValidateGeneratedName(GLuint buffer) const;
    void Delete(GLuint buffer);
    GLuint GetCurrentBinding(GLenum target) const;

    BufferObject* GetBufferObject(GLuint buffer);

    MG_Global::unordered_map<GLenum, GLuint> currentBindings_;
//    MG_Global::unordered_map<GLuint, BufferObject> buffers_;
private:
//    std::vector<GLuint> freeId_;
//    GLuint lastId_ = 1;
    IndexGenerator<GLuint> indexGenerator_;
    std::vector<std::unique_ptr<BufferObject>> bufferObjects_;

    static bool IsValidTarget_(GLenum target);
};

#endif //MOBILEGL_BUFFERSTATE_H
