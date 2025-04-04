//
// Created by BZLZHH on 2025/4/4.
//

#include "TextureState.h"

// TextureObject

bool TextureObject::IsImmutable() const {
    bool immutable = params.texPropertiesInt.count(GL_TEXTURE_IMMUTABLE_FORMAT);
    MG_Util::Debug::LogD("MG_State: Texture: TextureObject::IsImmutable returns %d", immutable);
    return immutable;
}

// TextureUnitState

void TextureUnitState::Bind(GLenum target, GLuint texture) {
    MG_Util::Debug::LogD("MG_State: Texture: TextureUnitState::Bind target=0x%x texture=%u", target, texture);
    boundTextures[target] = texture;
}

GLuint TextureUnitState::GetBoundTexture(GLenum target) {
    auto it = boundTextures.find(target);
    GLuint tex = (it != boundTextures.end()) ? it->second : 0;
    MG_Util::Debug::LogD("MG_State: Texture: TextureUnitState::GetBoundTexture target=0x%x returns %u", target, tex);
    return tex;
}

// TextureState

TextureState::TextureState() {
    textureUnits.resize(MG_Constants::Texture::MAX_TEXTURE_UNITS);
    MG_Util::Debug::LogD("MG_State: Texture: TextureState constructor, textureUnits=%zu", textureUnits.size());
}

TextureState &TextureState::GetInstance() {
    static TextureState instance;
    MG_Util::Debug::LogD("MG_State: Texture: GetInstance called");
    return instance;
}

bool TextureState::IsTextureGenerated(GLuint texture) {
    bool isValidTextureNumber = texture != 0;
    bool isValidTexture;
    bool generated = false;
    if (!isValidTextureNumber) {
        MG_Util::Debug::LogD("MG_State: Texture: IsTextureGenerated texture == 0");
    } else {
        isValidTexture = textures.find(texture) != textures.end();
        if (!isValidTexture) {
            MG_Util::Debug::LogD("MG_State: Texture: IsTextureGenerated invalid texture %d", texture);
        } else {
            generated = textures.at(texture).generated;
            if (!generated) {
                MG_Util::Debug::LogD("MG_State: Texture: IsTextureGenerated texture %d not generated", texture, generated);
            }
        }
    }
    MG_Util::Debug::LogD("MG_State: Texture: IsTextureGenerated texture=%u returns %d", texture, generated);
    return generated;
}

bool TextureState::IsTexture(GLuint texture) {
    bool isTex = texture != 0 && textures.find(texture) != textures.end();
    MG_Util::Debug::LogD("MG_State: Texture: IsTexture texture=%u returns %d", texture, isTex);
    return isTex;
}

GLenum TextureState::BindUnit(GLenum textureUnit) {
    MG_Util::Debug::LogD("MG_State: Texture: BindUnit textureUnit=0x%x", textureUnit);
    GLuint unitIndex = textureUnit - GL_TEXTURE0;
    if (unitIndex < textureUnits.size()) {
        activeTextureUnit = unitIndex;
        MG_Util::Debug::LogD("MG_State: Texture: BindUnit set activeTextureUnit=%u", activeTextureUnit);
    } else {
        MG_Util::Debug::LogE("MG_State: Texture: BindUnit invalid texture unit 0x%x", textureUnit);
        return GL_INVALID_ENUM;
    }
    return GL_NO_ERROR;
}

GLenum TextureState::Create(GLuint* texture) {
    MG_Util::Debug::LogD("MG_State: Texture: Create called");
    if (!texture) {
        MG_Util::Debug::LogE("MG_State: Texture: Create received null pointer");
        return GL_INVALID_VALUE;
    }
    return CreateN(1, texture);
}

GLenum TextureState::CreateN(GLsizei n, GLuint* textures) {
    MG_Util::Debug::LogD("MG_State: Texture: CreateN called with n=%d", n);
    if (n < 1) {
        MG_Util::Debug::LogE("MG_State: Texture: CreateN invalid n=%d", n);
        return GL_INVALID_VALUE;
    }

    for (GLsizei i = 0; i < n; ++i) {
        GLuint id = 0;
        if (!freeIDs.empty()) {
            id = *freeIDs.begin();
            freeIDs.erase(freeIDs.begin());
            MG_Util::Debug::LogD("MG_State: Texture: CreateN reusing free id=%u", id);
        } else {
            id = ++lastUsedID;
            MG_Util::Debug::LogD("MG_State: Texture: CreateN new id=%u", id);
        }
        TextureObject obj;
        obj.generated = true;
        this->textures[id] = obj;
        textures[i] = id;
    }
    return GL_NO_ERROR;
}

GLenum TextureState::Bind(GLenum target, GLuint texture) {
    MG_Util::Debug::LogD("MG_State: Texture: Bind called with target=0x%x, texture=%u", target, texture);
    if (MG_Constants::Texture::VALID_TARGETS.find(target) == MG_Constants::Texture::VALID_TARGETS.end()) {
        MG_Util::Debug::LogE("MG_State: Texture: Bind invalid target=0x%x", target);
        return GL_INVALID_ENUM;
    }
    
    if (texture == 0) {
        TextureUnitState& unit = textureUnits[activeTextureUnit];
        unit.Bind(target, 0);
        MG_Util::Debug::LogD("MG_State: Texture: Bind unbind succeeded for target=0x%x", target);
        return GL_NO_ERROR;
    }

    if (!IsTextureGenerated(texture)) {
        MG_Util::Debug::LogE("MG_State: Texture: Bind texture %u not generated", texture);
        return GL_INVALID_VALUE;
    }
    
    auto texObjIt = textures.find(texture);
    if(texObjIt != textures.end()){
        if (texObjIt->second.target != target && texObjIt->second.target != GL_NONE) {
            MG_Util::Debug::LogE("MG_State: Texture: Bind texture %u bound to different target 0x%x", texture, texObjIt->second.target);
            return GL_INVALID_OPERATION;
        }
    }
    

    TextureUnitState& unit = textureUnits[activeTextureUnit];
    TextureObject* texObj = nullptr;
    auto it = this->textures.find(texture);
    if (it == this->textures.end()) {
        MG_Util::Debug::LogW("MG_State: Texture: Bind texture %u not found, creating new (non-generated)", texture);
        TextureObject newObj;
        newObj.generated = false;
        this->textures[texture] = newObj;
        texObj = &this->textures[texture];
    } else {
        texObj = &it->second;
    }
    
    if (texObj->target == GL_NONE) {
        texObj->target = target;
        MG_Util::Debug::LogD("MG_State: Texture: Bind setting target=0x%x for texture %u", target, texture);
    } else if (texObj->target != target) {
        MG_Util::Debug::LogE("MG_State: Texture: Bind mismatch target for texture %u, expected 0x%x, got 0x%x", texture, texObj->target, target);
        return GL_INVALID_OPERATION;
    }

    unit.Bind(target, texture);
    MG_Util::Debug::LogD("MG_State: Texture: Bind succeeded for texture %u on target=0x%x", texture, target);

    return GL_NO_ERROR;
}

size_t TextureState::CalculatePixelDataSize(GLenum format, GLenum type, GLsizei width, GLsizei height) {
    MG_Util::Debug::LogD("MG_State: Texture: CalculatePixelDataSize called format=0x%x, type=0x%x, width=%d, height=%d", format, type, width, height);
    size_t bytesPerPixel = 0;
    switch (type) {
        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
            bytesPerPixel = 1;
            break;
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            bytesPerPixel = 2;
            break;
        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            bytesPerPixel = 4;
        default: {
            int components = 0;
            switch (format) {
                case GL_RED:              components = 1; break;
                case GL_RG:               components = 2; break;
                case GL_RGB:
                case GL_BGR:              components = 3; break;
                case GL_RGBA:
                case GL_BGRA:             components = 4; break;
                case GL_DEPTH_COMPONENT:  components = 1; break;
                case GL_DEPTH_STENCIL:    components = 2; break;
                default:                  components = 0; break;
            }
            size_t typeSize = 0;
            switch (type) {
                case GL_BYTE:
                case GL_UNSIGNED_BYTE:     typeSize = 1; break;
                case GL_SHORT:
                case GL_UNSIGNED_SHORT:    typeSize = 2; break;
                case GL_INT:
                case GL_UNSIGNED_INT:
                case GL_FLOAT:            typeSize = 4; break;
                default:                   typeSize = 0; break;
            }
            bytesPerPixel = components * typeSize;
            break;
        }
    }
    size_t totalSize = static_cast<size_t>(width) * static_cast<size_t>(height) * bytesPerPixel;
    MG_Util::Debug::LogD("MG_State: Texture: CalculatePixelDataSize returns %zu", totalSize);
    return totalSize;
}

GLenum TextureState::Upload2D(GLenum target, GLint level, GLint internalFormat,
                              GLsizei width, GLsizei height, GLint border, GLenum format,
                              GLenum type, const void* data) {
    MG_Util::Debug::LogD("MG_State: Texture: Upload2D called target=0x%x, level=%d, width=%d, height=%d", target, level, width, height);
    GLenum validity = CheckUploadingTexture2DValidity(target, level, internalFormat, width, height, border, format, type, data);
    if (validity != GL_NO_ERROR) {
        MG_Util::Debug::LogE("MG_State: Texture: Upload2D CheckUploadingTexture2DValidity failed with error 0x%x", validity);
        return validity;
    }
    
    bool isProxyTexture = (target == GL_PROXY_TEXTURE_1D || target == GL_PROXY_TEXTURE_2D ||
                           target == GL_PROXY_TEXTURE_3D || target == GL_PROXY_TEXTURE_CUBE_MAP ||
                           target == GL_PROXY_TEXTURE_1D_ARRAY || target == GL_PROXY_TEXTURE_2D_ARRAY ||
                           target == GL_PROXY_TEXTURE_RECTANGLE || target == GL_PROXY_TEXTURE_2D_MULTISAMPLE ||
                           target == GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY);
    if (isProxyTexture) {
        MG_Util::Debug::LogD("MG_State: Texture: Upload2D proxy texture detected");
        TextureObject& proxyTex = proxyTextures[target];
        TextureParams::MipmapLevel mip{};
        mip.width = width;
        mip.height = height;
        mip.internalFormat = internalFormat;
        mip.format = format;
        mip.type = type;
        proxyTex.generated = true;
        proxyTex.target = target;
        proxyTex.params.mipmapData[level] = mip;
        return GL_NO_ERROR;
    }
    
    GLuint boundTex = textureUnits[activeTextureUnit].GetBoundTexture(target);
    TextureObject& tex = textures[boundTex];
    TextureParams::MipmapLevel mip{};
    mip.width = width;
    mip.height = height;
    mip.internalFormat = internalFormat;
    mip.format = format;
    mip.type = type;
    if (data != nullptr) {
        size_t dataSize = CalculatePixelDataSize(format, type, width, height);
        mip.pixelData.resize(dataSize);
        memcpy(mip.pixelData.data(), data, dataSize);
        mip.hasData = true;
        MG_Util::Debug::LogD("MG_State: Texture: Upload2D uploaded data size=%zu", dataSize);
    } else {
        mip.hasData = false;
        MG_Util::Debug::LogD("MG_State: Texture: Upload2D data pointer is null");
    }
    tex.data = data;
    tex.params.mipmapData[level] = mip;
    MG_Util::Debug::LogD("MG_State: Texture: Upload2D succeeded for level=%d", level);

    return GL_NO_ERROR;
}

GLenum TextureState::UpdateRegion2D(GLenum target, GLint level, GLint xoffset,
                                    GLint yoffset, GLsizei width, GLsizei height, GLenum format,
                                    GLenum type, const GLvoid* data) {
    MG_Util::Debug::LogD("MG_State: Texture: UpdateRegion2D called target=0x%x, level=%d, x=%d, y=%d, w=%d, h=%d",
                         target, level, xoffset, yoffset, width, height);

    if (MG_Constants::Texture::VALID_TARGETS.find(target) == MG_Constants::Texture::VALID_TARGETS.end()) {
        MG_Util::Debug::LogE("MG_State: Texture: UpdateRegion2D invalid target=0x%x", target);
        return GL_INVALID_ENUM;
    }

    if (level < 0) {
        MG_Util::Debug::LogE("MG_State: Texture: UpdateRegion2D invalid level=%d", level);
        return GL_INVALID_VALUE;
    }

    if (xoffset < 0 || yoffset < 0) {
        MG_Util::Debug::LogE("MG_State: Texture: UpdateRegion2D negative offset x=%d, y=%d", xoffset, yoffset);
        return GL_INVALID_VALUE;
    }

    if (width < 0 || height < 0) {
        MG_Util::Debug::LogE("MG_State: Texture: UpdateRegion2D negative dimensions w=%d, h=%d", width, height);
        return GL_INVALID_VALUE;
    }

    if (!data) {
        // TODO: need to impl PBO and then impl here
        MG_Util::Debug::LogE("MG_State: Texture: UpdateRegion2D data pointer is null");
        return GL_INVALID_OPERATION ;
    }
    
    GLuint boundTex = textureUnits[activeTextureUnit].GetBoundTexture(target);
    if (boundTex == 0) {
        MG_Util::Debug::LogE("MG_State: Texture: UpdateRegion2D no texture bound for target=0x%x", target);
        return GL_INVALID_OPERATION;
    }

    if (!IsTextureGenerated(boundTex)) {
        MG_Util::Debug::LogE("MG_State: Texture: UpdateRegion2D texture %u not generated", boundTex);
        return GL_INVALID_OPERATION;
    }

    auto tex = textures[boundTex];

    if (tex.IsImmutable()) {
        MG_Util::Debug::LogE("MG_State: Texture: UpdateRegion2D texture is immutable");
        return GL_INVALID_OPERATION;
    }

    auto mipIt = tex.params.mipmapData.find(level);
    if (mipIt == tex.params.mipmapData.end()) {
        MG_Util::Debug::LogE("MG_State: Texture: UpdateRegion2D no mipmap level %d", level);
        return GL_INVALID_OPERATION;
    }

    TextureParams::MipmapLevel& mip = mipIt->second;

    if (MG_Constants::Texture::VALID_FORMATS.find(format) == MG_Constants::Texture::VALID_FORMATS.end()) {
        MG_Util::Debug::LogE("MG_State: Texture: UpdateRegion2D invalid format=0x%x", format);
        return GL_INVALID_ENUM;
    }

    if (MG_Constants::Texture::VALID_TYPES.find(type) == MG_Constants::Texture::VALID_TYPES.end()) {
        MG_Util::Debug::LogE("MG_State: Texture: UpdateRegion2D invalid type=0x%x", type);
        return GL_INVALID_ENUM;
    }

    bool typeNeedsRGB = (type == GL_UNSIGNED_BYTE_3_3_2 || type == GL_UNSIGNED_BYTE_2_3_3_REV ||
                         type == GL_UNSIGNED_SHORT_5_6_5 || type == GL_UNSIGNED_SHORT_5_6_5_REV);
    if (typeNeedsRGB && format != GL_RGB) {
        MG_Util::Debug::LogE("MG_State: Texture: UpdateRegion2D type requires RGB format");
        return GL_INVALID_OPERATION;
    }

    bool typeNeedsRGBA = (type == GL_UNSIGNED_SHORT_4_4_4_4 || type == GL_UNSIGNED_SHORT_4_4_4_4_REV ||
                          type == GL_UNSIGNED_SHORT_5_5_5_1 || type == GL_UNSIGNED_SHORT_1_5_5_5_REV ||
                          type == GL_UNSIGNED_INT_8_8_8_8 || type == GL_UNSIGNED_INT_8_8_8_8_REV ||
                          type == GL_UNSIGNED_INT_10_10_10_2 || type == GL_UNSIGNED_INT_2_10_10_10_REV);
    if (typeNeedsRGBA && format != GL_RGBA && format != GL_BGRA) {
        MG_Util::Debug::LogE("MG_State: Texture: UpdateRegion2D type requires RGBA/BGRA format");
        return GL_INVALID_OPERATION;
    }

    if (xoffset + width > mip.width || yoffset + height > mip.height) {
        MG_Util::Debug::LogE("MG_State: Texture: UpdateRegion2D region out of bounds: "
                             "x=%d+%d > %d or y=%d+%d > %d",
                             xoffset, width, mip.width, yoffset, height, mip.height);
        return GL_INVALID_VALUE;
    }

    bool isDepthInternal = (mip.internalFormat == GL_DEPTH_COMPONENT ||
                            mip.internalFormat == GL_DEPTH_COMPONENT16 ||
                            mip.internalFormat == GL_DEPTH_COMPONENT24 ||
                            mip.internalFormat == GL_DEPTH_COMPONENT32F ||
                            mip.internalFormat == GL_DEPTH_STENCIL ||
                            mip.internalFormat == GL_DEPTH24_STENCIL8 ||
                            mip.internalFormat == GL_DEPTH32F_STENCIL8);

    bool isDepthFormat = (format == GL_DEPTH_COMPONENT || format == GL_DEPTH_STENCIL);
    if (isDepthInternal != isDepthFormat) {
        MG_Util::Debug::LogE("MG_State: Texture: UpdateRegion2D format mismatch with internal format");
        return GL_INVALID_OPERATION;
    }

    mip.hasData = true;
    MG_Util::Debug::LogD("MG_State: Texture: UpdateRegion2D validated subimage update");

    tex.data = data;

    return GL_NO_ERROR;
}


GLenum TextureState::SetTexturePropertyFloat(GLenum target, GLenum pname, GLfloat param) {
    MG_Util::Debug::LogD("MG_State: Texture: SetTexturePropertyFloat called target=0x%x, pname=0x%x, param=%f", target, pname, param);
    if (MG_Constants::Texture::VALID_TARGETS.find(target) == MG_Constants::Texture::VALID_TARGETS.end()) {
        MG_Util::Debug::LogE("MG_State: Texture: SetTexturePropertyFloat invalid target=0x%x", target);
        return GL_INVALID_ENUM;
    }

    if (MG_Constants::Texture::VALID_TEXTURE_PARAM_NAMES.find(pname) == MG_Constants::Texture::VALID_TEXTURE_PARAM_NAMES.end()) {
        MG_Util::Debug::LogE("MG_State: Texture: SetTexturePropertyFloat invalid pname=0x%x", pname);
        return GL_INVALID_ENUM;
    }

    switch (pname) {
        case GL_TEXTURE_BORDER_COLOR:
            break;
        case GL_TEXTURE_MIN_LOD:
            if(param < -1000.0f){
                MG_Util::Debug::LogE("MG_State: Texture: SetTexturePropertyFloat param for GL_TEXTURE_MIN_LOD too low: %f", param);
                return GL_INVALID_VALUE;
            }
            break;
        case GL_TEXTURE_MAX_LOD:
            if(param > 1000.0f){
                MG_Util::Debug::LogE("MG_State: Texture: SetTexturePropertyFloat param for GL_TEXTURE_MAX_LOD too high: %f", param);
                return GL_INVALID_VALUE;
            }
            break;
        case GL_TEXTURE_LOD_BIAS:
            if(param < -1000.0f || param > 1000.0f) {
                MG_Util::Debug::LogE("MG_State: Texture: SetTexturePropertyFloat param for GL_TEXTURE_LOD_BIAS out of range: %f", param);
                return GL_INVALID_VALUE;
            }
            break;
        case GL_TEXTURE_SWIZZLE_R:
        case GL_TEXTURE_SWIZZLE_G:
        case GL_TEXTURE_SWIZZLE_B:
        case GL_TEXTURE_SWIZZLE_A:
            if (param != GL_ZERO && param != GL_ONE && param != GL_RED && param != GL_GREEN && param != GL_BLUE && param != GL_ALPHA) {
                MG_Util::Debug::LogE("MG_State: Texture: SetTexturePropertyFloat invalid swizzle param=%f", param);
                return GL_INVALID_ENUM;
            }
            break;
        default:
            break;
    }
    GLuint boundTex = textureUnits[activeTextureUnit].GetBoundTexture(target);
    TextureObject& tex = textures[boundTex];
    tex.params.texPropertiesFloat[pname] = param;
    MG_Util::Debug::LogD("MG_State: Texture: SetTexturePropertyFloat succeeded for texture %u", boundTex);
    return GL_NO_ERROR;
}

GLenum TextureState::SetTexturePropertyInt(GLenum target, GLenum pname, GLint param) {
    MG_Util::Debug::LogD("MG_State: Texture: SetTexturePropertyInt called target=0x%x, pname=0x%x, param=%d", target, pname, param);
    if (MG_Constants::Texture::VALID_TARGETS.find(target) == MG_Constants::Texture::VALID_TARGETS.end()) {
        MG_Util::Debug::LogE("MG_State: Texture: SetTexturePropertyInt invalid target=0x%x", target);
        return GL_INVALID_ENUM;
    }

    if (MG_Constants::Texture::VALID_TEXTURE_PARAM_NAMES.find(pname) == MG_Constants::Texture::VALID_TEXTURE_PARAM_NAMES.end()) {
        MG_Util::Debug::LogE("MG_State: Texture: SetTexturePropertyInt invalid pname=0x%x", pname);
        return GL_INVALID_ENUM;
    }

    switch (pname) {
        case GL_TEXTURE_MIN_FILTER:
        case GL_TEXTURE_MAG_FILTER:
            if (param != GL_NEAREST && param != GL_LINEAR && param != GL_NEAREST_MIPMAP_NEAREST &&
                param != GL_LINEAR_MIPMAP_NEAREST && param != GL_NEAREST_MIPMAP_LINEAR &&
                param != GL_LINEAR_MIPMAP_LINEAR) {
                MG_Util::Debug::LogE("MG_State: Texture: SetTexturePropertyInt invalid filter param=%d", param);
                return GL_INVALID_ENUM;
            }
            break;
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
        case GL_TEXTURE_WRAP_R:
            if (param != GL_CLAMP_TO_EDGE && param != GL_CLAMP_TO_BORDER &&
                param != GL_MIRRORED_REPEAT && param != GL_REPEAT) {
                MG_Util::Debug::LogE("MG_State: Texture: SetTexturePropertyInt invalid wrap param=%d", param);
                return GL_INVALID_ENUM;
            }
            break;
        case GL_TEXTURE_SWIZZLE_R:
        case GL_TEXTURE_SWIZZLE_G:
        case GL_TEXTURE_SWIZZLE_B:
        case GL_TEXTURE_SWIZZLE_A:
            if(param != GL_ZERO && param != GL_ONE && param != GL_RED && param != GL_GREEN && param != GL_BLUE && param != GL_ALPHA){
                MG_Util::Debug::LogE("MG_State: Texture: SetTexturePropertyInt invalid swizzle param=%d", param);
                return GL_INVALID_ENUM;
            }
            break;
        default:
            break;
    }

    GLuint boundTex = textureUnits[activeTextureUnit].GetBoundTexture(target);
    TextureObject& tex = textures[boundTex];
    tex.params.texPropertiesInt[pname] = param;
    MG_Util::Debug::LogD("MG_State: Texture: SetTexturePropertyInt succeeded for texture %u", boundTex);
    return GL_NO_ERROR;
}

GLenum TextureState::Delete(GLuint texture) {
    MG_Util::Debug::LogD("MG_State: Texture: Delete called for texture=%u", texture);
    if (!texture) {
        MG_Util::Debug::LogE("MG_State: Texture: Delete received texture 0");
        return GL_INVALID_VALUE;
    }

    auto it = this->textures.find(texture);
    if (it != this->textures.end()) {
        InvalidateTextureInAllUnits(texture);
        this->textures.erase(it);
        freeIDs.insert(texture);
        MG_Util::Debug::LogD("MG_State: Texture: Delete succeeded for texture=%u", texture);
    } else {
        MG_Util::Debug::LogW("MG_State: Texture: Delete texture %u not found", texture);
    }
    return GL_NO_ERROR;
}

GLenum TextureState::DeleteN(GLsizei n, const GLuint* textures) {
    MG_Util::Debug::LogD("MG_State: Texture: DeleteN called with n=%d", n);
    if (n < 1) {
        MG_Util::Debug::LogE("MG_State: Texture: DeleteN invalid n=%d", n);
        return GL_INVALID_VALUE;
    }

    for (GLsizei i = 0; i < n; ++i) {
        GLuint id = textures[i];
        if (id == 0) continue;
        auto it = this->textures.find(id);
        if (it != this->textures.end()) {
            InvalidateTextureInAllUnits(id);
            this->textures.erase(it);
            freeIDs.insert(id);
            MG_Util::Debug::LogD("MG_State: Texture: DeleteN deleted texture=%u", id);
        } else {
            MG_Util::Debug::LogW("MG_State: Texture: DeleteN texture %u not found", id);
        }
    }

    return GL_NO_ERROR;
}


GLenum TextureState::GetLevelPropertyIntVector(GLenum target, GLint level, GLenum pname, GLint* params) {
    MG_Util::Debug::LogD("MG_State: Texture: GetLevelPropertyIntVector called target=0x%x, level=%d, pname=0x%x", target, level, pname);
    if (MG_Constants::Texture::VALID_TARGETS.find(target) == MG_Constants::Texture::VALID_TARGETS.end()) {
        MG_Util::Debug::LogE("MG_State: Texture: GetLevelPropertyIntVector invalid target=0x%x", target);
        return GL_INVALID_ENUM;
    }

    if (level < 0) {
        MG_Util::Debug::LogE("MG_State: Texture: GetLevelPropertyIntVector invalid level=%d", level);
        return GL_INVALID_VALUE;
    }

    if (MG_Constants::Texture::VALID_QUERY_LEVEL_PROPERTY_PARAM_NAMES.find(pname) == MG_Constants::Texture::VALID_QUERY_LEVEL_PROPERTY_PARAM_NAMES.end()) {
        MG_Util::Debug::LogE("MG_State: Texture: GetLevelPropertyIntVector invalid pname=0x%x", pname);
        return GL_INVALID_ENUM;
    }

    if ((target == GL_TEXTURE_RECTANGLE || target == GL_PROXY_TEXTURE_RECTANGLE) && level != 0) {
        MG_Util::Debug::LogE("MG_State: Texture: GetLevelPropertyIntVector invalid level for rectangle texture");
        return GL_INVALID_VALUE;
    }

    GLuint activeUnit = activeTextureUnit;
    if (activeUnit >= textureUnits.size()) {
        MG_Util::Debug::LogE("MG_State: Texture: GetLevelPropertyIntVector active texture unit out of range");
        return GL_INVALID_OPERATION;
    }
    
    bool isProxyTexture = (target == GL_PROXY_TEXTURE_1D || target == GL_PROXY_TEXTURE_2D ||
                           target == GL_PROXY_TEXTURE_3D || target == GL_PROXY_TEXTURE_CUBE_MAP ||
                           target == GL_PROXY_TEXTURE_1D_ARRAY || target == GL_PROXY_TEXTURE_2D_ARRAY ||
                           target == GL_PROXY_TEXTURE_RECTANGLE || target == GL_PROXY_TEXTURE_2D_MULTISAMPLE ||
                           target == GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY);
    
    TextureUnitState& unit = textureUnits[activeUnit];
    GLuint boundTexture = unit.GetBoundTexture(target);

    if (!isProxyTexture && boundTexture == 0) {
        switch (pname) {
            case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
                MG_Util::Debug::LogE("MG_State: Texture: GetLevelPropertyIntVector no texture bound for compressed image size");
                return GL_INVALID_OPERATION;
            case GL_TEXTURE_INTERNAL_FORMAT:
                *params = GL_NONE;
                break;
            case GL_TEXTURE_COMPRESSED:
                *params = GL_FALSE;
                break;
            default:
                *params = 0;
                break;
        }
        MG_Util::Debug::LogD("MG_State: Texture: GetLevelPropertyIntVector returns default value for unbound texture");
        return GL_NO_ERROR;
    }

    std::unordered_map<GLuint, TextureObject>::iterator texIt;
    
    if (!isProxyTexture) {
        texIt = textures.find(boundTexture);
        if (texIt == textures.end()) {
            *params = 0;
            MG_Util::Debug::LogW(
                    "MG_State: Texture: GetLevelPropertyIntVector texture %u not found",
                    boundTexture);
            return GL_NO_ERROR;
        }
    }
    
    TextureObject& tex = isProxyTexture ? proxyTextures[target] : texIt->second;
    
    if (!isProxyTexture && tex.target != target && tex.target != GL_NONE) {
        MG_Util::Debug::LogE("MG_State: Texture: GetLevelPropertyIntVector texture target mismatch: texture target=0x%x, expected=0x%x", tex.target, target);
        return GL_INVALID_OPERATION;
    }
    
    auto mipIt = tex.params.mipmapData.find(level);
    if (mipIt == tex.params.mipmapData.end()) {
        switch (pname) {
            case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
                MG_Util::Debug::LogE("MG_State: Texture: GetLevelPropertyIntVector no mipmap level %d for compressed image size", level);
                return GL_INVALID_OPERATION;
            case GL_TEXTURE_INTERNAL_FORMAT:
                *params = GL_NONE;
                break;
            case GL_TEXTURE_COMPRESSED:
                *params = GL_FALSE;
                break;
            default:
                *params = 0;
                break;
        }
        MG_Util::Debug::LogD("MG_State: Texture: GetLevelPropertyIntVector returns default value for missing mipmap level");
        return GL_NO_ERROR;
    }

    const TextureParams::MipmapLevel& mip = mipIt->second;
    ComponentSizes sizes = GetComponentSizes(mip.internalFormat);

    switch (pname) {
        case GL_TEXTURE_WIDTH:
            *params = mip.width;
            break;
        case GL_TEXTURE_HEIGHT:
            *params = mip.height;
            break;
        case GL_TEXTURE_DEPTH:
            *params = 1; // Assuming 2D Texture
            break;
        case GL_TEXTURE_INTERNAL_FORMAT:
            *params = mip.internalFormat;
            break;
        case GL_TEXTURE_RED_SIZE:
        case GL_TEXTURE_GREEN_SIZE:
        case GL_TEXTURE_BLUE_SIZE:
        case GL_TEXTURE_ALPHA_SIZE:
        case GL_TEXTURE_DEPTH_SIZE:
        case GL_TEXTURE_STENCIL_SIZE:
            switch (pname) {
                case GL_TEXTURE_RED_SIZE: *params = sizes.red; break;
                case GL_TEXTURE_GREEN_SIZE: *params = sizes.green; break;
                case GL_TEXTURE_BLUE_SIZE: *params = sizes.blue; break;
                case GL_TEXTURE_ALPHA_SIZE: *params = sizes.alpha; break;
                case GL_TEXTURE_DEPTH_SIZE: *params = sizes.depth; break;
                case GL_TEXTURE_STENCIL_SIZE: *params = sizes.stencil; break;
                default: break;
            }
            break;
        case GL_TEXTURE_COMPRESSED:
            *params = sizes.isCompressed ? GL_TRUE : GL_FALSE;
            break;
        case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
            if (!sizes.isCompressed) {
                MG_Util::Debug::LogE("MG_State: Texture: GetLevelPropertyIntVector texture is not compressed");
                return GL_INVALID_OPERATION;
            }
            *params = static_cast<GLint>(mip.pixelData.size());
            break;
        default:
            MG_Util::Debug::LogE("MG_State: Texture: GetLevelPropertyIntVector invalid pname=0x%x", pname);
            return GL_INVALID_ENUM;
    }
    MG_Util::Debug::LogD("MG_State: Texture: GetLevelPropertyIntVector returns %d for pname=0x%x", *params, pname);
    return GL_NO_ERROR;
}

void TextureState::InvalidateTextureInAllUnits(GLuint texture) {
    MG_Util::Debug::LogD("MG_State: Texture: InvalidateTextureInAllUnits called for texture=%u", texture);
    for (auto& unit : textureUnits) {
        for (auto& [target, tex] : unit.boundTextures) {
            if (tex == texture) {
                tex = 0;
                MG_Util::Debug::LogD("MG_State: Texture: InvalidateTextureInAllUnits unset texture for target=0x%x", target);
            }
        }
    }
}

GLenum TextureState::CheckUploadingTexture2DValidity(GLenum target, GLint level, GLint internalFormat,
                                                     GLsizei width, GLsizei height, GLint border, GLenum format,
                                                     GLenum type, const void* data) {
    MG_Util::Debug::LogD("MG_State: Texture: CheckUploadingTexture2DValidity called target=0x%x, level=%d", target, level);
    if (MG_Constants::Texture::VALID_TARGETS.find(target) == MG_Constants::Texture::VALID_TARGETS.end()) {
        MG_Util::Debug::LogE("MG_State: Texture: CheckUploadingTexture2DValidity invalid target=0x%x", target);
        return GL_INVALID_ENUM;
    }

    if (level < 0) {
        MG_Util::Debug::LogE("MG_State: Texture: CheckUploadingTexture2DValidity invalid level=%d", level);
        return GL_INVALID_VALUE;
    }

    if ((target == GL_TEXTURE_RECTANGLE || target == GL_PROXY_TEXTURE_RECTANGLE) && level != 0) {
        MG_Util::Debug::LogE("MG_State: Texture: CheckUploadingTexture2DValidity invalid level for rectangle texture");
        return GL_INVALID_VALUE;
    }

    if (width < 0 || height < 0) {
        MG_Util::Debug::LogE("MG_State: Texture: CheckUploadingTexture2DValidity invalid dimensions width=%d, height=%d", width, height);
        return GL_INVALID_VALUE;
    }

    if ((target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z) && width != height) {
        MG_Util::Debug::LogE("MG_State: Texture: CheckUploadingTexture2DValidity cube map requires equal width and height");
        return GL_INVALID_ENUM;
    }

    if (width > MG_Constants::Texture::MAX_TEXTURE_SIZE) {
        MG_Util::Debug::LogE("MG_State: Texture: CheckUploadingTexture2DValidity width exceeds maximum: %d", width);
        return GL_INVALID_VALUE;
    }

    if (target == GL_TEXTURE_1D_ARRAY || target == GL_PROXY_TEXTURE_1D_ARRAY) {
        if (height > MG_Constants::Texture::MAX_ARRAY_LAYERS) {
            MG_Util::Debug::LogE("MG_State: Texture: CheckUploadingTexture2DValidity height exceeds MAX_ARRAY_LAYERS: %d", height);
            return GL_INVALID_VALUE;
        }
    } else if (height > MG_Constants::Texture::MAX_TEXTURE_SIZE) {
        MG_Util::Debug::LogE("MG_State: Texture: CheckUploadingTexture2DValidity height exceeds maximum: %d", height);
        return GL_INVALID_VALUE;
    }

    if (MG_Constants::Texture::VALID_INTERNAL_FORMATS.find(internalFormat) == MG_Constants::Texture::VALID_INTERNAL_FORMATS.end()) {
        MG_Util::Debug::LogE("MG_State: Texture: CheckUploadingTexture2DValidity invalid internalFormat=0x%x", internalFormat);
        return GL_INVALID_VALUE;
    }

    if (MG_Constants::Texture::VALID_FORMATS.find(format) == MG_Constants::Texture::VALID_FORMATS.end()) {
        MG_Util::Debug::LogE("MG_State: Texture: CheckUploadingTexture2DValidity invalid format=0x%x", format);
        return GL_INVALID_ENUM;
    }

    if (MG_Constants::Texture::VALID_TYPES.find(type) == MG_Constants::Texture::VALID_TYPES.end()) {
        MG_Util::Debug::LogE("MG_State: Texture: CheckUploadingTexture2DValidity invalid type=0x%x", type);
        return GL_INVALID_ENUM;
    }

    bool typeNeedsRGB = (type == GL_UNSIGNED_BYTE_3_3_2 || type == GL_UNSIGNED_SHORT_5_6_5);
    if (typeNeedsRGB && format != GL_RGB) {
        MG_Util::Debug::LogE("MG_State: Texture: CheckUploadingTexture2DValidity type requires RGB but format=0x%x", format);
        return GL_INVALID_OPERATION;
    }

    bool typeNeedsRGBA = (type == GL_UNSIGNED_SHORT_4_4_4_4 || type == GL_UNSIGNED_INT_8_8_8_8);
    if (typeNeedsRGBA && format != GL_RGBA && format != GL_BGRA) {
        MG_Util::Debug::LogE("MG_State: Texture: CheckUploadingTexture2DValidity type requires RGBA but format=0x%x", format);
        return GL_INVALID_OPERATION;
    }

    bool isDepthInternal = (internalFormat == GL_DEPTH_COMPONENT32F || internalFormat == GL_DEPTH_COMPONENT24
                            || internalFormat == GL_DEPTH_COMPONENT16 || internalFormat == GL_DEPTH32F_STENCIL8
                            || internalFormat == GL_DEPTH24_STENCIL8 || internalFormat == GL_DEPTH_COMPONENT
                            || internalFormat == GL_DEPTH_STENCIL);
    bool isDepthFormat = (format == GL_DEPTH_COMPONENT || format == GL_DEPTH_STENCIL);
    if (isDepthInternal != isDepthFormat) {
        MG_Util::Debug::LogE("MG_State: Texture: CheckUploadingTexture2DValidity depth internal/format mismatch");
        return GL_INVALID_OPERATION;
    }

    if (isDepthInternal && (target != GL_TEXTURE_2D && target != GL_PROXY_TEXTURE_2D &&
                            target != GL_TEXTURE_RECTANGLE && target != GL_PROXY_TEXTURE_RECTANGLE)) {
        MG_Util::Debug::LogE("MG_State: Texture: CheckUploadingTexture2DValidity invalid target for depth texture");
        return GL_INVALID_OPERATION;
    }

    bool isProxyTexture = (target == GL_PROXY_TEXTURE_1D || target == GL_PROXY_TEXTURE_2D ||
                           target == GL_PROXY_TEXTURE_3D || target == GL_PROXY_TEXTURE_CUBE_MAP ||
                           target == GL_PROXY_TEXTURE_1D_ARRAY || target == GL_PROXY_TEXTURE_2D_ARRAY ||
                           target == GL_PROXY_TEXTURE_RECTANGLE || target == GL_PROXY_TEXTURE_2D_MULTISAMPLE ||
                           target == GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY);
    if (!isProxyTexture) {
        GLuint boundTex = textureUnits[activeTextureUnit].GetBoundTexture(target);
        if (boundTex == 0) {
            MG_Util::Debug::LogE(
                    "MG_State: Texture: CheckUploadingTexture2DValidity no texture bound for target=0x%x",
                    target);
            return GL_INVALID_OPERATION;
        }

        TextureObject &tex = textures[boundTex];
        if (tex.target != target) {
            MG_Util::Debug::LogE(
                    "MG_State: Texture: CheckUploadingTexture2DValidity texture target mismatch: texture target=0x%x, expected=0x%x",
                    tex.target, target);
            return GL_INVALID_OPERATION;
        }

        if (tex.IsImmutable()) {
            MG_Util::Debug::LogE(
                    "MG_State: Texture: CheckUploadingTexture2DValidity texture is immutable");
            return GL_INVALID_OPERATION;
        }
    }
    
    MG_Util::Debug::LogD("MG_State: Texture: CheckUploadingTexture2DValidity succeeded");
    return GL_NO_ERROR;
}

ComponentSizes TextureState::GetComponentSizes(GLenum internalFormat) {
    MG_Util::Debug::LogD("MG_State: Texture: GetComponentSizes called for internalFormat=0x%x", internalFormat);
    ComponentSizes sizes = {0};
    switch (internalFormat) {
        // Base formats
        case GL_R8:
        case GL_R8_SNORM: sizes.red = 8; break;
        case GL_R16:
        case GL_R16_SNORM:
        case GL_R16F: sizes.red = 16; break;
        case GL_R32F: sizes.red = 32; break;
        case GL_R8I:
        case GL_R8UI: sizes.red = 8; break;
        case GL_R16I:
        case GL_R16UI: sizes.red = 16; break;
        case GL_R32I:
        case GL_R32UI: sizes.red = 32; break;

        case GL_RG8:
        case GL_RG8_SNORM: sizes.red = 8; sizes.green = 8; break;
        case GL_RG16:
        case GL_RG16_SNORM:
        case GL_RG16F: sizes.red = 16; sizes.green = 16; break;
        case GL_RG32F: sizes.red = 32; sizes.green = 32; break;
        case GL_RG8I:
        case GL_RG8UI: sizes.red = 8; sizes.green = 8; break;
        case GL_RG16I:
        case GL_RG16UI: sizes.red = 16; sizes.green = 16; break;
        case GL_RG32I:
        case GL_RG32UI: sizes.red = 32; sizes.green = 32; break;

        case GL_RGB8:
        case GL_RGB8_SNORM: sizes.red = 8; sizes.green = 8; sizes.blue = 8; break;
        case GL_RGB16:
        case GL_RGB16_SNORM:
        case GL_RGB16F: sizes.red = 16; sizes.green = 16; sizes.blue = 16; break;
        case GL_RGB32F: sizes.red = 32; sizes.green = 32; sizes.blue = 32; break;
        case GL_RGB8I:
        case GL_RGB8UI: sizes.red = 8; sizes.green = 8; sizes.blue = 8; break;
        case GL_RGB16I:
        case GL_RGB16UI: sizes.red = 16; sizes.green = 16; sizes.blue = 16; break;
        case GL_RGB32I:
        case GL_RGB32UI: sizes.red = 32; sizes.green = 32; sizes.blue = 32; break;

        case GL_RGBA8:
        case GL_RGBA8_SNORM: sizes.red = 8; sizes.green = 8; sizes.blue = 8; sizes.alpha = 8; break;
        case GL_RGBA16:
        case GL_RGBA16_SNORM:
        case GL_RGBA16F: sizes.red = 16; sizes.green = 16; sizes.blue = 16; sizes.alpha = 16; break;
        case GL_RGBA32F: sizes.red = 32; sizes.green = 32; sizes.blue = 32; sizes.alpha = 32; break;
        case GL_RGBA8I:
        case GL_RGBA8UI: sizes.red = 8; sizes.green = 8; sizes.blue = 8; sizes.alpha = 8; break;
        case GL_RGBA16I:
        case GL_RGBA16UI: sizes.red = 16; sizes.green = 16; sizes.blue = 16; sizes.alpha = 16; break;
        case GL_RGBA32I:
        case GL_RGBA32UI: sizes.red = 32; sizes.green = 32; sizes.blue = 32; sizes.alpha = 32; break;

            // Packed formats
        case GL_RGB565: sizes.red = 5; sizes.green = 6; sizes.blue = 5; break;
        case GL_RGBA4: sizes.red = 4; sizes.green = 4; sizes.blue = 4; sizes.alpha = 4; break;
        case GL_RGB5_A1: sizes.red = 5; sizes.green = 5; sizes.blue = 5; sizes.alpha = 1; break;
        case GL_RGB10_A2:
        case GL_RGB10_A2UI: sizes.red = 10; sizes.green = 10; sizes.blue = 10; sizes.alpha = 2; break;
        case GL_R11F_G11F_B10F: sizes.red = 11; sizes.green = 11; sizes.blue = 10; break;
        case GL_RGB9_E5: sizes.red = 9; sizes.green = 9; sizes.blue = 9; break; // shared exponent

            // sRGB formats
        case GL_SRGB8: sizes.red = 8; sizes.green = 8; sizes.blue = 8; break;
        case GL_SRGB8_ALPHA8: sizes.red = 8; sizes.green = 8; sizes.blue = 8; sizes.alpha = 8; break;

            // Depth/stencil formats
        case GL_DEPTH_COMPONENT16: sizes.depth = 16; break;
        case GL_DEPTH_COMPONENT24: sizes.depth = 24; break;
        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT32F: sizes.depth = 32; break;
        case GL_DEPTH24_STENCIL8: sizes.depth = 24; sizes.stencil = 8; break;
        case GL_DEPTH32F_STENCIL8: sizes.depth = 32; sizes.stencil = 8; break;

            // Compressed formats
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            sizes.isCompressed = true;
            sizes.red = 5; sizes.green = 6; sizes.blue = 5;
            if (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) sizes.alpha = 1;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_RG_RGTC2:
        case GL_COMPRESSED_SIGNED_RG_RGTC2:
        case GL_COMPRESSED_RED_RGTC1:
        case GL_COMPRESSED_SIGNED_RED_RGTC1:
            sizes.isCompressed = true;
            break;

        default:
            if (internalFormat == GL_RGBA || internalFormat == GL_BGRA ||
                internalFormat == GL_RGBA_INTEGER || internalFormat == GL_BGRA_INTEGER) {
                // Typically matches RGBA8 when no size is specified
                sizes.red = 8; sizes.green = 8; sizes.blue = 8; sizes.alpha = 8;
            } else if (internalFormat == GL_RGB || internalFormat == GL_BGR ||
                       internalFormat == GL_RGB_INTEGER || internalFormat == GL_BGR_INTEGER) {
                sizes.red = 8; sizes.green = 8; sizes.blue = 8;
            } else if (internalFormat == GL_RG || internalFormat == GL_RG_INTEGER) {
                sizes.red = 8; sizes.green = 8;
            } else if (internalFormat == GL_RED || internalFormat == GL_RED_INTEGER) {
                sizes.red = 8;
            } else if (internalFormat == GL_DEPTH_COMPONENT) {
                sizes.depth = 32; // Common default
            } else if (internalFormat == GL_DEPTH_STENCIL) {
                sizes.depth = 32; sizes.stencil = 8;
            }
            break;
    }
    MG_Util::Debug::LogD("MG_State: Texture: GetComponentSizes returns (red=%d, green=%d, blue=%d, alpha=%d, depth=%d, stencil=%d, isCompressed=%d)",
                         sizes.red, sizes.green, sizes.blue, sizes.alpha, sizes.depth, sizes.stencil, sizes.isCompressed);
    return sizes;
}
