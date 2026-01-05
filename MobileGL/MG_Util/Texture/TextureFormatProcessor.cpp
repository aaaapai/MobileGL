// MobileGL - MobileGL/MG_Util/Texture/TextureFormatProcessor.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "TextureFormatProcessor.h"
#include "MG_Util/Converters/GLToStr/GLEnumConverter.h"

namespace MobileGL::MG_Util::TextureFormatProcessor {
    void NormalizePixelFormat(GLenum internalFormat, GLenum* outInternalFormat, GLenum* outType,
                              GLenum* outFormat) {
#ifdef TRACY_ENABLE
        ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
        switch (internalFormat) {
            case GL_DEPTH_COMPONENT16:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_SHORT;
                if (outFormat) *outFormat = GL_DEPTH_COMPONENT;
                break;

            case GL_DEPTH_COMPONENT24:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_INT;
                if (outFormat) *outFormat = GL_DEPTH_COMPONENT;
                break;

            case GL_DEPTH_COMPONENT32:
                if (outInternalFormat) *outInternalFormat = GL_DEPTH_COMPONENT32F;
                if (outType) *outType = GL_FLOAT;
                if (outFormat) *outFormat = GL_DEPTH_COMPONENT;
                break;

            case GL_DEPTH_COMPONENT32F:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_FLOAT;
                if (outFormat) *outFormat = GL_DEPTH_COMPONENT;
                break;

            case GL_DEPTH_COMPONENT:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_INT;
                if (outFormat) *outFormat = GL_DEPTH_COMPONENT;
                break;
            case GL_DEPTH32F_STENCIL8:
            case GL_DEPTH_STENCIL:
                if (outInternalFormat) *outInternalFormat = GL_DEPTH32F_STENCIL8;
                if (outType) *outType = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
                if (outFormat) *outFormat = GL_DEPTH_STENCIL;
                break;

            case GL_RGB10_A2:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_INT_2_10_10_10_REV;
                if (outFormat) *outFormat = GL_RGBA;
                break;

            case GL_RGB5_A1:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_SHORT_5_5_5_1;
                if (outFormat) *outFormat = GL_RGBA;
                break;

            case GL_COMPRESSED_RED_RGTC1:
            case GL_COMPRESSED_RG_RGTC2:
                break;

            case GL_SRGB8:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RGB;
                break;

            case GL_RGBA32F:
            case GL_RGB32F:
            case GL_RG32F:
            case GL_R32F:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_FLOAT;
                if (outFormat) switch (internalFormat) {
                        case GL_RGBA32F:
                            if (outFormat) *outFormat = GL_RGBA;
                            break;
                        case GL_RGB32F:
                            if (outFormat) *outFormat = GL_RGB;
                            break;
                        case GL_RG32F:
                            if (outFormat) *outFormat = GL_RG;
                            break;
                        case GL_R32F:
                            if (outFormat) *outFormat = GL_RED;
                            break;
                    }
                break;

            case GL_RGB9_E5:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_INT_5_9_9_9_REV;
                if (outFormat) *outFormat = GL_RGB;
                break;

            case GL_R11F_G11F_B10F:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_INT_10F_11F_11F_REV;
                if (outFormat) *outFormat = GL_RGB;
                break;

            case GL_RGBA32UI:
            case GL_RGB32UI:
            case GL_RG32UI:
            case GL_R32UI:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_INT;
                if (outFormat) switch (internalFormat) {
                        case GL_RGBA32UI:
                            if (outFormat) *outFormat = GL_RGBA_INTEGER;
                            break;
                        case GL_RGB32UI:
                            if (outFormat) *outFormat = GL_RGB_INTEGER;
                            break;
                        case GL_RG32UI:
                            if (outFormat) *outFormat = GL_RG_INTEGER;
                            break;
                        case GL_R32UI:
                            if (outFormat) *outFormat = GL_RED_INTEGER;
                            break;
                    }
                break;

            case GL_RGBA32I:
            case GL_RGB32I:
            case GL_RG32I:
            case GL_R32I:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_INT;
                if (outFormat) switch (internalFormat) {
                        case GL_RGBA32I:
                            if (outFormat) *outFormat = GL_RGBA_INTEGER;
                            break;
                        case GL_RGB32I:
                            if (outFormat) *outFormat = GL_RGB_INTEGER;
                            break;
                        case GL_RG32I:
                            if (outFormat) *outFormat = GL_RG_INTEGER;
                            break;
                        case GL_R32I:
                            if (outFormat) *outFormat = GL_RED_INTEGER;
                            break;
                    }
                break;

            case GL_RGBA16: {
                // TODO: check for extension GL_EXT_texture_norm16 for eligibility of (GL_RGBA16, GL_UNSIGNED_SHORT)
                // Most Mali does not support this (< Mali-G6xx, some G720?)
                if (outInternalFormat) *outInternalFormat = GL_RGBA16F;
                if (outType) *outType = GL_FLOAT;
                if (outFormat) *outFormat = GL_RGBA;
                break;
            }
            case GL_RGBA8:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RGBA;
                break;

            case GL_RGBA:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RGBA;
                break;

            case GL_RGBA16F:
            case GL_R16F:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_HALF_FLOAT;
                if (outFormat) {
                    if (internalFormat == GL_RGBA16F) {
                        *outFormat = GL_RGBA;
                    } else {
                        *outFormat = GL_RED;
                    }
                }
                break;

            case GL_RGB16:
                if (outInternalFormat) *outInternalFormat = GL_RGB16F;
                if (outType) *outType = GL_HALF_FLOAT;
                if (outFormat) *outFormat = GL_RGB;
                break;

            case GL_RGB16F:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_HALF_FLOAT;
                if (outFormat) *outFormat = GL_RGB;
                break;

            case GL_RG16:
                if (outInternalFormat) *outInternalFormat = GL_RG16F;
                if (outType) *outType = GL_HALF_FLOAT;
                if (outFormat) *outFormat = GL_RG;
                break;
            case GL_R16:
                /* TODO: By using R16F as R16, we're losing ~5bit precision along the way,
                 * deal with this later.
                 * This R16F as R16 conversion should only be applied on ES,
                 * see: https://registry.khronos.org/OpenGL/extensions/OES/OES_texture_buffer.txt
                 * ```
                 *   Issues:
                 *   (6) Should the R16, RG16 and RGBA16 texture formats be supported?
                         RESOLVED.  No. OpenGL ES 3.0 does not support these formats. They were
                         considered for late addition to OpenGL ES 3.1 in Bug 11366, but didn't
                         make the cut. In the absence of another extension to add them, they
                         are not supported here either.
                    ```
                 */
                if (outInternalFormat) *outInternalFormat = GL_R16F;
                if (outType) *outType = GL_FLOAT;
                if (outFormat) *outFormat = GL_RED;
                break;
            case GL_R8:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RED;
                break;

            case GL_R8_SNORM:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_BYTE;
                if (outFormat) *outFormat = GL_RED;
                break;

            case GL_RED:
                // For GL_RED, we need to infer based on type or use default
                if (outInternalFormat) *outInternalFormat = GL_R8; // Default fallback
                if (outType) *outType = GL_UNSIGNED_BYTE;          // Default fallback
                if (outFormat) *outFormat = GL_RED;
                break;

            case GL_R8UI:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RED_INTEGER;
                break;

            case GL_R8I:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_BYTE;
                if (outFormat) *outFormat = GL_RED_INTEGER;
                break;

            case GL_R16UI:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_SHORT;
                if (outFormat) *outFormat = GL_RED_INTEGER;
                break;

            case GL_R16I:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_SHORT;
                if (outFormat) *outFormat = GL_RED_INTEGER;
                break;

            case GL_RG8:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RG;
                break;

            case GL_RG8_SNORM:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_BYTE;
                if (outFormat) *outFormat = GL_RG;
                break;

            case GL_RG16F:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_HALF_FLOAT;
                if (outFormat) *outFormat = GL_RG;
                break;

            case GL_RG8UI:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RG_INTEGER;
                break;

            case GL_RG8I:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_BYTE;
                if (outFormat) *outFormat = GL_RG_INTEGER;
                break;

            case GL_RG16UI:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_SHORT;
                if (outFormat) *outFormat = GL_RG_INTEGER;
                break;

            case GL_RG16I:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_SHORT;
                if (outFormat) *outFormat = GL_RG_INTEGER;
                break;


            case GL_RGB8_SNORM:
            case GL_RGBA8_SNORM:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_BYTE;
                if (outFormat) {
                    if (internalFormat == GL_RGB8_SNORM) {
                        *outFormat = GL_RGB;
                    } else {
                        *outFormat = GL_RGBA;
                    }
                }
                break;

            case GL_RGB8:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RGB;
                break;

            case GL_RGBA16_SNORM:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_SHORT;
                if (outFormat) *outFormat = GL_RGBA;
                break;

            default:
                MGLOG_E("NormalizePixelFormat: unhandled internalFormat: %s", MG_Util::ConvertGLEnumToString(internalFormat).c_str());
                // Fallback handling for other formats
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE; // More reasonable default
                if (outFormat) {
                    // Try to infer format from internal format name
                    if (strstr(MG_Util::ConvertGLEnumToString(internalFormat).c_str(), "RGBA") != nullptr) {
                        *outFormat = GL_RGBA;
                    } else if (strstr(MG_Util::ConvertGLEnumToString(internalFormat).c_str(), "RGB") != nullptr) {
                        *outFormat = GL_RGB;
                    } else if (strstr(MG_Util::ConvertGLEnumToString(internalFormat).c_str(), "RG") != nullptr) {
                        *outFormat = GL_RG;
                    } else if (strstr(MG_Util::ConvertGLEnumToString(internalFormat).c_str(), "RED") != nullptr) {
                        *outFormat = GL_RED;
                    } else {
                        *outFormat = GL_RGBA; // Ultimate fallback
                    }
                }
                break;
        }
    }
} // namespace MobileGL::MG_Util::TextureFormatProcessor