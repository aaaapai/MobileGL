//
// Created by BZLZHH on 2025/4/30.
//

#include "CommonState.h"

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
