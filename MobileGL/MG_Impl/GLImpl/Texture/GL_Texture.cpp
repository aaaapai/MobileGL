#include "GL_Texture.h"
#include "Validators.h"
#include <MG_State/GLState/Core.h>
#include <MG_State/GLState/ErrorState/Error.h>
#include <MG_Util/Converters/GLToMG/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToGL/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToStr/TextureEnumConverter.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        void TexSubImage3D_State(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width,
                                 GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) {
            // TODO: implement
        }

        void TexSubImage2D_State(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                                 GLsizei height, GLenum format, GLenum type, const void* pixels) {
            // TODO: implement
        }

        void TexSubImage1D_State(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type,
                                 const GLvoid* pixels) {
            // TODO: implement
        }

        void TexParameterf_State(GLenum target, GLenum pname, GLfloat param) {
            // TODO: implement
        }

        void TexParameteri_State(GLenum target, GLenum pname, GLint param) {
            // TODO: implement
        }

        void TexImage3DMultisample_State(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
                                         GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) {
            // TODO: implement
        }

        void TexImage2DMultisample_State(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
                                         GLsizei height, GLboolean fixedsamplelocations) {
            // TODO: implement
        }

        void TexImage3D_State(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
                              GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) {
            // TODO: implement
        }

        void TexImage2D_State(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
                              GLint border, GLenum format, GLenum type, const void* pixels) {
            // TODO: implement
        }

        void TexImage1D_State(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border,
                              GLenum format, GLenum type, const GLvoid* pixels) {
            // TODO: implement
        }

        void TexBuffer_State(GLenum target, GLenum internalformat, GLuint texture) {
            // TODO: implement
        }

        GLboolean IsTexture_State(GLuint texture) {
            if (!TextureImpl::ValidateTextureName(texture)) return GL_FALSE;
            return MG_State::pGLContext->ValidateTextureObject(texture) ? GL_TRUE : GL_FALSE;
        }

        void GetTexParameterIuiv_State(GLenum target, GLenum pname, GLuint* params) {
            // TODO: implement
        }

        void GetTexParameterIiv_State(GLenum target, GLenum pname, GLint* params) {
            // TODO: implement
        }

        void GetTexParameteriv_State(GLenum target, GLenum pname, GLint* params) {
            // TODO: implement
        }

        void GetTexParameterfv_State(GLenum target, GLenum pname, GLfloat* params) {
            // TODO: implement
        }

        void GetTexLevelParameteriv_State(GLenum target, GLint level, GLenum pname, GLint* params) {
            // TODO: implement
        }

        void GetTexLevelParameterfv_State(GLenum target, GLint level, GLenum pname, GLfloat* params) {
            // TODO: implement
        }

        void GetTexImage_State(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels) {
            // TODO: implement
        }

        void GetCompressedTexImage_State(GLenum target, GLint level, void* img) {
            // TODO: implement
        }

        void GenTextures_State(GLsizei n, GLuint* textures) {
            if (n < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "GenTextures_State", "n must be non-negative"));
                return;
            }
            auto textureNames = MG_State::pGLContext->GenTextureNames(n);
            Copy(textureNames.data(), textures, textureNames.size());
        }

        void DeleteTextures_State(GLsizei n, const GLuint* textures) {
            if (n < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "DeleteTextures_State", "n must be non-negative."));
                return;
            }

            if (!textures) {
                MG_State::pGLContext->RecordError(ErrorCode::InvalidValue,
                                                  MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "DeleteTextures_State",
                                                                               "Texture names array cannot be null."));
                return;
            }

            for (SizeT i = 0; i < static_cast<SizeT>(n); ++i) {
                Uint textureName = textures[i];
                if (textureName == 0) continue;
                if (!TextureImpl::ValidateTextureName(textureName, true)) continue;
                MG_State::pGLContext->MarkTextureObjectForDeletion(textureName);
            }
        }

        void CopyTexSubImage3D_State(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x,
                                     GLint y, GLsizei width, GLsizei height) {
            // TODO: implement
        }

        void CopyTexSubImage2D_State(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
                                     GLsizei width, GLsizei height) {
            // TODO: implement
        }

        void CopyTexSubImage1D_State(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
            // TODO: implement
        }

        void CopyTexImage2D_State(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width,
                                  GLsizei height, GLint border) {
            // TODO: implement
        }

        void CopyTexImage1D_State(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width,
                                  GLint border) {
            // TODO: implement
        }

        void CompressedTexSubImage3D_State(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                                           GLsizei width, GLsizei height, GLsizei depth, GLenum format,
                                           GLsizei imageSize, const void* data) {
            // TODO: implement
        }

        void CompressedTexSubImage2D_State(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                                           GLsizei height, GLenum format, GLsizei imageSize, const void* data) {
            // TODO: implement
        }

        void CompressedTexSubImage1D_State(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format,
                                           GLsizei imageSize, const void* data) {
            // TODO: implement
        }

        void CompressedTexImage3D_State(GLenum target, GLint level, GLenum internalformat, GLsizei width,
                                        GLsizei height, GLsizei depth, GLint border, GLsizei imageSize,
                                        const void* data) {
            // TODO: implement
        }

        void CompressedTexImage2D_State(GLenum target, GLint level, GLenum internalformat, GLsizei width,
                                        GLsizei height, GLint border, GLsizei imageSize, const void* data) {
            // TODO: implement
        }

        void CompressedTexImage1D_State(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border,
                                        GLsizei imageSize, const void* data) {
            // TODO: implement
        }

        void BindTexture_State(GLenum target, GLuint texture) {
            if (!TextureImpl::ValidateTextureName(texture, true)) return;

            TextureTarget textureTarget = MG_Util::ConvertGLEnumToTextureTarget(target);
            if (!TextureImpl::ValidateTextureTarget(textureTarget)) return;

            auto textureObject = MG_State::pGLContext->GetTextureObject(texture);

            if (!textureObject) {
                textureObject = MG_State::pGLContext->CreateTextureObject(texture, textureTarget);
            } else if (textureObject->GetTarget() != textureTarget) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidOperation,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "BindTexture_State",
                                                 "Texture target does not match the previously created texture"));
                return;
            }

            auto& currentUnit =
                MG_State::pGLContext->GetTextureUnitObject(MG_State::pGLContext->GetActiveTextureUnit());
            auto& bindingSlot = currentUnit.GetBindingSlot(textureTarget);
            bindingSlot.Bind(textureObject);
        }

        void ActiveTexture_State(GLenum texture) {
            if (texture < GL_TEXTURE0 || texture > GL_TEXTURE31) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>(
                        "MG_Impl/GLImpl", "ActiveTexture_State",
                        "Texture must be one of GL_TEXTUREi, where i is in the range 0 to 31."));
                return;
            }

            MG_State::pGLContext->SetActiveTextureUnit(texture - GL_TEXTURE0);
        }

        /* @INSERTION_POINT:FUNCTION_IMPLEMENTATION@ */
        void TexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width,
                           GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) {
            TexSubImage3D_State(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
        }

        void TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
                           GLenum format, GLenum type, const void* pixels) {
            TexSubImage2D_State(target, level, xoffset, yoffset, width, height, format, type, pixels);
        }

        void TexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type,
                           const GLvoid* pixels) {
            TexSubImage1D_State(target, level, xoffset, width, format, type, pixels);
        }

        void TexParameterf(GLenum target, GLenum pname, GLfloat param) {
            TexParameterf_State(target, pname, param);
        }

        void TexParameteri(GLenum target, GLenum pname, GLint param) {
            TexParameteri_State(target, pname, param);
        }

        void TexImage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height,
                                   GLsizei depth, GLboolean fixedsamplelocations) {
            TexImage3DMultisample_State(target, samples, internalformat, width, height, depth, fixedsamplelocations);
        }

        void TexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height,
                                   GLboolean fixedsamplelocations) {
            TexImage2DMultisample_State(target, samples, internalformat, width, height, fixedsamplelocations);
        }

        void TexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth,
                        GLint border, GLenum format, GLenum type, const void* pixels) {
            TexImage3D_State(target, level, internalformat, width, height, depth, border, format, type, pixels);
        }

        void TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border,
                        GLenum format, GLenum type, const void* pixels) {
            TexImage2D_State(target, level, internalformat, width, height, border, format, type, pixels);
        }

        void TexImage1D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format,
                        GLenum type, const GLvoid* pixels) {
            TexImage1D_State(target, level, internalFormat, width, border, format, type, pixels);
        }

        void TexBuffer(GLenum target, GLenum internalformat, GLuint texture) {
            TexBuffer_State(target, internalformat, texture);
        }

        GLboolean IsTexture(GLuint texture) {
            return IsTexture_State(texture);
        }

        void GetTexParameterIuiv(GLenum target, GLenum pname, GLuint* params) {
            GetTexParameterIuiv_State(target, pname, params);
        }

        void GetTexParameterIiv(GLenum target, GLenum pname, GLint* params) {
            GetTexParameterIiv_State(target, pname, params);
        }

        void GetTexParameteriv(GLenum target, GLenum pname, GLint* params) {
            GetTexParameteriv_State(target, pname, params);
        }

        void GetTexParameterfv(GLenum target, GLenum pname, GLfloat* params) {
            GetTexParameterfv_State(target, pname, params);
        }

        void GetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params) {
            GetTexLevelParameteriv_State(target, level, pname, params);
        }

        void GetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params) {
            GetTexLevelParameterfv_State(target, level, pname, params);
        }

        void GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels) {
            GetTexImage_State(target, level, format, type, pixels);
        }

        void GetCompressedTexImage(GLenum target, GLint level, void* img) {
            GetCompressedTexImage_State(target, level, img);
        }

        void GenTextures(GLsizei n, GLuint* textures) {
            GenTextures_State(n, textures);
        }

        void DeleteTextures(GLsizei n, const GLuint* textures) {
            DeleteTextures_State(n, textures);
        }

        void CopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x,
                               GLint y, GLsizei width, GLsizei height) {
            CopyTexSubImage3D_State(target, level, xoffset, yoffset, zoffset, x, y, width, height);
        }

        void CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
                               GLsizei width, GLsizei height) {
            CopyTexSubImage2D_State(target, level, xoffset, yoffset, x, y, width, height);
        }

        void CopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
            CopyTexSubImage1D_State(target, level, xoffset, x, y, width);
        }

        void CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width,
                            GLsizei height, GLint border) {
            CopyTexImage2D_State(target, level, internalformat, x, y, width, height, border);
        }

        void CopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width,
                            GLint border) {
            CopyTexImage1D_State(target, level, internalformat, x, y, width, border);
        }

        void CompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                                     GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize,
                                     const void* data) {
            CompressedTexSubImage3D_State(target, level, xoffset, yoffset, zoffset, width, height, depth, format,
                                          imageSize, data);
        }

        void CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                                     GLsizei height, GLenum format, GLsizei imageSize, const void* data) {
            CompressedTexSubImage2D_State(target, level, xoffset, yoffset, width, height, format, imageSize, data);
        }

        void CompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format,
                                     GLsizei imageSize, const void* data) {
            CompressedTexSubImage1D_State(target, level, xoffset, width, format, imageSize, data);
        }

        void CompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height,
                                  GLsizei depth, GLint border, GLsizei imageSize, const void* data) {
            CompressedTexImage3D_State(target, level, internalformat, width, height, depth, border, imageSize, data);
        }

        void CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height,
                                  GLint border, GLsizei imageSize, const void* data) {
            CompressedTexImage2D_State(target, level, internalformat, width, height, border, imageSize, data);
        }

        void CompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border,
                                  GLsizei imageSize, const void* data) {
            CompressedTexImage1D_State(target, level, internalformat, width, border, imageSize, data);
        }

        void BindTexture(GLenum target, GLuint texture) {
            BindTexture_State(target, texture);
        }

        void ActiveTexture(GLenum texture) {
            ActiveTexture_State(texture);
        }

    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
