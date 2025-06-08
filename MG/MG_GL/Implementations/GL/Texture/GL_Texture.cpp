//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_Texture.h"

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
  
    Diligent::TEXTURE_ADDRESS_MODE ConvertAddressMode(GLint param) {
        switch (param) {
            case GL_REPEAT: return Diligent::TEXTURE_ADDRESS_WRAP;
            case GL_CLAMP_TO_EDGE: return Diligent::TEXTURE_ADDRESS_CLAMP;
            case GL_MIRRORED_REPEAT: return Diligent::TEXTURE_ADDRESS_MIRROR;
            default: return Diligent::TEXTURE_ADDRESS_WRAP;
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

        if (!pTexture) {
            MG_Util::Debug::LogD("TexImage2D: No existing Diligent texture found for GL name %u. Creating new.", boundTextureID);
            needCreateTexture = true;
            // Create a placeholder texture if none exists
            if (width == 0 || height == 0) {
                Diligent::TextureDesc PlaceholderTexDesc;
                PlaceholderTexDesc.Type = (target == GL_TEXTURE_2D) ?
                               Diligent::RESOURCE_DIM_TEX_2D : Diligent::RESOURCE_DIM_UNDEFINED;
                PlaceholderTexDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
                PlaceholderTexDesc.Width = 1;
                PlaceholderTexDesc.Height = 1;
                bool isDepth = false;
                switch (newFormat) {
                    case Diligent::TEX_FORMAT_D16_UNORM:
                    case Diligent::TEX_FORMAT_D32_FLOAT:
                    case Diligent::TEX_FORMAT_D24_UNORM_S8_UINT:
                    case Diligent::TEX_FORMAT_D32_FLOAT_S8X24_UINT:
                        isDepth = true;
                        break;
                    default:
                        break;
                }
                PlaceholderTexDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | (isDepth ? Diligent::BIND_DEPTH_STENCIL : Diligent::BIND_RENDER_TARGET);

                Diligent::ITexture* pPlaceholderTexture = nullptr;
                MG_Diligent::g_pDevice->CreateTexture(PlaceholderTexDesc, nullptr, &pPlaceholderTexture);

                Diligent::ITextureView* pPlaceholderSRV = nullptr;
                if (pPlaceholderTexture) {
                    Diligent::TextureViewDesc SRVDesc;
                    SRVDesc.ViewType = data == nullptr ? Diligent::TEXTURE_VIEW_RENDER_TARGET : Diligent::TEXTURE_VIEW_SHADER_RESOURCE;
                    SRVDesc.TextureDim = Diligent::RESOURCE_DIM_TEX_2D;
                    pPlaceholderTexture->CreateView(SRVDesc, &pPlaceholderSRV);
                }
                MG_Diligent::g_TextureMap[boundTextureID] = pPlaceholderTexture;
                MG_Diligent::g_TextureViewMap[boundTextureID] = pPlaceholderSRV;
                MG_Util::Debug::LogD("Created placeholder Diligent texture for GL name %u", boundTextureID);
            }
        } else {
            Diligent::TextureDesc existingDesc = pTexture->GetDesc();
            if (existingDesc.Width != static_cast<Diligent::Uint32>(width) ||
                existingDesc.Height != static_cast<Diligent::Uint32>(height) ||
                existingDesc.Format != newFormat) {
                MG_Util::Debug::LogD("TexImage2D: Texture properties changed. Recreating texture.");
                MG_Util::Debug::LogD("TexImage2D: Old (W:%u, H:%u, F:%d), New (W:%d, H:%d, F:%d)",
                                     existingDesc.Width, existingDesc.Height, existingDesc.Format,
                                     width, height, newFormat);

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

                needCreateTexture = true;
            }
        }

        if (needCreateTexture) {
            Diligent::ITexture* newTexture = nullptr;
            Diligent::ITextureView* newSRV = nullptr;

            Diligent::TextureDesc TexDesc;
            TexDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
            TexDesc.Width = width;
            TexDesc.Height = height;
            TexDesc.Format = newFormat;

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
            
            if (level > 0) {
                TexDesc.MipLevels = level + 1;
            }

            MG_Util::Debug::LogD("TexImage2D: Creating new Diligent texture with Width: %d, Height: %d, Format: %d",
                                 width, height, newFormat);

            MG_Diligent::g_pDevice->CreateTexture(TexDesc, nullptr, &newTexture);

            if (newTexture) {
                MG_Util::Debug::LogD("TexImage2D: Created new Diligent texture %p for GL name %u.",
                                     (void*)newTexture, boundTextureID);

                Diligent::TextureViewDesc SRVDesc;
                SRVDesc.ViewType = Diligent::TEXTURE_VIEW_SHADER_RESOURCE;
                SRVDesc.TextureDim = Diligent::RESOURCE_DIM_TEX_2D;
                SRVDesc.MostDetailedMip = 0;
                SRVDesc.NumMipLevels = TexDesc.MipLevels;

                newTexture->CreateView(SRVDesc, &newSRV);
                MG_Util::Debug::LogD("TexImage2D: Created new SRV %p for texture.", (void*)newSRV);

                MG_Diligent::g_TextureMap[boundTextureID] = newTexture;
                MG_Diligent::g_TextureViewMap[boundTextureID] = newSRV;

                pTexture = newTexture;
                pSRV = newSRV;
            } else {
                MG_Util::Debug::LogE("TexImage2D: Failed to create new texture!");
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
            MG_Util::Debug::LogD("TexImage2D: UpdateTexture completed.");
        }
    }


    void TexParameterf(GLenum target, GLenum pname, GLfloat param) {
        MG_Util::Debug::LogD("glTexParameterf, target: %d, pname: %d, param: %f", target, pname, param);
        GLenum result = MG_State::SetTexturePropertyFloat(target, pname, param);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void TexParameteri(GLenum target, GLenum pname, GLint param) {
        MG_Util::Debug::LogD("glTexParameteri, target: %d, pname: %d, param: %d", target, pname, param);
        GLenum result = MG_State::SetTexturePropertyInt(target, pname, param);
        if (result != GL_NO_ERROR) {
            MG_State::SetError(result);
            MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
            return;
        }
        
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
                SamDesc.MinFilter = (param == GL_NEAREST) ?
                                    Diligent::FILTER_TYPE_POINT : Diligent::FILTER_TYPE_LINEAR;
                break;
            case GL_TEXTURE_MAG_FILTER:
                SamDesc.MagFilter = (param == GL_NEAREST) ?
                                    Diligent::FILTER_TYPE_POINT : Diligent::FILTER_TYPE_LINEAR;
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

        Diligent::TextureDesc desc = pTexture->GetDesc();

        Diligent::TextureSubResData SubResData;
        SubResData.pData = pixels;
        SubResData.Stride = width * GetBytesPerPixel(format, type);

        Diligent::Box UpdateBox;
        UpdateBox.MinX = xoffset;
        UpdateBox.MaxX = xoffset + width;
        UpdateBox.MinY = yoffset;
        UpdateBox.MaxY = yoffset + height;
        UpdateBox.MinZ = 0;
        UpdateBox.MaxZ = 1;

        MG_Util::Debug::LogD("TexSubImage2D: Updating region [%d,%d]-[%d,%d] at level %d",
                             xoffset, yoffset, xoffset+width, yoffset+height, level);

        if (MG_Diligent::IsInRenderPass) {
            MG_Diligent::g_pContext->EndRenderPass();
            MG_Diligent::IsInRenderPass = false;
        }
        
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
}
