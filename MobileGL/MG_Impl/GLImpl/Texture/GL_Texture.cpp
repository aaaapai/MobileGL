#include "GL_Texture.h"
#include "GL/gl.h"
#include "MG_Util/Types.h"
#include "Validators.h"
#include "ProxyTexture.h"
#include <MG_State/GLState/Core.h>
#include <MG_Util/Metrics/TextureMetrics.h>
#include <MG_State/GLState/ErrorState/Error.h>
#include <MG_Util/Converters/GLToMG/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToGL/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToStr/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToMG/TextureEnumConverter.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        void TexSubImage3D_State(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width,
                                 GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) {
            // TODO: implement
        }

        void TexSubImage2D_State(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                                 GLsizei height, GLenum format, GLenum type, const void* pixels) {
            // ======================= Converting ================================
            TextureUploadTarget textureUploadingTarget = MG_Util::ConvertGLEnumToTextureUploadTarget(target);
            TextureTarget textureTarget = MG_Util::ConvertTextureUploadTargetToTextureTarget(textureUploadingTarget);
            TextureInputFormat textureInputFormat = MG_Util::ConvertGLEnumToTextureInputFormat(format);
            TexturePixelDataType texturePixelDataType = MG_Util::ConvertGLEnumToTexturePixelDataType(type);
            TextureInternalFormat textureInternalFormat = MG_Util::ConvertGLEnumToTextureInternalFormat(format);

            // ===================== Error Checking ==============================
            if (!TextureImpl::ValidateTexturePixelDataType(texturePixelDataType)) return;
            if (!TextureImpl::ValidateTextureInputFormat(textureInputFormat)) return;
            if (!TextureImpl::ValidateTextureUploadTarget(textureUploadingTarget)) return;
            if (!TextureImpl::ValidateTextureLevelNumber(level)) return;
            if (!TextureImpl::ValidateTextureSizeWithTextureUploadTarget(textureUploadingTarget, width, height)) return;
            if (!TextureImpl::ValidateTextureSizeRange(width, height)) return;
            if (!TextureImpl::ValidateTextureInternalFormat(textureInternalFormat)) return;
            if (!TextureImpl::ValidateTextureFormatWithType(textureInputFormat, textureInternalFormat,
                                                            texturePixelDataType))
                return;
            if (!TextureImpl::ValidateTextureLevelWithUploadTarget(textureUploadingTarget, level)) return;

            // TODO: GL_INVALID_OPERATION is generated if a non-zero buffer object name is bound to the
            // GL_PIXEL_UNPACK_BUFFER target and the buffer object's data store is currently mapped.
            // GL_INVALID_OPERATION is generated if a non-zero buffer object name is bound to the
            // GL_PIXEL_UNPACK_BUFFER target and the data would be unpacked from the buffer object such that the
            // memory reads required would exceed the data store size. GL_INVALID_OPERATION is generated if a
            // non-zero buffer object name is bound to the GL_PIXEL_UNPACK_BUFFER target and data is not evenly
            // divisible into the number of bytes needed to store in memory a datum indicated by type.

            // ======================= Processing ================================
            auto activeUnit = MG_State::pGLContext->GetTextureUnitObject(MG_State::pGLContext->GetActiveTextureUnit());
            auto& bindingSlot = activeUnit.GetBindingSlot(textureTarget);
            auto textureObject = bindingSlot.GetBoundObject();

            // ===================== Error Checking ==============================
            if (!TextureImpl::ValidateTextureObject(textureObject)) return;
            if (!TextureImpl::ValidateTextureSubImageOffsets(textureObject, xoffset, width, yoffset, height)) return;

            // ======================= Processing ================================
            //            SizeT imageSize =
            //                    MG_Util::CalculateInputTextureImageSize(textureInternalFormat,
            //                                                            texturePixelDataType,
            //                                                            {width, height, 1});
            auto& mipmap = textureObject->GetMipmap(level);
            Vector<Uint8>& data = mipmap.data;

            SizeT bytesPerPixel = MG_Util::GetInputBytesPerPixel(textureInternalFormat, texturePixelDataType);
            //            SizeT rowStride = mipmap.size.x() * bytesPerPixel;

            const Uint8* srcData = reinterpret_cast<const Uint8*>(pixels);
            if (!srcData) {
                return;
            }

            if (pixels != nullptr) {
                auto rowsToSkip = MG_State::pGLContext->GetPixelStoreParam(PixelStoreParam::UnpackSkipRows);
                auto pixelsToSkip = MG_State::pGLContext->GetPixelStoreParam(PixelStoreParam::UnpackSkipPixels);

                srcData += (rowsToSkip * width + pixelsToSkip);
            }

            for (GLint row = 0; row < height; ++row) {
                SizeT dstOffset = ((yoffset + row) * mipmap.size.x() + xoffset) * bytesPerPixel;
                // Should take states from glPixelStorei/glPixelStoref into account
                SizeT srcOffset = row * width * bytesPerPixel;

                if (dstOffset + width * bytesPerPixel <= data.size()) {
                    Memcpy(data.data() + dstOffset, srcData + srcOffset, width * bytesPerPixel);
                } else {
                    MG_State::pGLContext->RecordError(
                        ErrorCode::InvalidOperation,
                        MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", __func__,
                                                     "Copy failed. Dest image does not have sufficient space."));
                    break;
                }
            }

            mipmap.dirty = true;
        }

        void TexSubImage1D_State(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type,
                                 const GLvoid* pixels) {
            // TODO: implement
        }

        void TexParameterf_State(GLenum target, GLenum pname, GLfloat param) {

            // ======================= Converting ================================
            TextureUploadTarget textureUploadingTarget = MG_Util::ConvertGLEnumToTextureUploadTarget(target);
            TextureTarget textureTarget = MG_Util::ConvertTextureUploadTargetToTextureTarget(textureUploadingTarget);

            // ======================= Processing ================================
            SharedPtr<MG_State::GLState::ITextureObject> textureObject = nullptr;
            if (TextureImpl::IsProxyTextureTarget(textureUploadingTarget)) {
                textureObject =
                    TextureImpl::pProxyTextureManager->CreateOrReplaceProxyTextureObject(textureUploadingTarget);
            } else {
                auto activeUnit =
                    MG_State::pGLContext->GetTextureUnitObject(MG_State::pGLContext->GetActiveTextureUnit());
                auto& bindingSlot = activeUnit.GetBindingSlot(textureTarget);
                textureObject = bindingSlot.GetBoundObject();
            }

            if (!TextureImpl::ValidateTextureObject(textureObject)) return;

            switch (pname) {
            case GL_TEXTURE_MAG_FILTER:
                textureObject->GetSamplerObject()->SetMagFilter(MG_Util::ConvertGLEnumToSamplerFilterMode(param));
                break;
            case GL_TEXTURE_MIN_FILTER:
                textureObject->GetSamplerObject()->SetMinFilter(MG_Util::ConvertGLEnumToSamplerFilterMode(param));
                break;
            case GL_TEXTURE_MIN_LOD: {
                Float maxLod = textureObject->GetSamplerObject()->GetMaxLod();
                textureObject->GetSamplerObject()->SetLodRange(param, maxLod);
                break;
            }
            case GL_TEXTURE_MAX_LOD: {
                Float minLod = textureObject->GetSamplerObject()->GetMinLod();
                textureObject->GetSamplerObject()->SetLodRange(minLod, param);
                break;
            }
            case GL_TEXTURE_BASE_LEVEL:
            case GL_TEXTURE_MAX_LEVEL:
            case GL_TEXTURE_SWIZZLE_R:
            case GL_TEXTURE_SWIZZLE_G:
            case GL_TEXTURE_SWIZZLE_B:
            case GL_TEXTURE_SWIZZLE_A:
            case GL_TEXTURE_SWIZZLE_RGBA:
            case GL_TEXTURE_BORDER_COLOR:
                break; // TODO
            case GL_TEXTURE_WRAP_S:
                textureObject->GetSamplerObject()->SetWrapS(MG_Util::ConvertGLEnumToSamplerWrapMode(param));
                break;
            case GL_TEXTURE_WRAP_T:
                textureObject->GetSamplerObject()->SetWrapT(MG_Util::ConvertGLEnumToSamplerWrapMode(param));
                break;
            case GL_TEXTURE_WRAP_R:
                textureObject->GetSamplerObject()->SetWrapR(MG_Util::ConvertGLEnumToSamplerWrapMode(param));
                break;
            case GL_TEXTURE_COMPARE_MODE:
                textureObject->GetSamplerObject()->SetCompareMode(MG_Util::ConvertGLEnumToSamplerCompareMode(param));
                break;
            case GL_TEXTURE_COMPARE_FUNC:
                textureObject->GetSamplerObject()->SetSamplerCompareFunc(
                    MG_Util::ConvertGLEnumToSamplerCompareFunc(param));
                break;
            default:
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "TexParameteri_State",
                                                                         "pname is not a valid texture parameter."));
                return;
            }
        }

        void TexParameteri_State(GLenum target, GLenum pname, GLint param) {
            // ======================= Converting ================================
            TextureUploadTarget textureUploadingTarget = MG_Util::ConvertGLEnumToTextureUploadTarget(target);
            TextureTarget textureTarget = MG_Util::ConvertTextureUploadTargetToTextureTarget(textureUploadingTarget);

            // ======================= Processing ================================
            SharedPtr<MG_State::GLState::ITextureObject> textureObject = nullptr;
            if (TextureImpl::IsProxyTextureTarget(textureUploadingTarget)) {
                textureObject =
                    TextureImpl::pProxyTextureManager->CreateOrReplaceProxyTextureObject(textureUploadingTarget);
            } else {
                auto activeUnit =
                    MG_State::pGLContext->GetTextureUnitObject(MG_State::pGLContext->GetActiveTextureUnit());
                auto& bindingSlot = activeUnit.GetBindingSlot(textureTarget);
                textureObject = bindingSlot.GetBoundObject();
            }

            if (!TextureImpl::ValidateTextureObject(textureObject)) return;

            switch (pname) {
            case GL_TEXTURE_MAG_FILTER:
                textureObject->GetSamplerObject()->SetMagFilter(MG_Util::ConvertGLEnumToSamplerFilterMode(param));
                break;
            case GL_TEXTURE_MIN_FILTER:
                textureObject->GetSamplerObject()->SetMinFilter(MG_Util::ConvertGLEnumToSamplerFilterMode(param));
                break;
            case GL_TEXTURE_MIN_LOD: {
                Float maxLod = textureObject->GetSamplerObject()->GetMaxLod();
                textureObject->GetSamplerObject()->SetLodRange(param, maxLod);
                break;
            }
            case GL_TEXTURE_MAX_LOD: {
                Float minLod = textureObject->GetSamplerObject()->GetMinLod();
                textureObject->GetSamplerObject()->SetLodRange(minLod, param);
                break;
            }
            case GL_TEXTURE_BASE_LEVEL:
            case GL_TEXTURE_MAX_LEVEL:
            case GL_TEXTURE_SWIZZLE_R:
            case GL_TEXTURE_SWIZZLE_G:
            case GL_TEXTURE_SWIZZLE_B:
            case GL_TEXTURE_SWIZZLE_A:
            case GL_TEXTURE_SWIZZLE_RGBA:
            case GL_TEXTURE_BORDER_COLOR:
                break; // TODO
            case GL_TEXTURE_WRAP_S:
                textureObject->GetSamplerObject()->SetWrapS(MG_Util::ConvertGLEnumToSamplerWrapMode(param));
                break;
            case GL_TEXTURE_WRAP_T:
                textureObject->GetSamplerObject()->SetWrapT(MG_Util::ConvertGLEnumToSamplerWrapMode(param));
                break;
            case GL_TEXTURE_WRAP_R:
                textureObject->GetSamplerObject()->SetWrapR(MG_Util::ConvertGLEnumToSamplerWrapMode(param));
                break;
            case GL_TEXTURE_COMPARE_MODE:
                textureObject->GetSamplerObject()->SetCompareMode(MG_Util::ConvertGLEnumToSamplerCompareMode(param));
                break;
            case GL_TEXTURE_COMPARE_FUNC:
                textureObject->GetSamplerObject()->SetSamplerCompareFunc(
                    MG_Util::ConvertGLEnumToSamplerCompareFunc(param));
                break;
            default:
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "TexParameteri_State",
                                                                         "pname is not a valid texture parameter."));
                return;
            }
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
            // ======================= Converting ================================
            TextureUploadTarget textureUploadingTarget = MG_Util::ConvertGLEnumToTextureUploadTarget(target);
            TextureTarget textureTarget = MG_Util::ConvertTextureUploadTargetToTextureTarget(textureUploadingTarget);
            TextureInputFormat textureInputFormat = MG_Util::ConvertGLEnumToTextureInputFormat(format);
            TexturePixelDataType texturePixelDataType = MG_Util::ConvertGLEnumToTexturePixelDataType(type);
            TextureInternalFormat textureInternalFormat = MG_Util::ConvertGLEnumToTextureInternalFormat(internalformat);

            // ===================== Error Checking ==============================
            if (!TextureImpl::ValidateTexturePixelDataType(texturePixelDataType)) return;
            if (!TextureImpl::ValidateTextureInputFormat(textureInputFormat)) return;
            if (!TextureImpl::ValidateTextureUploadTarget(textureUploadingTarget)) return;
            if (!TextureImpl::ValidateTextureLevelNumber(level)) return;
            if (!TextureImpl::ValidateTextureSizeWithTextureUploadTarget(textureUploadingTarget, width, height)) return;
            if (!TextureImpl::ValidateTextureSizeRange(width, height)) return;
            if (!TextureImpl::ValidateTextureInternalFormat(textureInternalFormat)) return;
            if (!TextureImpl::ValidateTextureBorderNumber(border)) return;
            if (!TextureImpl::ValidateTextureFormatWithType(textureInputFormat, textureInternalFormat,
                                                            texturePixelDataType))
                return;
            if (!TextureImpl::ValidateTextureLevelWithUploadTarget(textureUploadingTarget, level)) return;

            // TODO: GL_INVALID_OPERATION is generated if a non-zero buffer object name is bound to the
            // GL_PIXEL_UNPACK_BUFFER target and the buffer object's data store is currently mapped.
            // GL_INVALID_OPERATION is generated if a non-zero buffer object name is bound to the GL_PIXEL_UNPACK_BUFFER
            // target and the data would be unpacked from the buffer object such that the memory reads required would
            // exceed the data store size.
            // GL_INVALID_OPERATION is generated if a non-zero buffer object name is bound to the GL_PIXEL_UNPACK_BUFFER
            // target and data is not evenly divisible into the number of bytes needed to store in memory a datum
            // indicated by type.

            // ======================= Processing ================================
            SharedPtr<MG_State::GLState::ITextureObject> textureObject = nullptr;
            Bool isProxy = TextureImpl::IsProxyTextureTarget(textureUploadingTarget);
            if (isProxy) {
                textureObject =
                    TextureImpl::pProxyTextureManager->CreateOrReplaceProxyTextureObject(textureUploadingTarget);
            } else {
                auto activeUnit =
                    MG_State::pGLContext->GetTextureUnitObject(MG_State::pGLContext->GetActiveTextureUnit());
                auto& bindingSlot = activeUnit.GetBindingSlot(textureTarget);
                textureObject = bindingSlot.GetBoundObject();
            }

            // ===================== Error Checking ==============================
            if (!TextureImpl::ValidateTextureObject(textureObject)) return;

            // ======================= Processing ================================
            SizeT imageSize = isProxy ? 0
                                      : MG_Util::CalculateInputTextureImageSize(
                                            textureInternalFormat, texturePixelDataType, {width, height, 1});

            Uint8* skippedPixels = (Uint8*)pixels;
            if (pixels != nullptr) {
                auto rowsToSkip = MG_State::pGLContext->GetPixelStoreParam(PixelStoreParam::UnpackSkipRows);
                auto pixelsToSkip = MG_State::pGLContext->GetPixelStoreParam(PixelStoreParam::UnpackSkipPixels);

                skippedPixels += (rowsToSkip * width + pixelsToSkip);
            }

            textureObject->SetInternalFormat(textureInternalFormat);
            MG_State::GLState::MipmapLevelInput mipmap = MG_State::GLState::MipmapLevelInput(
                {width, height, 1}, level, false, 0,
                {const_cast<void*>(isProxy ? (void*)skippedPixels : nullptr), imageSize});
            textureObject->SetMipmapLevel(mipmap);
        }

        void TexImage1D_State(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border,
                              GLenum format, GLenum type, const GLvoid* pixels) {
            // TODO: implement
        }

        void TexBuffer_State(GLenum target, GLenum internalformat, GLuint texture) {
            // TODO: implement
        }

        GLboolean IsTexture_State(GLuint texture) {
            // ======================= Processing ================================
            if (!TextureImpl::ValidateTextureName(texture, true)) return GL_FALSE;
            return MG_State::pGLContext->ValidateTextureObject(texture) ? GL_TRUE : GL_FALSE;
        }

        void GetTexParameterIuiv_State(GLenum target, GLenum pname, GLuint* params) {
            // TODO: implement
        }

        void GetTexParameterIiv_State(GLenum target, GLenum pname, GLint* params) {
            // TODO: implement
        }

        void GetTexParameteriv_State(GLenum target, GLenum pname, GLint* params) {
            // ======================= Converting ================================
            TextureUploadTarget textureUploadingTarget = MG_Util::ConvertGLEnumToTextureUploadTarget(target);
            TextureTarget textureTarget = MG_Util::ConvertTextureUploadTargetToTextureTarget(textureUploadingTarget);

            // ======================= Processing ================================
            SharedPtr<MG_State::GLState::ITextureObject> textureObject = nullptr;
            if (TextureImpl::IsProxyTextureTarget(textureUploadingTarget)) {
                textureObject = TextureImpl::pProxyTextureManager->GetProxyTextureObject(textureUploadingTarget);
            } else {
                auto activeUnit =
                    MG_State::pGLContext->GetTextureUnitObject(MG_State::pGLContext->GetActiveTextureUnit());
                auto& bindingSlot = activeUnit.GetBindingSlot(textureTarget);
                textureObject = bindingSlot.GetBoundObject();
            }

            if (!TextureImpl::ValidateTextureObject(textureObject)) return;

            switch (pname) {
            case GL_TEXTURE_MAG_FILTER:
                if (params) {
                    *params =
                        MG_Util::ConvertSamplerFilterModeToGLEnum(textureObject->GetSamplerObject()->GetMagFilter());
                }
                break;
            case GL_TEXTURE_MIN_FILTER:
                if (params) {
                    *params =
                        MG_Util::ConvertSamplerFilterModeToGLEnum(textureObject->GetSamplerObject()->GetMinFilter());
                }
                break;
            case GL_TEXTURE_MIN_LOD:
                if (params) {
                    *params = static_cast<GLint>(textureObject->GetSamplerObject()->GetMinLod());
                }
                break;
            case GL_TEXTURE_MAX_LOD:
                if (params) {
                    *params = static_cast<GLint>(textureObject->GetSamplerObject()->GetMaxLod());
                }
                break;
            case GL_TEXTURE_BASE_LEVEL:
            case GL_TEXTURE_MAX_LEVEL:
            case GL_TEXTURE_SWIZZLE_R:
            case GL_TEXTURE_SWIZZLE_G:
            case GL_TEXTURE_SWIZZLE_B:
            case GL_TEXTURE_SWIZZLE_A:
            case GL_TEXTURE_SWIZZLE_RGBA:
            case GL_TEXTURE_BORDER_COLOR:
                break; // TODO
            case GL_TEXTURE_WRAP_S:
                if (params) {
                    *params = MG_Util::ConvertSamplerWrapModeToGLEnum(textureObject->GetSamplerObject()->GetWrapS());
                }
                break;
            case GL_TEXTURE_WRAP_T:
                if (params) {
                    *params = MG_Util::ConvertSamplerWrapModeToGLEnum(textureObject->GetSamplerObject()->GetWrapT());
                }
                break;
            case GL_TEXTURE_WRAP_R:
                if (params) {
                    *params = MG_Util::ConvertSamplerWrapModeToGLEnum(textureObject->GetSamplerObject()->GetWrapR());
                }
                break;
            case GL_TEXTURE_COMPARE_MODE:
                if (params) {
                    *params =
                        MG_Util::ConvertSamplerCompareModeToGLEnum(textureObject->GetSamplerObject()->GetCompareMode());
                }
                break;
            case GL_TEXTURE_COMPARE_FUNC:
                if (params) {
                    *params = MG_Util::ConvertSamplerCompareFuncToGLEnum(
                        textureObject->GetSamplerObject()->GetSamplerCompareFunc());
                }
                break;
            default:
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "GetTexParameteriv_State",
                                                                         "pname is not a valid texture parameter."));
                return;
            }
        }

        void GetTexParameterfv_State(GLenum target, GLenum pname, GLfloat* params) {
            // ======================= Converting ================================
            TextureUploadTarget textureUploadingTarget = MG_Util::ConvertGLEnumToTextureUploadTarget(target);
            TextureTarget textureTarget = MG_Util::ConvertTextureUploadTargetToTextureTarget(textureUploadingTarget);

            // ======================= Processing ================================
            SharedPtr<MG_State::GLState::ITextureObject> textureObject = nullptr;
            if (TextureImpl::IsProxyTextureTarget(textureUploadingTarget)) {
                textureObject = TextureImpl::pProxyTextureManager->GetProxyTextureObject(textureUploadingTarget);
            } else {
                auto activeUnit =
                    MG_State::pGLContext->GetTextureUnitObject(MG_State::pGLContext->GetActiveTextureUnit());
                auto& bindingSlot = activeUnit.GetBindingSlot(textureTarget);
                textureObject = bindingSlot.GetBoundObject();
            }

            if (!TextureImpl::ValidateTextureObject(textureObject)) return;

            switch (pname) {
            case GL_TEXTURE_MAG_FILTER:
                if (params) {
                    *params =
                        MG_Util::ConvertSamplerFilterModeToGLEnum(textureObject->GetSamplerObject()->GetMagFilter());
                }
                break;
            case GL_TEXTURE_MIN_FILTER:
                if (params) {
                    *params =
                        MG_Util::ConvertSamplerFilterModeToGLEnum(textureObject->GetSamplerObject()->GetMinFilter());
                }
                break;
            case GL_TEXTURE_MIN_LOD:
                if (params) {
                    *params = static_cast<GLfloat>(textureObject->GetSamplerObject()->GetMinLod());
                }
                break;
            case GL_TEXTURE_MAX_LOD:
                if (params) {
                    *params = static_cast<GLfloat>(textureObject->GetSamplerObject()->GetMaxLod());
                }
                break;
            case GL_TEXTURE_BASE_LEVEL:
            case GL_TEXTURE_MAX_LEVEL:
            case GL_TEXTURE_SWIZZLE_R:
            case GL_TEXTURE_SWIZZLE_G:
            case GL_TEXTURE_SWIZZLE_B:
            case GL_TEXTURE_SWIZZLE_A:
            case GL_TEXTURE_SWIZZLE_RGBA:
            case GL_TEXTURE_BORDER_COLOR:
                break; // TODO
            case GL_TEXTURE_WRAP_S:
                if (params) {
                    *params = MG_Util::ConvertSamplerWrapModeToGLEnum(textureObject->GetSamplerObject()->GetWrapS());
                }
                break;
            case GL_TEXTURE_WRAP_T:
                if (params) {
                    *params = MG_Util::ConvertSamplerWrapModeToGLEnum(textureObject->GetSamplerObject()->GetWrapT());
                }
                break;
            case GL_TEXTURE_WRAP_R:
                if (params) {
                    *params = MG_Util::ConvertSamplerWrapModeToGLEnum(textureObject->GetSamplerObject()->GetWrapR());
                }
                break;
            case GL_TEXTURE_COMPARE_MODE:
                if (params) {
                    *params =
                        MG_Util::ConvertSamplerCompareModeToGLEnum(textureObject->GetSamplerObject()->GetCompareMode());
                }
                break;
            case GL_TEXTURE_COMPARE_FUNC:
                if (params) {
                    *params = MG_Util::ConvertSamplerCompareFuncToGLEnum(
                        textureObject->GetSamplerObject()->GetSamplerCompareFunc());
                }
                break;
            default:
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum, MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "GetTexParameterfv_State",
                                                                         "pname is not a valid texture parameter."));
                return;
            }
        }

        void GetTexLevelParameteriv_State(GLenum target, GLint level, GLenum pname, GLint* params) {
            MGLOG_D("GetTexLevelParameteriv_State called");
            // ======================= Converting ================================
            TextureUploadTarget textureUploadingTarget = MG_Util::ConvertGLEnumToTextureUploadTarget(target);
            TextureTarget textureTarget = MG_Util::ConvertTextureUploadTargetToTextureTarget(textureUploadingTarget);

            // ===================== Error Checking ==============================
            if (!TextureImpl::ValidateTextureUploadTarget(textureUploadingTarget)) return;
            if (!TextureImpl::ValidateTextureLevelNumber(level)) return;
            if (!TextureImpl::ValidateTextureLevelWithUploadTarget(textureUploadingTarget, level)) return;

            // ======================= Processing ================================
            SharedPtr<MG_State::GLState::ITextureObject> textureObject = nullptr;

            if (TextureImpl::IsProxyTextureTarget(textureUploadingTarget)) {
                textureObject = TextureImpl::pProxyTextureManager->GetProxyTextureObject(textureUploadingTarget);
            } else {
                auto activeUnit =
                    MG_State::pGLContext->GetTextureUnitObject(MG_State::pGLContext->GetActiveTextureUnit());
                auto& bindingSlot = activeUnit.GetBindingSlot(textureTarget);
                textureObject = bindingSlot.GetBoundObject();
            }

            if (!TextureImpl::ValidateTextureObject(textureObject)) return;

            switch (pname) {
            case GL_TEXTURE_WIDTH:
                if (params) {
                    *params = textureObject->GetMipmap(level).size.x();
                }
                break;
            case GL_TEXTURE_HEIGHT:
                if (params) {
                    *params = textureObject->GetMipmap(level).size.y();
                }
                break;
            case GL_TEXTURE_DEPTH:
                if (params) {
                    *params = textureObject->GetMipmap(level).size.z();
                }
                break;
            case GL_TEXTURE_INTERNAL_FORMAT:
                if (params) {
                    *params = MG_Util::ConvertTextureInternalFormatToGLEnum(textureObject->GetFormat());
                }
                break;
            case GL_TEXTURE_RED_TYPE:
            case GL_TEXTURE_GREEN_TYPE:
            case GL_TEXTURE_BLUE_TYPE:
            case GL_TEXTURE_ALPHA_TYPE:
            case GL_TEXTURE_DEPTH_TYPE:
            case GL_TEXTURE_RED_SIZE:
            case GL_TEXTURE_GREEN_SIZE:
            case GL_TEXTURE_BLUE_SIZE:
            case GL_TEXTURE_ALPHA_SIZE:
            case GL_TEXTURE_DEPTH_SIZE:
            case GL_TEXTURE_COMPRESSED:
            case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
                break; // TODO
            default:
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "GetTexLevelParameteriv_State",
                                                 "pname is not a valid texture level parameter."));
                return;
            }
        }

        void GetTexLevelParameterfv_State(GLenum target, GLint level, GLenum pname, GLfloat* params) {
            // ======================= Converting ================================
            TextureUploadTarget textureUploadingTarget = MG_Util::ConvertGLEnumToTextureUploadTarget(target);
            TextureTarget textureTarget = MG_Util::ConvertTextureUploadTargetToTextureTarget(textureUploadingTarget);

            // ===================== Error Checking ==============================
            if (!TextureImpl::ValidateTextureUploadTarget(textureUploadingTarget)) return;
            if (!TextureImpl::ValidateTextureLevelNumber(level)) return;
            if (!TextureImpl::ValidateTextureLevelWithUploadTarget(textureUploadingTarget, level)) return;

            // ======================= Processing ================================
            SharedPtr<MG_State::GLState::ITextureObject> textureObject = nullptr;
            if (TextureImpl::IsProxyTextureTarget(textureUploadingTarget)) {
                textureObject = TextureImpl::pProxyTextureManager->GetProxyTextureObject(textureUploadingTarget);
            } else {
                auto activeUnit =
                    MG_State::pGLContext->GetTextureUnitObject(MG_State::pGLContext->GetActiveTextureUnit());
                auto& bindingSlot = activeUnit.GetBindingSlot(textureTarget);
                textureObject = bindingSlot.GetBoundObject();
            }

            if (!TextureImpl::ValidateTextureObject(textureObject)) return;

            switch (pname) {
            case GL_TEXTURE_WIDTH:
                if (params) {
                    *params = textureObject->GetMipmap(level).size.x();
                }
                break;
            case GL_TEXTURE_HEIGHT:
                if (params) {
                    *params = textureObject->GetMipmap(level).size.y();
                }
                break;
            case GL_TEXTURE_DEPTH:
                if (params) {
                    *params = textureObject->GetMipmap(level).size.z();
                }
                break;
            case GL_TEXTURE_INTERNAL_FORMAT:
                if (params) {
                    *params = MG_Util::ConvertTextureInternalFormatToGLEnum(textureObject->GetFormat());
                }
                break;
            case GL_TEXTURE_RED_TYPE:
            case GL_TEXTURE_GREEN_TYPE:
            case GL_TEXTURE_BLUE_TYPE:
            case GL_TEXTURE_ALPHA_TYPE:
            case GL_TEXTURE_DEPTH_TYPE:
            case GL_TEXTURE_RED_SIZE:
            case GL_TEXTURE_GREEN_SIZE:
            case GL_TEXTURE_BLUE_SIZE:
            case GL_TEXTURE_ALPHA_SIZE:
            case GL_TEXTURE_DEPTH_SIZE:
            case GL_TEXTURE_COMPRESSED:
            case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
                break; // TODO
            default:
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "GetTexLevelParameterfv_State",
                                                 "pname is not a valid texture level parameter."));
                return;
            }
        }

        void GetTexImage_State(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels) {
            // TODO: implement
        }

        void GetCompressedTexImage_State(GLenum target, GLint level, void* img) {
            // TODO: implement
        }

        void GenTextures_State(GLsizei n, GLuint* textures) {
            // ===================== Error Checking ==============================
            if (n < 0) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidValue,
                    MakeShared<GenericErrorInfo>("MG_Impl/GLImpl", "GenTextures_State", "n must be non-negative"));
                return;
            }

            // ======================= Processing ================================
            auto textureNames = MG_State::pGLContext->GenTextureNames(n);
            Copy(textureNames.data(), textures, textureNames.size());
        }

        void DeleteTextures_State(GLsizei n, const GLuint* textures) {
            // ===================== Error Checking ==============================
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

            // ======================= Processing ================================
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
            // ======================= Converting ================================
            TextureTarget textureTarget = MG_Util::ConvertGLEnumToTextureTarget(target);

            // ===================== Error Checking ==============================
            if (!TextureImpl::ValidateTextureName(texture, true)) return;
            if (!TextureImpl::ValidateTextureTarget(textureTarget)) return;

            // ======================= Processing ================================
            auto textureObject = MG_State::pGLContext->GetTextureObject(texture);

            // ===================== Error Checking ==============================
            if (!TextureImpl::ValidateTextureTargetUniformity(textureObject, textureTarget)) return;

            // ======================= Processing ================================
            if (!textureObject) {
                textureObject = MG_State::pGLContext->CreateTextureObject(texture, textureTarget);
            }

            auto& currentUnit =
                MG_State::pGLContext->GetTextureUnitObject(MG_State::pGLContext->GetActiveTextureUnit());
            auto& bindingSlot = currentUnit.GetBindingSlot(textureTarget);
            bindingSlot.Bind(textureObject);
        }

        void ActiveTexture_State(GLenum texture) {
            // ===================== Error Checking ==============================
            if (texture < GL_TEXTURE0 || texture > GL_TEXTURE31) {
                MG_State::pGLContext->RecordError(
                    ErrorCode::InvalidEnum,
                    MakeShared<GenericErrorInfo>(
                        "MG_Impl/GLImpl", "ActiveTexture_State",
                        "Texture must be one of GL_TEXTUREi, where i is in the range 0 to 31."));
                return;
            }

            // ======================= Processing ================================
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
