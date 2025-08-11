#include "GL_Texture.h"

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        /* @INSERTION_POINT:FUNCTION_IMPLEMENTATION@ */
        void TexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) {
            // TODO: implement
        }
        
        void TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) {
            // TODO: implement
        }
        
        void TexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid* pixels) {
            // TODO: implement
        }
        
        void TexParameterf(GLenum target, GLenum pname, GLfloat param) {
            // TODO: implement
        }
        
        void TexParameteri(GLenum target, GLenum pname, GLint param) {
            // TODO: implement
        }
        
        void TexImage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) {
            // TODO: implement
        }
        
        void TexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) {
            // TODO: implement
        }
        
        void TexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) {
            // TODO: implement
        }
        
        void TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) {
            // TODO: implement
        }
        
        void TexImage1D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels) {
            // TODO: implement
        }
        
        void TexBuffer(GLenum target, GLenum internalformat, GLuint buffer) {
            // TODO: implement
        }
        
        GLboolean IsTexture(GLuint texture) {
            // TODO: implement
        }
        
        void GetTexParameterIuiv(GLenum target, GLenum pname, GLuint* params) {
            // TODO: implement
        }
        
        void GetTexParameterIiv(GLenum target, GLenum pname, GLint* params) {
            // TODO: implement
        }
        
        void GetTexParameteriv(GLenum target, GLenum pname, GLint* params) {
            // TODO: implement
        }
        
        void GetTexParameterfv(GLenum target, GLenum pname, GLfloat* params) {
            // TODO: implement
        }
        
        void GetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params) {
            // TODO: implement
        }
        
        void GetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params) {
            // TODO: implement
        }
        
        void GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels) {
            // TODO: implement
        }

        void GetCompressedTexImage(GLenum target, GLint level, void* img) {
            // TODO: implement
        }

        void GenTextures(GLsizei n, GLuint* textures) {
            // TODO: implement
        }

        void DeleteTextures(GLsizei n, const GLuint* textures) {
            // TODO: implement
        }

        void CopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x,
                               GLint y, GLsizei width, GLsizei height) {
            // TODO: implement
        }

        void CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
                               GLsizei width, GLsizei height) {
            // TODO: implement
        }

        void CopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
            // TODO: implement
        }

        void CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width,
                            GLsizei height, GLint border) {
            // TODO: implement
        }

        void CopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width,
                            GLint border) {
            // TODO: implement
        }

        void CompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                                     GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize,
                                     const void* data) {
            // TODO: implement
        }

        void CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                                     GLsizei height, GLenum format, GLsizei imageSize, const void* data) {
            // TODO: implement
        }

        void CompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format,
                                     GLsizei imageSize, const void* data) {
            // TODO: implement
        }

        void CompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height,
                                  GLsizei depth, GLint border, GLsizei imageSize, const void* data) {
            // TODO: implement
        }

        void CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height,
                                  GLint border, GLsizei imageSize, const void* data) {
            // TODO: implement
        }

        void CompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border,
                                  GLsizei imageSize, const void* data) {
            // TODO: implement
        }

        void BindTexture(GLenum target, GLuint texture) {
            // TODO: implement
        }

        void ActiveTexture(GLenum texture) {
            // TODO: implement
        }

    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
