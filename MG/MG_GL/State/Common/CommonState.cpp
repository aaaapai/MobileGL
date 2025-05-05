//
// Created by BZLZHH on 2025/4/30.
//

#include "CommonState.h"

CommonState::CommonState() {
    pixelStoreParams[GL_PACK_ALIGNMENT] = 4;
    pixelStoreParams[GL_UNPACK_ALIGNMENT] = 4;
}

GLenum CommonState::SetPixelStoreInt(GLenum pname, GLint param) {
    if (MG_Constants::PixelStore::VALID_PARAM_NAMES.find(pname) == MG_Constants::PixelStore::VALID_PARAM_NAMES.end()) {
        return GL_INVALID_ENUM;
    }

    switch (pname) {
        case GL_PACK_SWAP_BYTES:
        case GL_PACK_LSB_FIRST:
        case GL_UNPACK_SWAP_BYTES:
        case GL_UNPACK_LSB_FIRST:
            break;
        case GL_PACK_ROW_LENGTH:
        case GL_PACK_IMAGE_HEIGHT:
        case GL_PACK_SKIP_ROWS:
        case GL_PACK_SKIP_PIXELS:
        case GL_PACK_SKIP_IMAGES:
        case GL_UNPACK_ROW_LENGTH:
        case GL_UNPACK_IMAGE_HEIGHT:
        case GL_UNPACK_SKIP_ROWS:
        case GL_UNPACK_SKIP_PIXELS:
        case GL_UNPACK_SKIP_IMAGES:
            if (param < 0) {
                return GL_INVALID_VALUE;
            }
            break;

        case GL_PACK_ALIGNMENT:
        case GL_UNPACK_ALIGNMENT:
            if (param != 1 && param != 2 && param != 4 && param != 8) {
                return GL_INVALID_VALUE;
            }
            break;

        default:
            return GL_INVALID_ENUM;
    }

    pixelStoreParams[pname] = param;
    return GL_NO_ERROR;
}

GLint CommonState::QueryPixelStoreInt(GLenum pname) {
    auto it = pixelStoreParams.find(pname);
    if (it != pixelStoreParams.end()) {
        return it->second;
    }

    auto defaultIt = MG_Constants::PixelStore::DEFAULT_VALUES_MAP.find(pname);
    return (defaultIt != MG_Constants::PixelStore::DEFAULT_VALUES_MAP.end()) ? defaultIt->second : 0;
}

GLenum CommonState::Enable(GLenum cap) {
    if (MG_Constants::CommonState::VALID_CAPS.find(cap) == MG_Constants::CommonState::VALID_CAPS.end()) {
        return GL_INVALID_ENUM;
    }
    capabilities[cap] = true;
    return GL_NO_ERROR;
}

GLenum CommonState::Disable(GLenum cap) {
    if (MG_Constants::CommonState::VALID_CAPS.find(cap) == MG_Constants::CommonState::VALID_CAPS.end()) {
        return GL_INVALID_ENUM;
    }
    capabilities[cap] = false;
    return GL_NO_ERROR;
}

GLenum CommonState::BlendFunc(GLenum sfactor, GLenum dfactor) {
    if (MG_Constants::Blend::VALID_FACTORS.find(sfactor) == MG_Constants::Blend::VALID_FACTORS.end() ||
        MG_Constants::Blend::VALID_FACTORS.find(dfactor) == MG_Constants::Blend::VALID_FACTORS.end()) {
        return GL_INVALID_ENUM;
    }

    blendSrcRGB = blendSrcAlpha = sfactor;
    blendDstRGB = blendDstAlpha = dfactor;
    return GL_NO_ERROR;
}

GLenum CommonState::BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB,
                                      GLenum srcAlpha, GLenum dstAlpha) {
    if (MG_Constants::Blend::VALID_FACTORS.find(srcRGB) == MG_Constants::Blend::VALID_FACTORS.end() ||
        MG_Constants::Blend::VALID_FACTORS.find(dstRGB) == MG_Constants::Blend::VALID_FACTORS.end() ||
        MG_Constants::Blend::VALID_FACTORS.find(srcAlpha) == MG_Constants::Blend::VALID_FACTORS.end() ||
        MG_Constants::Blend::VALID_FACTORS.find(dstAlpha) == MG_Constants::Blend::VALID_FACTORS.end()) {
        return GL_INVALID_ENUM;
    }
    blendSrcRGB = srcRGB;
    blendDstRGB = dstRGB;
    blendSrcAlpha = srcAlpha;
    blendDstAlpha = dstAlpha;
    return GL_NO_ERROR;
}

GLenum CommonState::Clear(GLbitfield mask) {
    const GLbitfield validBits = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
    if ((mask & ~validBits) != 0) {
        return GL_INVALID_VALUE;
    }
    clearMask = mask;
    return GL_NO_ERROR;
}

GLenum CommonState::ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    clearColor[0] = red;
    clearColor[1] = green;
    clearColor[2] = blue;
    clearColor[3] = alpha;
    return GL_NO_ERROR;
}

GLenum CommonState::ClearDepth(GLfloat depth) {
    clearDepth = depth;
    return GL_NO_ERROR;
}

GLenum CommonState::ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
    colorMask[0] = red;
    colorMask[1] = green;
    colorMask[2] = blue;
    colorMask[3] = alpha;
    return GL_NO_ERROR;
}

GLenum CommonState::DepthFunc(GLenum func) {
    if (MG_Constants::Depth::VALID_FUNCS.find(func) == MG_Constants::Depth::VALID_FUNCS.end()) {
        return GL_INVALID_ENUM;
    }
    depthFunc = func;
    return GL_NO_ERROR;
}

GLenum CommonState::DepthMask(GLboolean flag) {
    depthMask = flag;
    return GL_NO_ERROR;
}

GLenum CommonState::Viewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    if (width < 0 || height < 0) {
        return GL_INVALID_VALUE;
    }

    viewport[0] = x;
    viewport[1] = y;
    viewport[2] = width;
    viewport[3] = height;
    return GL_NO_ERROR;
}