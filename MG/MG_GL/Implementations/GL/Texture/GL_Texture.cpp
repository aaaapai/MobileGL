//
// Created by BZLZHH on 2025/3/15.
//

#include "../../../../Includes.h"

namespace MG_GL::GL {
  
    Diligent::TEXTURE_FORMAT ConvertInternalFormat(GLint internalFormat) {
        switch (internalFormat) {
            case GL_RGBA: return Diligent::TEX_FORMAT_RGBA8_UNORM;
            case GL_RGBA8: return Diligent::TEX_FORMAT_RGBA8_UNORM;
            case GL_RGBA8_SNORM: return Diligent::TEX_FORMAT_RGBA8_SNORM;
            case GL_RGBA16F: return Diligent::TEX_FORMAT_RGBA16_FLOAT;
            case GL_RGBA32F: return Diligent::TEX_FORMAT_RGBA32_FLOAT;

            case GL_RGB: return Diligent::TEX_FORMAT_RGBA8_UNORM;
            case GL_RGB8: return Diligent::TEX_FORMAT_RGBA8_UNORM; // Diligent doesn't have RGB8, promote to RGBA8
            case GL_RGB16F: return Diligent::TEX_FORMAT_RGBA16_FLOAT; // Promote
            case GL_RGB32F: return Diligent::TEX_FORMAT_RGBA32_FLOAT; // Promote

            case GL_DEPTH_COMPONENT: return Diligent::TEX_FORMAT_D32_FLOAT;
            case GL_DEPTH_COMPONENT16: return Diligent::TEX_FORMAT_D16_UNORM;
            case GL_DEPTH_COMPONENT24: return Diligent::TEX_FORMAT_D24_UNORM_S8_UINT; // No D24_UNORM, use with stencil
            case GL_DEPTH_COMPONENT32: return Diligent::TEX_FORMAT_D32_FLOAT;
            case GL_DEPTH_COMPONENT32F: return Diligent::TEX_FORMAT_D32_FLOAT;

            case GL_DEPTH_STENCIL: return Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
            case GL_DEPTH24_STENCIL8: return Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
            case GL_DEPTH32F_STENCIL8: return Diligent::TEX_FORMAT_D32_FLOAT_S8X24_UINT;

            case GL_RED: return Diligent::TEX_FORMAT_R8_UNORM;
            case GL_R8: return Diligent::TEX_FORMAT_R8_UNORM;
            case GL_R16F: return Diligent::TEX_FORMAT_R16_FLOAT;
            case GL_R32F: return Diligent::TEX_FORMAT_R32_FLOAT;

            case GL_RG: return Diligent::TEX_FORMAT_RG8_UNORM;
            case GL_RG8: return Diligent::TEX_FORMAT_RG8_UNORM;
            case GL_RG16F: return Diligent::TEX_FORMAT_RG16_FLOAT;
            case GL_RG32F: return Diligent::TEX_FORMAT_RG32_FLOAT;

            default: return Diligent::TEX_FORMAT_RGBA8_UNORM;
        }
    }
  
    size_t GetBytesPerPixel(GLenum format, GLenum type) {
        int components = 0;
        switch (format) {
            case GL_RED: components = 1; break;
            case GL_RG: components = 2; break;
            case GL_RGB: components = 3; break;
            case GL_RGBA: components = 4; break;
            case GL_DEPTH_COMPONENT: components = 1; break;
            case GL_DEPTH_STENCIL: components = 2; break;
            default: components = 4; break;
        }

        size_t typeSize = 0;
        switch (type) {
            case GL_UNSIGNED_BYTE:
            case GL_BYTE: typeSize = 1; break;
            case GL_UNSIGNED_SHORT:
            case GL_SHORT: typeSize = 2; break;
            case GL_UNSIGNED_INT:
            case GL_INT:
            case GL_FLOAT: typeSize = 4; break;
            default: typeSize = 1; break;
        }

        return components * typeSize;
    }

    Diligent::FILTER_TYPE ConvertCompareFilter(Diligent::FILTER_TYPE& filter, bool compare) {
        if (compare) {
            switch (filter) {
                case Diligent::FILTER_TYPE_POINT: return Diligent::FILTER_TYPE_COMPARISON_POINT;
                case Diligent::FILTER_TYPE_LINEAR: return Diligent::FILTER_TYPE_COMPARISON_LINEAR;
                case Diligent::FILTER_TYPE_ANISOTROPIC: return Diligent::FILTER_TYPE_COMPARISON_ANISOTROPIC;
                default: return filter;
            }
        } else {
            switch (filter) {
                case Diligent::FILTER_TYPE_COMPARISON_POINT: return Diligent::FILTER_TYPE_POINT;
                case Diligent::FILTER_TYPE_COMPARISON_LINEAR: return Diligent::FILTER_TYPE_LINEAR;
                case Diligent::FILTER_TYPE_COMPARISON_ANISOTROPIC: return Diligent::FILTER_TYPE_ANISOTROPIC;
                default: return filter;
            }
        }
    }

    bool IsCompareFilter(Diligent::FILTER_TYPE filter) {
        switch (filter) {
            case Diligent::FILTER_TYPE_COMPARISON_POINT: return true;
            case Diligent::FILTER_TYPE_COMPARISON_LINEAR: return true;
            case Diligent::FILTER_TYPE_COMPARISON_ANISOTROPIC: return true;
            default: return false;
        }
    }

    void FillFilterTypeFromGLEnum(GLenum pname, GLenum param, Diligent::FILTER_TYPE& minFilter, Diligent::FILTER_TYPE& magFilter, Diligent::FILTER_TYPE& mipFilter) {
        if (pname == GL_TEXTURE_COMPARE_MODE) {
            bool compare = (param == GL_COMPARE_REF_TO_TEXTURE);
            minFilter = ConvertCompareFilter(minFilter, compare);
            magFilter = ConvertCompareFilter(magFilter, compare);
            mipFilter = ConvertCompareFilter(mipFilter, compare);
            return;
        }

        Diligent::FILTER_TYPE* pFilter = nullptr;
        if (pname == GL_TEXTURE_MIN_FILTER) {
            pFilter = &minFilter;
        } else if (pname == GL_TEXTURE_MAG_FILTER) {
            pFilter = &magFilter;
        } else {
            MG_Util::Debug::LogD("FillFilterTypeFromGLEnum: not allowed pname!");
            return;
        }

        Diligent::FILTER_TYPE& filter = *pFilter;

        bool filterIsCompare = IsCompareFilter(filter);
        bool mipFilterIsCompare = IsCompareFilter(mipFilter);

        switch (param) {
            case GL_NEAREST:
                filter = Diligent::FILTER_TYPE_POINT;
                if (pname == GL_TEXTURE_MIN_FILTER)
                    mipFilter = Diligent::FILTER_TYPE_POINT;
                break;
            case GL_LINEAR:
                filter = Diligent::FILTER_TYPE_LINEAR;
                if (pname == GL_TEXTURE_MIN_FILTER)
                    mipFilter = Diligent::FILTER_TYPE_POINT;
                break;
            case GL_NEAREST_MIPMAP_NEAREST:
                filter = Diligent::FILTER_TYPE_POINT;
                mipFilter = Diligent::FILTER_TYPE_POINT;
                break;
            case GL_LINEAR_MIPMAP_NEAREST:
                filter = Diligent::FILTER_TYPE_LINEAR;
                mipFilter = Diligent::FILTER_TYPE_POINT;
                break;
            case GL_NEAREST_MIPMAP_LINEAR:
                filter = Diligent::FILTER_TYPE_POINT;
                mipFilter = Diligent::FILTER_TYPE_LINEAR;
                break;
            case GL_LINEAR_MIPMAP_LINEAR:
                filter = Diligent::FILTER_TYPE_LINEAR;
                mipFilter = Diligent::FILTER_TYPE_LINEAR;
                break;
            default:
                break;
        }

        filter = ConvertCompareFilter(filter, filterIsCompare);
        mipFilter = ConvertCompareFilter(mipFilter, mipFilterIsCompare);
    }

    Diligent::COMPARISON_FUNCTION ConvertComparsionFunc(GLint param) {
        switch (param) {
            case GL_LEQUAL: return Diligent::COMPARISON_FUNC_LESS_EQUAL;
            case GL_GEQUAL: return Diligent::COMPARISON_FUNC_GREATER_EQUAL;
            case GL_LESS: return Diligent::COMPARISON_FUNC_LESS;
            case GL_GREATER: return Diligent::COMPARISON_FUNC_GREATER;
            case GL_EQUAL: return Diligent::COMPARISON_FUNC_EQUAL;
            case GL_NOTEQUAL: return Diligent::COMPARISON_FUNC_NOT_EQUAL;
            case GL_ALWAYS: return Diligent::COMPARISON_FUNC_ALWAYS;
            case GL_NEVER: return Diligent::COMPARISON_FUNC_NEVER;
            default:
                MG_Util::Debug::LogE("%s: not handled param: %s", __func__, MG_Util::Debug::GLEnumToString(param));
                return Diligent::COMPARISON_FUNC_NEVER;
        }
    }

    Diligent::TEXTURE_ADDRESS_MODE ConvertAddressMode(GLint param) {
        switch (param) {
            case GL_REPEAT: return Diligent::TEXTURE_ADDRESS_WRAP;
            case GL_CLAMP_TO_EDGE: return Diligent::TEXTURE_ADDRESS_CLAMP;
            case GL_MIRRORED_REPEAT: return Diligent::TEXTURE_ADDRESS_MIRROR;
            default:
                MG_Util::Debug::LogE("%s: not handled param: %s", __func__, MG_Util::Debug::GLEnumToString(param));
                return Diligent::TEXTURE_ADDRESS_WRAP;
        }
    }
  
    void ActiveTexture(GLenum texture) {
        MG_Util::Debug::LogD("glActiveTexture, texture: %d", texture);
        GLenum result = MG_State::BindTextureUnit(texture);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void BindTexture(GLenum target, GLuint texture) {
        MG_Util::Debug::LogD("glBindTexture, target: %d, texture: %d", target, texture);
        GLenum result = MG_State::BindTexture(target, texture);
        if (result == GL_NO_ERROR) {
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void DeleteTextures(GLsizei n, const GLuint* textures) {
        MG_Util::Debug::LogD("glDeleteTextures, n: %d, textures: %p", n, textures);
        GLenum result = MG_State::DeleteTextures(n, textures);
        if (result == GL_NO_ERROR) {
            for (GLsizei i = 0; i < n; ++i) {
                GLuint tex = textures[i];
                auto texIt = MG_Diligent::g_TextureMap.find(tex);
                if (texIt != MG_Diligent::g_TextureMap.end()) {
                    if (texIt->second) {
                        texIt->second->Release();
                    }
                    MG_Diligent::g_TextureMap.erase(texIt);
                }

                auto srvIt = MG_Diligent::g_TextureViewMap.find(tex);
                if (srvIt != MG_Diligent::g_TextureViewMap.end()) {
                    if (srvIt->second) {
                        srvIt->second->Release();
                    }
                    MG_Diligent::g_TextureViewMap.erase(srvIt);
                }
            }
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void GenTextures(GLsizei n, GLuint* textures) {
        MG_Util::Debug::LogD("glGenTextures, n: %d, textures: %p", n, textures);
        GLenum result = MG_State::CreateTextures(n, textures);
        if (result == GL_NO_ERROR) {
            for (GLsizei i = 0; i < n; ++i) {
                MG_Diligent::g_TextureMap[textures[i]] = nullptr;
                MG_Diligent::g_TextureViewMap[textures[i]] = nullptr;
            }
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    Diligent::Uint32 CalculateMipLevels(GLsizei width, GLsizei height) {
        Diligent::Uint32 levels = 1;
        GLsizei size = std::max(width, height);

        while (size > 1) {
            size >>= 1;
            levels++;
        }

        return levels;
    }

    void TexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border,
                    GLenum format, GLenum type, const void* data) {
        MG_Util::Debug::LogD("glTexImage2D, target: %d, level: %d, internalFormat: %d, width: %d, height: %d, format: %d, type: %d, data: %p",
                             target, level, internalFormat, width, height, format, type, data);

        GLenum result = MG_State::UploadTexture2D(target, level, internalFormat, width, height,
                                                  border, format, type, data);
        if (result != GL_NO_ERROR) {
            MG_State::SetError(result);
            MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
            return;
        }

        MG_Util::Debug::LogD("TexImage2D: MG_State::UploadTexture2D successful.");

        auto& texState = *MG_State_T::textureState;
        auto activeUnit = texState.activeTextureUnit_;
        auto unitState = &texState.textureUnits_[activeUnit];
        GLuint boundTextureID = unitState->GetBoundTexture(target);

        if (boundTextureID == 0) {
            MG_Util::Debug::LogD("TexImage2D: No texture bound to active unit. Returning.");
            return;
        }

        Diligent::ITexture* pTexture = nullptr;
        Diligent::ITextureView* pSRV = nullptr;

        auto texIt = MG_Diligent::g_TextureMap.find(boundTextureID);
        if (texIt != MG_Diligent::g_TextureMap.end()) {
            pTexture = texIt->second;
        }
        auto srvIt = MG_Diligent::g_TextureViewMap.find(boundTextureID);
        if (srvIt != MG_Diligent::g_TextureViewMap.end()) {
            pSRV = srvIt->second;
        }

        Diligent::TEXTURE_FORMAT newFormat = ConvertInternalFormat(internalFormat);
        MG_Util::Debug::LogD("TexImage2D: New internalFormat: %d maps to Diligent format: %d",
                             internalFormat, newFormat);

        bool needCreateTexture = false;
        bool isBaseLevel = (level == 0);

        if (!pTexture) {
            MG_Util::Debug::LogD("TexImage2D: No existing Diligent texture found for GL name %u. Creating new.", boundTextureID);
            needCreateTexture = true;
        } else {
            Diligent::TextureDesc existingDesc = pTexture->GetDesc();

            if (isBaseLevel &&
                (existingDesc.Width != static_cast<Diligent::Uint32>(width) ||
                 existingDesc.Height != static_cast<Diligent::Uint32>(height) ||
                 existingDesc.Format != newFormat)) {

                MG_Util::Debug::LogD("TexImage2D: Base level properties changed. Recreating texture.");
                MG_Util::Debug::LogD("TexImage2D: Old (W:%u, H:%u, F:%d), New (W:%d, H:%d, F:%d)",
                                     existingDesc.Width, existingDesc.Height, existingDesc.Format,
                                     width, height, newFormat);

                needCreateTexture = true;
            } else if (level >= static_cast<GLint>(existingDesc.MipLevels)) {
                MG_Util::Debug::LogD("TexImage2D: Level %d exceeds current mip levels (%d). Recreating texture.",
                                     level, existingDesc.MipLevels);
                needCreateTexture = true;
            }
        }

        if (needCreateTexture) {
            if (pSRV) {
                MG_Util::Debug::LogD("TexImage2D: Releasing existing SRV %p", (void*)pSRV);
                pSRV->Release();
                MG_Diligent::g_TextureViewMap[boundTextureID] = nullptr;
                pSRV = nullptr;
            }

            if (pTexture) {
                MG_Util::Debug::LogD("TexImage2D: Releasing existing texture %p", (void*)pTexture);
                pTexture->Release();
                MG_Diligent::g_TextureMap[boundTextureID] = nullptr;
                pTexture = nullptr;
            }

            Diligent::Uint32 mipLevels = 1;
             if (isBaseLevel) {
                mipLevels = CalculateMipLevels(width, height);
            } else if (pTexture) {
                mipLevels = pTexture->GetDesc().MipLevels;
            }

            Diligent::TextureDesc TexDesc;
            if constexpr (MG_Global::Common::LogLevel <= MG_Constants::Common::LOG_LEVEL_DEBUG) {
                TexDesc.Name = std::format("GL Texture {}", boundTextureID).c_str();
            }
            TexDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
            TexDesc.Width = width;
            TexDesc.Height = height;
            TexDesc.Format = newFormat;
            TexDesc.MipLevels = mipLevels;

            bool isDepthStencil = false;
            switch (internalFormat) {
                case GL_DEPTH_COMPONENT:
                case GL_DEPTH_COMPONENT16:
                case GL_DEPTH_COMPONENT24:
                case GL_DEPTH_COMPONENT32:
                case GL_DEPTH_COMPONENT32F:
                case GL_DEPTH_STENCIL:
                case GL_DEPTH24_STENCIL8:
                case GL_DEPTH32F_STENCIL8:
                    isDepthStencil = true;
                    break;
                default:
                    isDepthStencil = false;
            }

            if (isDepthStencil) {
                TexDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_DEPTH_STENCIL;
            } else {
                TexDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
            }

            MG_Util::Debug::LogD("TexImage2D: Creating new Diligent texture with Width: %d, Height: %d, Format: %d, MipLevels: %u",
                                 width, height, newFormat, mipLevels);

            Diligent::ITexture* newTexture = nullptr;
            MG_Diligent::g_pDevice->CreateTexture(TexDesc, nullptr, &newTexture);

            if (newTexture) {
                MG_Util::Debug::LogD("TexImage2D: Created new Diligent texture %p for GL name %u.",
                                     (void*)newTexture, boundTextureID);

                Diligent::TextureViewDesc SRVDesc;
                SRVDesc.ViewType = Diligent::TEXTURE_VIEW_SHADER_RESOURCE;
                SRVDesc.TextureDim = Diligent::RESOURCE_DIM_TEX_2D;
                SRVDesc.MostDetailedMip = 0;
                SRVDesc.NumMipLevels = TexDesc.MipLevels;

                Diligent::ITextureView* newSRV = nullptr;
                newTexture->CreateView(SRVDesc, &newSRV);
                MG_Util::Debug::LogD("TexImage2D: Created new SRV %p for texture.", (void*)newSRV);

                MG_Diligent::g_TextureMap[boundTextureID] = newTexture;
                MG_Diligent::g_TextureViewMap[boundTextureID] = newSRV;

                pTexture = newTexture;
                pSRV = newSRV;
            } else {
                MG_Util::Debug::LogE("TexImage2D: Failed to create new texture!");
                return;
            }
        }

        if (pTexture && data) {
            Diligent::TextureSubResData SubResData;
            SubResData.pData = data;
            SubResData.Stride = width * GetBytesPerPixel(format, type);

            Diligent::Box UpdateBox;
            UpdateBox.MinX = 0;
            UpdateBox.MaxX = width;
            UpdateBox.MinY = 0;
            UpdateBox.MaxY = height;
            UpdateBox.MinZ = 0;
            UpdateBox.MaxZ = 1;

            MG_Util::Debug::LogD("TexImage2D: Updating texture. Level: %d, Box: [%u,%u]x[%u,%u], Stride: %llu",
                                 level, UpdateBox.MinX, UpdateBox.MaxX, UpdateBox.MinY, UpdateBox.MaxY, SubResData.Stride);

            MG_Diligent::g_pContext->UpdateTexture(
                    pTexture,
                    level,
                    0,
                    UpdateBox,
                    SubResData,
                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
            );
            MG_Util::Debug::LogD("TexImage2D: UpdateTexture completed for level %d.", level);
        }
    }

    void TexParameterf(GLenum target, GLenum pname, GLfloat param) {
        MG_Util::Debug::LogD("glTexParameterf, target: %d, pname: %d, param: %f", target, pname, param);
        GLenum result = MG_State::SetTexturePropertyFloat(target, pname, param);
        if (result == GL_NO_ERROR) {
            MG_State::SetError(result);
            MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
            return;
        }

        // TODO: Fix sampler
        auto& texState = *MG_State_T::textureState;
        auto activeUnit = texState.activeTextureUnit_;
        auto unitState = &texState.textureUnits_[activeUnit];
        GLuint boundTextureID = unitState->GetBoundTexture(target);

        if (boundTextureID == 0) {
            MG_Util::Debug::LogD("TexParameterf: No texture bound. Skipping sampler update.");
            return;
        }

        Diligent::SamplerDesc SamDesc;
        auto it = MG_Diligent::g_SamplerMap.find(boundTextureID);
        if (it != MG_Diligent::g_SamplerMap.end() && it->second) {
            SamDesc = it->second->GetDesc();
        } else {
            SamDesc.MinFilter = Diligent::FILTER_TYPE_LINEAR;
            SamDesc.MagFilter = Diligent::FILTER_TYPE_LINEAR;
            SamDesc.AddressU = Diligent::TEXTURE_ADDRESS_WRAP;
            SamDesc.AddressV = Diligent::TEXTURE_ADDRESS_WRAP;
            SamDesc.AddressW = Diligent::TEXTURE_ADDRESS_WRAP;
        }

        switch (pname) {
            case GL_TEXTURE_MIN_FILTER:
                FillFilterTypeFromGLEnum(pname, param, SamDesc.MinFilter, SamDesc.MagFilter, SamDesc.MipFilter);
                break;
            case GL_TEXTURE_MAG_FILTER:
                FillFilterTypeFromGLEnum(pname, param, SamDesc.MinFilter, SamDesc.MagFilter, SamDesc.MipFilter);
                break;
            case GL_TEXTURE_WRAP_S:
                SamDesc.AddressU = ConvertAddressMode(param);
                break;
            case GL_TEXTURE_WRAP_T:
                SamDesc.AddressV = ConvertAddressMode(param);
                break;
            case GL_TEXTURE_WRAP_R:
                SamDesc.AddressW = ConvertAddressMode(param);
                break;
            case GL_TEXTURE_MIN_LOD:
                SamDesc.MinLOD = (float)param;
                break;
            case GL_TEXTURE_MAX_LOD:
                SamDesc.MaxLOD = param;
                break;
            case GL_TEXTURE_COMPARE_FUNC:
                SamDesc.ComparisonFunc = ConvertComparsionFunc(param);
                break;
            case GL_TEXTURE_COMPARE_MODE:
                FillFilterTypeFromGLEnum(pname, param, SamDesc.MinFilter, SamDesc.MagFilter, SamDesc.MipFilter);
                break;
            default:
                MG_Util::Debug::LogE("%s: not handled pname: %s", __func__, MG_Util::Debug::GLEnumToString(pname));
        }

        Diligent::ISampler* pNewSampler = nullptr;
        MG_Diligent::g_pDevice->CreateSampler(SamDesc, &pNewSampler);

        if (it != MG_Diligent::g_SamplerMap.end()) {
            if (it->second) {
                it->second->Release();
            }
            it->second = pNewSampler;
        } else {
            MG_Diligent::g_SamplerMap[boundTextureID] = pNewSampler;
        }

        MG_Util::Debug::LogD("TexParameterf: Updated sampler for texture %u", boundTextureID);
    }

    void TexParameteri(GLenum target, GLenum pname, GLint param) {
        MG_Util::Debug::LogD("glTexParameteri, target: %d, pname: %d, param: %d", target, pname, param);
        GLenum result = MG_State::SetTexturePropertyInt(target, pname, param);
        if (result != GL_NO_ERROR) {
            MG_State::SetError(result);
            MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
            return;
        }

        // TODO: Fix sampler
        auto& texState = *MG_State_T::textureState;
        auto activeUnit = texState.activeTextureUnit_;
        auto unitState = &texState.textureUnits_[activeUnit];
        GLuint boundTextureID = unitState->GetBoundTexture(target);
        
        if (boundTextureID == 0) {
            MG_Util::Debug::LogD("TexParameteri: No texture bound. Skipping sampler update.");
            return;
        }
        
        Diligent::SamplerDesc SamDesc;
        auto it = MG_Diligent::g_SamplerMap.find(boundTextureID);
        if (it != MG_Diligent::g_SamplerMap.end() && it->second) {
            SamDesc = it->second->GetDesc();
        } else {
            SamDesc.MinFilter = Diligent::FILTER_TYPE_LINEAR;
            SamDesc.MagFilter = Diligent::FILTER_TYPE_LINEAR;
            SamDesc.AddressU = Diligent::TEXTURE_ADDRESS_WRAP;
            SamDesc.AddressV = Diligent::TEXTURE_ADDRESS_WRAP;
            SamDesc.AddressW = Diligent::TEXTURE_ADDRESS_WRAP;
        }
        
        switch (pname) {
            case GL_TEXTURE_MIN_FILTER:
                FillFilterTypeFromGLEnum(pname, param, SamDesc.MinFilter, SamDesc.MagFilter, SamDesc.MipFilter);
                break;
            case GL_TEXTURE_MAG_FILTER:
                FillFilterTypeFromGLEnum(pname, param, SamDesc.MagFilter, SamDesc.MagFilter, SamDesc.MipFilter);
                break;
            case GL_TEXTURE_WRAP_S:
                SamDesc.AddressU = ConvertAddressMode(param);
                break;
            case GL_TEXTURE_WRAP_T:
                SamDesc.AddressV = ConvertAddressMode(param);
                break;
            case GL_TEXTURE_WRAP_R:
                SamDesc.AddressW = ConvertAddressMode(param);
                break;
            case GL_TEXTURE_MIN_LOD:
                SamDesc.MinLOD = (float)param;
                break;
            case GL_TEXTURE_MAX_LOD:
                SamDesc.MaxLOD = (float)param;
                break;
            case GL_TEXTURE_COMPARE_FUNC:
                SamDesc.ComparisonFunc = ConvertComparsionFunc(param);
                break;
            case GL_TEXTURE_COMPARE_MODE:
                FillFilterTypeFromGLEnum(pname, param, SamDesc.MinFilter, SamDesc.MagFilter, SamDesc.MipFilter);
                break;
            default:
                MG_Util::Debug::LogE("%s: not handled pname: %s", __func__, MG_Util::Debug::GLEnumToString(pname));
        }
        
        Diligent::ISampler* pNewSampler = nullptr;
        MG_Diligent::g_pDevice->CreateSampler(SamDesc, &pNewSampler);
        
        if (it != MG_Diligent::g_SamplerMap.end()) {
            if (it->second) {
                it->second->Release();
            }
            it->second = pNewSampler;
        } else {
            MG_Diligent::g_SamplerMap[boundTextureID] = pNewSampler;
        }
        
        MG_Util::Debug::LogD("TexParameteri: Updated sampler for texture %u", boundTextureID);
    }

    void TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                       GLsizei height, GLenum format, GLenum type, const void* pixels) {
        MG_Util::Debug::LogD("glTexSubImage2D, target: %d, level: %d, xoffset: %d, yoffset: %d, width: %d, height: %d, format: %d, type: %d, pixels: %p",
                             target, level, xoffset, yoffset, width, height, format, type, pixels);

        GLenum result = MG_State::UpdateTextureRegion2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
        if (result != GL_NO_ERROR) {
            MG_State::SetError(result);
            MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
            return;
        }

        auto& texState = *MG_State_T::textureState;
        auto activeUnit = texState.activeTextureUnit_;
        auto unitState = &texState.textureUnits_[activeUnit];
        GLuint boundTextureID = unitState->GetBoundTexture(target);

        MG_Util::Debug::LogD("TexSubImage2D: bound texture: GL name %u", boundTextureID);

        if (boundTextureID == 0) {
            MG_Util::Debug::LogD("TexSubImage2D: No texture bound to active unit. Skipping update.");
            return;
        }

        Diligent::ITexture* pTexture = nullptr;
        auto texIt = MG_Diligent::g_TextureMap.find(boundTextureID);
        if (texIt != MG_Diligent::g_TextureMap.end()) {
            pTexture = texIt->second;
        }

        if (!pTexture) {
            MG_Util::Debug::LogW("TexSubImage2D: Diligent texture not found for GL name %u", boundTextureID);
            return;
        }

        const auto& texDesc = pTexture->GetDesc();
        if (!(texDesc.BindFlags & Diligent::BIND_SHADER_RESOURCE)) {
            MG_Util::Debug::LogE("TexSubImage2D: Texture %u does not have BIND_SHADER_RESOURCE flag", boundTextureID);
            return;
        }

        auto rowLength = MG_State::QueryPixelStoreInt(GL_UNPACK_ROW_LENGTH);
        rowLength = ((rowLength > 0) ? rowLength : width);
        auto skipRows = MG_State::QueryPixelStoreInt(GL_UNPACK_SKIP_ROWS);
        auto skipPixels = MG_State::QueryPixelStoreInt(GL_UNPACK_SKIP_PIXELS);
        auto bpp = GetBytesPerPixel(format, type);
        char* pixelStart = (char*)pixels + bpp * (rowLength * skipRows + skipPixels);

        Diligent::TextureSubResData SubResData;
        SubResData.Stride = rowLength * bpp;
        SubResData.pData = pixelStart;

        Diligent::Box UpdateBox;
        UpdateBox.MinX = xoffset;
        UpdateBox.MaxX = xoffset + width;
        UpdateBox.MinY = yoffset;
        UpdateBox.MaxY = yoffset + height;
        UpdateBox.MinZ = 0;
        UpdateBox.MaxZ = 1;

        MG_Util::Debug::LogD("TexSubImage2D: Got GL_UNPACK_ROW_LENGTH = %d", rowLength);

        MG_Util::Debug::LogD("TexSubImage2D: Updating region [%d,%d]-[%d,%d] at level %d, %d bytes",
                             xoffset, yoffset, xoffset+width, yoffset+height, level, width * height * GetBytesPerPixel(format, type));

//        if (MG_Diligent::IsInRenderPass) {
//            MG_Diligent::g_pContext->EndRenderPass();
//            MG_Diligent::IsInRenderPass = false;
//        }
        
        MG_Diligent::g_pContext->UpdateTexture(
                pTexture,
                level,
                0,
                UpdateBox,
                SubResData,
                Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
        );

        MG_Util::Debug::LogD("TexSubImage2D: Update completed successfully");
    }


    void GetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params) {
        MG_Util::Debug::LogD("glGetTexLevelParameteriv, target: %d, level: %d, pname: %d, params: %p",
                             target, level, pname, params);
        GLenum result = MG_State::QueryTextureLevelPropertyIntVector(target, level, pname, params);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
        MG_Util::Debug::LogD("glCopyTexSubImage2D, target: %d, level: %d, xoffset: %d, yoffset: %d, x: %d, y: %d, width: %d, height: %d",
                             target, level, xoffset, yoffset, x, y, width, height);

    }
}
