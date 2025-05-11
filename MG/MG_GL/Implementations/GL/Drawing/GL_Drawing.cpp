//
// Created by BZLZHH on 2025/3/15.
//

#include "GL_Drawing.h"

#undef MOBILEGL_GLSLTOOL_H
#include "../../../../Includes.h"

namespace MG_GL::GL {
    template <typename K, typename V>
    using unordered_map = ankerl::unordered_dense::map<K, V>;

    void NormalizePixelFormat(GLenum internalFormat, GLenum type, GLenum format, GLenum* outInternalFormat, GLenum* outType, GLenum* outFormat) {
//        if (format && *format == GL_BGRA)
//            *format = GL_RGBA;
        switch (internalFormat) {
            case GL_DEPTH_COMPONENT16:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_UNSIGNED_SHORT;
                if (outFormat)
                    *outFormat = format;
                break;

            case GL_DEPTH_COMPONENT24:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_UNSIGNED_INT;
                if (outFormat)
                    *outFormat = format;
                break;

            case GL_DEPTH_COMPONENT32:
                if (outInternalFormat)
                    *outInternalFormat = GL_DEPTH_COMPONENT32F;
                if (outType)
                    *outType = GL_UNSIGNED_INT;
                if (outFormat)
                    *outFormat = format;
                break;

            case GL_DEPTH_COMPONENT32F:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_FLOAT;
                if (outFormat)
                    *outFormat = format;
                break;

            case GL_DEPTH_COMPONENT:
                MG_Util::Debug::LogD("Find GL_DEPTH_COMPONENT: internalFormat: %s, format: %s, type: %s",
                      MG_Util::Debug::GLEnumToString(internalFormat), MG_Util::Debug::GLEnumToString(format), MG_Util::Debug::GLEnumToString(type));

                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_UNSIGNED_INT;
                if (outFormat)
                    *outFormat = format;
                break;

            case GL_DEPTH_STENCIL:
                if (outInternalFormat)
                    *outInternalFormat = GL_DEPTH32F_STENCIL8;
                if (outType)
                    *outType = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
                if (outFormat)
                    *outFormat = format;
                break;

            case GL_RGB10_A2:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_UNSIGNED_INT_2_10_10_10_REV;
                if (outFormat)
                    *outFormat = format;
                break;

            case GL_RGB5_A1:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_UNSIGNED_SHORT_5_5_5_1;
                if (outFormat)
                    *outFormat = format;
                break;

            case GL_COMPRESSED_RED_RGTC1:
            case GL_COMPRESSED_RG_RGTC2:
                MG_Util::Debug::LogD("GL_COMPRESSED_RED_RGTC1 or GL_COMPRESSED_RG_RGTC2 is not supported!");
                break;

            case GL_SRGB8:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_UNSIGNED_BYTE;
                if (outFormat)
                    *outFormat = format;
                break;

            case GL_RGBA32F:
            case GL_RGB32F:
            case GL_RG32F:
            case GL_R32F:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_FLOAT;
                if (outFormat)
                    *outFormat = format;
                break;

            case GL_RGB9_E5:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_UNSIGNED_INT_5_9_9_9_REV;
                if (outFormat)
                    *outFormat = format;
                break;

            case GL_R11F_G11F_B10F:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_UNSIGNED_INT_10F_11F_11F_REV;
                if (outFormat)
                    *outFormat = GL_RGB;
                break;

            case GL_RGBA32UI:
            case GL_RGB32UI:
            case GL_RG32UI:
            case GL_R32UI:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_UNSIGNED_INT;
                if (outFormat)
                    *outFormat = format;
                break;

            case GL_RGBA32I:
            case GL_RGB32I:
            case GL_RG32I:
            case GL_R32I:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_INT;
                if (outFormat)
                    *outFormat = format;
                break;

            case GL_RGBA16: {
//                if (!checked_rgba16) {
//                    support_rgba16 = check_rgba16();
//                    checked_rgba16 = true;
//                }
//                if (support_rgba16) {
//                    if(type)
//                        *type = GL_UNSIGNED_SHORT;
//                } else {
//                    *internal_format = GL_RGBA16F;
//                    if(type)
//                        *type = GL_FLOAT;
//                }
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_FLOAT;
                if (outFormat)
                    *outFormat = format;
                break;
            }
            case GL_RGBA8:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_UNSIGNED_BYTE;
                if (outFormat)
                    *outFormat = GL_RGBA;
                break;

            case GL_RGBA:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_UNSIGNED_BYTE;
                if (outFormat)
                    *outFormat = GL_RGBA;
                break;

            case GL_RGBA16F:
            case GL_R16F:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_HALF_FLOAT;
                if (outFormat)
                    *outFormat = format;
                break;

            case GL_R16:
                if (outInternalFormat)
                    *outInternalFormat = GL_R16F;
                if (outType)
                    *outType = GL_FLOAT;
                if (outFormat)
                    *outFormat = format;
                break;

            case GL_RGB16:
                if (outInternalFormat)
                    *outInternalFormat = GL_RGB16F;
                if (outType)
                    *outType = GL_HALF_FLOAT;
                if (outFormat)
                    *outFormat = GL_RGB;
                break;

            case GL_RGB16F:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_HALF_FLOAT;
                if (outFormat)
                    *outFormat = GL_RGB;
                break;

            case GL_RG16:
            case GL_RG16F:
                if (outInternalFormat)
                    *outInternalFormat = GL_RG16F;
                if (outType)
                    *outType = GL_HALF_FLOAT;
                if (outFormat)
                    *outFormat = GL_RG;
                break;

            case GL_R8:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_UNSIGNED_BYTE;
                if (outFormat)
                    *outFormat = GL_RED;
                break;
            case GL_R8UI:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_UNSIGNED_BYTE;
                if (outFormat)
                    *outFormat = GL_RED_INTEGER;
                break;

            case GL_RGB8_SNORM:
            case GL_RGBA8_SNORM:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_BYTE;
                if (outFormat)
                    *outFormat = format;
                break;
            case GL_RGB8:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType && type != GL_UNSIGNED_BYTE)
                    *outType = GL_UNSIGNED_BYTE;
                if (outFormat)
                    *outFormat = GL_RGB;
                break;
            case GL_RGBA16_SNORM:
                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = GL_SHORT;
                if (outFormat)
                    *outFormat = format;
                break;
            default:
                MG_Util::Debug::LogD("NormalizePixelFormat: no conversion");

                if (outInternalFormat)
                    *outInternalFormat = internalFormat;
                if (outType)
                    *outType = type;
                if (outFormat)
                    *outFormat = format;
                break;
        }
    }


    std::string processOutColorLocations(const std::string& glslCode) {
        const static std::regex pattern(R"(\n(out highp vec4 outColor)(\d+);)");
        const std::string replacement = "\nlayout(location=$2) $1$2;";
        return std::regex_replace(glslCode, pattern, replacement);
    }
    
    std::string forceSupporterOutput(const std::string& glslCode) {
        bool hasPrecisionFloat = glslCode.find("precision ") != std::string::npos &&
                                 glslCode.find("float;") != std::string::npos;
        bool hasPrecisionInt = glslCode.find("precision ") != std::string::npos &&
                               glslCode.find("int;") != std::string::npos;

        std::string result = glslCode;
        std::string precisionFloat;
        std::string precisionInt;

        if (hasPrecisionFloat && hasPrecisionInt) {
            std::istringstream iss(result);
            std::vector<std::string> lines;
            std::string line;
            while (std::getline(iss, line)) {
                bool isPrecisionLine = (line.find("precision ") != std::string::npos) &&
                                       (line.find("float;") != std::string::npos || line.find("int;") != std::string::npos);
                if (!isPrecisionLine) {
                    lines.push_back(line);
                }
            }
            result.clear();
            for (size_t i = 0; i < lines.size(); ++i) {
                if (i != 0) result += '\n';
                result += lines[i];
            }
            precisionFloat = "precision highp float;\n";
            precisionInt = "precision highp int;\n";
        } else {
            precisionFloat = hasPrecisionFloat ? "" : "precision highp float;\n";
            precisionInt = hasPrecisionInt ? "" : "precision highp int;\n";
        }

        size_t lastExtensionPos = result.rfind("#extension");
        size_t insertionPos = 0;

        if (lastExtensionPos != std::string::npos) {
            size_t nextNewline = result.find('\n', lastExtensionPos);
            if (nextNewline != std::string::npos) {
                insertionPos = nextNewline + 1;
            } else {
                insertionPos = result.length();
            }
        } else {
            size_t firstNewline = result.find('\n');
            if (firstNewline != std::string::npos) {
                insertionPos = firstNewline + 1;
            } else {
                result = precisionFloat + precisionInt + result;
                return result;
            }
        }

        result.insert(insertionPos, precisionFloat + precisionInt);
        return result;
    }

    std::string removeLayoutBinding(const std::string& glslCode) {
        static std::regex bindingRegex(R"(layout\s*\(\s*binding\s*=\s*\d+\s*\)\s*)");
        std::string result = std::regex_replace(glslCode, bindingRegex, "");
        static std::regex bindingRegex2(R"(layout\s*\(\s*binding\s*=\s*\d+\s*,)");
        result = std::regex_replace(result, bindingRegex2, "layout(");
        return result;
    }
    
    static unordered_map<GLuint, GLuint> s_textureMap;
    static unordered_map<GLuint, GLuint> s_vaoMap;
    static unordered_map<GLuint, GLuint> s_bufferMap;
    static unordered_map<GLuint, GLuint> s_programMap;
    static unordered_map<GLuint, GLuint> s_framebufferMap;
    struct MipLevelInfo {
        GLenum internalFormat;
        GLsizei width;
        GLsizei height;
        GLenum format;
        GLenum type;
    };

    void CheckGLESError() {
        while (GLenum err = ::GLES::glGetError() != GL_NO_ERROR) {
            MG_Util::Debug::LogE("-> GLES Error: %s", MG_Util::Debug::GLEnumToString(err));
        }
    }

#define CallAndCheck(operation) MG_Util::Debug::LogD("GLES call: %s", #operation); operation CheckGLESError();
//    static std::unordered_map<GLuint, std::unordered_map<GLint, bool>> s_textureLevelUploaded;

    void SyncAllTexturesToGLES(TextureState* textureState) {
        MG_Util::Debug::LogD("Syncing all textures to GLES...");
        for (auto& [mgTexId, texObj] : textureState->textures) {
            if (!texObj.generated)
                continue;

            GLuint glTexId = 0;
            if (s_textureMap.find(mgTexId) == s_textureMap.end()) {
                CallAndCheck(::GLES::glGenTextures(1, &glTexId);)
                s_textureMap[mgTexId] = glTexId;
                MG_Util::Debug::LogD("Created new GLES texture %u (MobileGL ID)", mgTexId);
            } else {
                glTexId = s_textureMap[mgTexId];
            }
            GLenum target = texObj.target;
            CallAndCheck(::GLES::glBindTexture(target, glTexId);)

            for (auto& [pname, param] : texObj.params.texPropertiesInt) {
                CallAndCheck(::GLES::glTexParameteri(target, pname, param);)
            }
            for (auto& [pname, param] : texObj.params.texPropertiesFloat) {
                CallAndCheck(::GLES::glTexParameterf(target, pname, param);)
            }

            for (auto& [level, mip] : texObj.params.mipmapData) {
                if (!mip.dirty)
                    continue;
                mip.dirty = false;
                const void* data = !mip.pixelData.empty() ? mip.pixelData.data() : nullptr;
                switch (target) {
                    case GL_TEXTURE_2D: {
                        GLenum internalFormat = 0, type = 0, format = 0;
                        NormalizePixelFormat(mip.internalFormat, mip.type, mip.format, &internalFormat, &type, &format);

                        CallAndCheck(::GLES::glTexImage2D(
                                target, level, internalFormat,
                                mip.width, mip.height, 0,
                                format, type, data
                        );)
//                        s_textureLevelUploaded[mgTexId][level] = true;
                        MG_Util::Debug::LogD("Initial upload texture %u level %d (size=%zu)",
                                             mgTexId, level, mip.pixelData.size());
                        break;
                    }
                    default:
                        MG_Util::Debug::LogE("Unsupported target: %s", MG_Util::Debug::GLEnumToString(target));
                }
            }
            MG_Util::Debug::LogD("Updated GLES texture %u (MobileGL ID)", mgTexId);
        }
    }

//    static std::unordered_map<GLuint, void*> s_bufferDirtyFlags_bufferObj;
    void SyncAllBuffersToGLES(BufferState* bufferState) {
        GLint prev_vbo = 0;
        CallAndCheck(::GLES::glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prev_vbo);)

        for (auto& [mgname, obj] : bufferState->buffers_) {
            if (!obj.generated)
                continue;

            if (!obj.dirty && !obj.isMapped)
                continue;

            obj.dirty = false;

            // Gen real buffers at ES
            if (s_bufferMap.find(mgname) == s_bufferMap.end()) {
                GLuint glname;
                CallAndCheck(::GLES::glGenBuffers(1, &glname);)
                s_bufferMap[mgname] = glname;
            }

            GLuint glname = s_bufferMap[mgname];

            // Populate data to ES
            CallAndCheck(::GLES::glBindBuffer(GL_ARRAY_BUFFER, glname);)
            // TODO: Check why obj.dataValid is broken for Minecraft 1.21.1-
            CallAndCheck(::GLES::glBufferData(
                        GL_ARRAY_BUFFER,
                        obj.data.size(),
                        obj.data.data(), //obj.dataValid || obj.isMapped ? obj.data.data() : nullptr,
                        obj.usage);)

//            s_bufferDirtyFlags_bufferObj[mgname] = obj.data.data();
        }
        CallAndCheck(::GLES::glBindBuffer(GL_ARRAY_BUFFER, prev_vbo);)
    }


    static GLuint lastBoundVAO = 0;
    static GLuint lastBoundProgram = 0;
    static GLuint lastBoundFBO[2] = {0};
    static std::array<GLuint, 32> lastBoundTextures;

    void RealizeFBOState(GLenum fbtype) {
        FramebufferState* fbState = MG_State_T::framebufferState;

        GLuint fb = fbState->currentBindings_[fbtype];
        if (fb != lastBoundFBO[(fbtype == GL_DRAW_FRAMEBUFFER) ? 0 : 1]) {
            GLuint glFBO = 0;

            if (fb == 0) {
                CallAndCheck(::GLES::glBindFramebuffer(fbtype, 0);)
                glFBO = 0;
            } else {
                bool isNewGlesFBO = false;

                if (s_framebufferMap.find(fb) == s_framebufferMap.end()) {
                    CallAndCheck(::GLES::glGenFramebuffers(1, &glFBO);)
                    s_framebufferMap[fb] = glFBO;
                    CallAndCheck(::GLES::glBindFramebuffer(fbtype, glFBO);)
                    MG_Util::Debug::LogD("Generated and bound new GLES FBO %u for MobileGL FBO %u", glFBO, fb);
                    isNewGlesFBO = true;
                } else {
                    glFBO = s_framebufferMap[fb];
                    CallAndCheck(::GLES::glBindFramebuffer(fbtype, glFBO);)
                    MG_Util::Debug::LogD("Bound existing GLES FBO %u for MobileGL FBO %u", glFBO, fb);
                }

                if (glFBO != 0) {
                    FramebufferObject* mgFBO = fbState->GetCurrentFBO(fbtype);
                    if (mgFBO) {
                        MG_Util::Debug::LogD("Checking/Syncing attachments for GLES FBO %u (MobileGL FBO %u)", glFBO, fb);

                        for (auto const& [mgAttachmentPoint, mgAtt] : mgFBO->attachments) {

                            if (mgAtt.type != GL_TEXTURE_2D) {
                                MG_Util::Debug::LogW("Skipping non-TEXTURE_2D attachment 0x%X for FBO %u", mgAttachmentPoint, fb);
                                continue;
                            }

                            GLuint expectedGLTexId = 0;
                            if (mgAtt.handle != 0) {
                                if (s_textureMap.find(mgAtt.handle) != s_textureMap.end()) {
                                    expectedGLTexId = s_textureMap[mgAtt.handle];
                                } else {
                                    MG_Util::Debug::LogE("MobileGL Texture %u for FBO %u attachment %s not found in s_textureMap during FBO sync!", mgAtt.handle, fb, MG_Util::Debug::GLEnumToString(mgAttachmentPoint));
                                    for (auto const& [key, val] : s_textureMap)
                                        MG_Util::Debug::LogW("  key: %d, val: %d", key, val);
                                    continue;
                                }
                            }

                            GLint glesAttachedType = 0;
                            GLint glesAttachedName = 0;

                            CallAndCheck(::GLES::glGetFramebufferAttachmentParameteriv(fbtype, mgAttachmentPoint, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &glesAttachedType);)

                            if (glesAttachedType == GL_TEXTURE) {
                                CallAndCheck(::GLES::glGetFramebufferAttachmentParameteriv(fbtype, mgAttachmentPoint, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &glesAttachedName);)
                            } else if (glesAttachedType != GL_NONE) {
                                MG_Util::Debug::LogW("GLES FBO %u attachment 0x%X has non-texture type 0x%X (expected GL_TEXTURE or GL_NONE)", glFBO, mgAttachmentPoint, glesAttachedType);
                            }

                            if ((GLuint)glesAttachedName != expectedGLTexId) {
                                MG_Util::Debug::LogD("Syncing FBO %u attachment 0x%X: Expected GLES TexID %u, Found GLES ObjName %d. Attaching/Detaching...",
                                                     fb, mgAttachmentPoint, expectedGLTexId, glesAttachedName);

                                CallAndCheck(::GLES::glFramebufferTexture2D(
                                        fbtype,
                                        mgAttachmentPoint,
                                        GL_TEXTURE_2D,
                                        expectedGLTexId,
                                        mgAtt.mipLevel
                                );)
                            }
                        }

                        GLenum status = ::GLES::glCheckFramebufferStatus(fbtype);
                        if (status != GL_FRAMEBUFFER_COMPLETE) {
                            MG_Util::Debug::LogE("Framebuffer %u (GLES FBO %u) is not complete after sync! Status: 0x%X (%s)",
                                                 fb, glFBO, status, MG_Util::Debug::GLEnumToString(status));
                        }

                    } else {
                        MG_Util::Debug::LogW("Could not get MobileGL FBO object for ID %u during attachment sync.", fb);
                    }
                }
            }

            lastBoundFBO[(fbtype == GL_DRAW_FRAMEBUFFER) ? 0 : 1] = fb;
        }
    }


    void DrawArraysSHITTILY(GLenum mode, GLint first, GLsizei count) {
        
    }

    void DrawElementsSHITTILY(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices) {
        CommonState* commonState = MG_State_T::commonState;
        TextureState* textureState = MG_State_T::textureState;
        BufferState* bufferState = MG_State_T::bufferState;
        VertexArrayState* vaState = MG_State_T::vertexArrayState;
        ProgramState* programState = MG_State_T::programState;
        FramebufferState* fbState = MG_State_T::framebufferState;
        
        // Common
        const GLint* viewport = commonState->viewport;
        if (viewport[2] > 0 && viewport[3] > 0) { 
            CallAndCheck(::GLES::glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);)
        }
        
        if (commonState->capabilities[GL_BLEND]) {
            CallAndCheck(::GLES::glEnable(GL_BLEND);)
            CallAndCheck(::GLES::glBlendFuncSeparate(
                    commonState->blendSrcRGB, commonState->blendDstRGB,
                    commonState->blendSrcAlpha, commonState->blendDstAlpha
            );)
        } else {
            CallAndCheck(::GLES::glDisable(GL_BLEND);)
        }
        
        if (commonState->capabilities[GL_DEPTH_TEST]) {
            CallAndCheck(::GLES::glEnable(GL_DEPTH_TEST);)
            CallAndCheck(::GLES::glDepthMask(commonState->depthMask);)
            CallAndCheck(::GLES::glDepthFunc(commonState->depthFunc);)
        } else {
            CallAndCheck(::GLES::glDisable(GL_DEPTH_TEST);)
        }
        
        if (commonState->capabilities[GL_CULL_FACE]) {
            CallAndCheck(::GLES::glEnable(GL_CULL_FACE);)
        } else {
            CallAndCheck(::GLES::glDisable(GL_CULL_FACE);)
        }
        
        CallAndCheck(::GLES::glColorMask(
                commonState->colorMask[0], commonState->colorMask[1],
                commonState->colorMask[2], commonState->colorMask[3]
        );)
        
        // Texture
        SyncAllTexturesToGLES(textureState);
        for (size_t unitIdx = 0; unitIdx < textureState->textureUnits_.size(); ++unitIdx) {
            TextureUnitState& mgUnit = textureState->textureUnits_[unitIdx];
            GLenum glUnit = GL_TEXTURE0 + unitIdx;
            
            if (lastBoundTextures[unitIdx] != mgUnit.boundTextures[GL_TEXTURE_2D]) {
                CallAndCheck(::GLES::glActiveTexture(glUnit);)
                for (auto& [target, texId] : mgUnit.boundTextures) {
                    if (texId == 0) {
                        CallAndCheck(::GLES::glBindTexture(target, 0);)
                        lastBoundTextures[unitIdx] = 0;
                        continue;
                    }
                    
                    if (s_textureMap.find(texId) == s_textureMap.end()) {
                        GLuint glTexId;
                        CallAndCheck(::GLES::glGenTextures(1, &glTexId);)
                        s_textureMap[texId] = glTexId;
                        
                        CallAndCheck(::GLES::glBindTexture(target, glTexId);)
                        TextureObject& mgTex = textureState->textures[texId];
                        
                        for (auto& [pname, param] : mgTex.params.texPropertiesInt) {
                            CallAndCheck(::GLES::glTexParameteri(target, pname, param);)
                        }
                        for (auto& [pname, param] : mgTex.params.texPropertiesFloat) {
                            CallAndCheck(::GLES::glTexParameterf(target, pname, param);)
                        }
                        
                        for (auto& [level, mip] : mgTex.params.mipmapData) {
                            CallAndCheck(::GLES::glTexImage2D(
                                    target, level, mip.internalFormat,
                                    mip.width, mip.height, 0,
                                    mip.format, mip.type, mip.pixelData.data()
                                    );)
                        }
                    }
                    CallAndCheck(::GLES::glBindTexture(target, s_textureMap[texId]);)
                    lastBoundTextures[unitIdx] = s_textureMap[texId];
                }
            }
        }

        // Buffer
        SyncAllBuffersToGLES(bufferState);


        GLuint vbo = MG_State_T::bufferState->GetCurrentBinding(GL_ARRAY_BUFFER);
        CallAndCheck(::GLES::glBindBuffer(GL_ARRAY_BUFFER, s_bufferMap[vbo]);)

        // VAO
//        GLuint mgVAOId = vaState->currentVao_;
//        VertexArrayObject* vao = &vaState->vaos_[mgVAOId];
        for (auto& [mgid, vao] : vaState->vaos_) {
            if (!vao.generated)
                continue;

            MG_Util::Debug::LogD("Creating MG VAO: %d", mgid);

            // TODO: Check is the VAO changes rather than always update it.
            //if (!s_vaoMap.count(mgVAOId)) {
            GLuint glVAO;
            if (s_vaoMap.find(mgid) == s_vaoMap.end()) {
                CallAndCheck(::GLES::glGenVertexArrays(1, &glVAO);)
                s_vaoMap[mgid] = glVAO;
            } else {
                // VAO already generated, just skip generation
                // TODO: VAO modified along the way?
                continue;
//                glVAO = s_vaoMap[mgid];
            }
            CallAndCheck(::GLES::glBindVertexArray(glVAO);)
            MG_Util::Debug::LogD("Bind VAO (MG -> ES): %d -> %d", mgid, glVAO);

//            std::string name = std::format("MG VAO {}", mgid);
//            ::GLES::glObjectLabel(GL_VERTEX_ARRAY, mgid, name.length(), name.c_str());

            if (vao.elementBuffer != 0 && s_bufferMap.find(vao.elementBuffer) != s_bufferMap.end()) {
                CallAndCheck(::GLES::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_bufferMap[vao.elementBuffer]);)
            }
            MG_Util::Debug::LogD("VAO has %d attributes:", vao.attribs.size());
            for (auto& [index, attrib] : vao.attribs) {
//                if (attrib.buffer != 0 && s_bufferMap.find(attrib.buffer) != s_bufferMap.end()) {
                    MG_Util::Debug::LogD("attrib #%d: size=%d, type=%s, stride=%d, pointer=%d, %s, isInt=%s",
                                         index, attrib.size, MG_Util::Debug::GLEnumToString(attrib.type), attrib.stride, attrib.pointer,
                                         (attrib.enabled ? "enabled" : "disabled"), (attrib.isInteger ? "true" : "false"));

                    if (attrib.isInteger) {
                        CallAndCheck(::GLES::glVertexAttribIPointer(
                                index, attrib.size, attrib.type,
                                attrib.stride, attrib.pointer
                        );)
                    } else {
                        CallAndCheck(::GLES::glVertexAttribPointer(
                                index, attrib.size, attrib.type,
                                attrib.normalized ? GL_TRUE : GL_FALSE,
                                attrib.stride, attrib.pointer
                        );)
                    }

                    if (attrib.enabled) {
                        CallAndCheck(::GLES::glEnableVertexAttribArray(index);)
                    } else {
                        CallAndCheck(::GLES::glDisableVertexAttribArray(index);)
                    }
//                }
            }
            CallAndCheck(::GLES::glBindVertexArray(0);)
            //}
        }
        GLuint currentMgVAO = vaState->currentVao_;
        MG_Util::Debug::LogD("Now binding to VAO %d...", currentMgVAO);
        if (s_vaoMap.find(currentMgVAO) != s_vaoMap.end()) {
            CallAndCheck(::GLES::glBindVertexArray(s_vaoMap[currentMgVAO]);)

//            VertexArrayObject& currentVAO = vaState->vaos_[currentMgVAO];
//            for (auto& [index, attrib] : currentVAO.attribs) {
//                if (attrib.enabled) {
//                    CallAndCheck(::GLES::glEnableVertexAttribArray(index);)
//                } else {
//                    CallAndCheck(::GLES::glDisableVertexAttribArray(index);)
//                }
//            }
        } else {
            CallAndCheck(::GLES::glBindVertexArray(0);)
        }

        // EBO
        GLuint curVaoId = vaState->currentVao_;
        VertexArrayObject* curvao = &vaState->vaos_[curVaoId];
        if (curvao->elementBuffer != 0) {
            if (s_bufferMap.find(curvao->elementBuffer) != s_bufferMap.end()) {
                CallAndCheck(::GLES::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_bufferMap[curvao->elementBuffer]);)
            }
        } else if (indices != nullptr) {
            static GLuint dynamicIBO = 0;
            if (dynamicIBO == 0) {
                CallAndCheck(::GLES::glGenBuffers(1, &dynamicIBO);)
            }
            
            size_t typeSize = 0;
            switch(type) {
                case GL_UNSIGNED_BYTE:  typeSize = sizeof(GLubyte); break;
                case GL_UNSIGNED_SHORT: typeSize = sizeof(GLushort); break;
                case GL_UNSIGNED_INT:   typeSize = sizeof(GLuint); break;
            }
            size_t dataSize = count * typeSize;
            
            CallAndCheck(::GLES::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dynamicIBO);)
            CallAndCheck(::GLES::glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataSize, indices, GL_STREAM_DRAW);)
        }
        
        // Program
        GLuint currentProgram = programState->GetCurrentProgram();
        if (currentProgram != lastBoundProgram) {
            if (s_programMap.find(currentProgram) == s_programMap.end()) {
                GLuint glProgram = ::GLES::glCreateProgram();
                ProgramObject& mgProgram = programState->programs_[currentProgram];

//                std::string name = std::format("MG Program {}", currentProgram);
//                ::GLES::glObjectLabel(GL_PROGRAM, glProgram, name.length(), name.c_str());

                // Attribute Names
                // before vertex shader attach
                for (auto& [name, idx]: mgProgram.attribLocations) {
                    MG_Util::Debug::LogD("%s: location = %d", name.c_str(), idx);
                    CallAndCheck(::GLES::glBindAttribLocation(glProgram, idx, name.c_str());)
                }

                // Shader
                for (GLuint shaderId : mgProgram.attachedShaders) {
                    ShaderObject& mgShader = programState->shaders_[shaderId];
                    GLuint glShader = ::GLES::glCreateShader(mgShader.type);

                    std::string source = MG_Util::Program::CompileSPIRVToGLSL(mgShader.compiledSpirv, 320, true);
                    // Post-processing ESSL
                    source = removeLayoutBinding(source);
                    source = processOutColorLocations(source);
                    source = forceSupporterOutput(source);
                    MG_Util::Debug::LogD(
                            "shader source:\n%s\nConverted source:\n%s\nSpirv Size:%d",
                            mgShader.source.c_str(),source.c_str(),mgShader.compiledSpirv.size()
                    );

                    const char* s[] = { source.c_str() };
                    CallAndCheck(::GLES::glShaderSource(glShader, 1, s, nullptr);)
                    CallAndCheck(::GLES::glCompileShader(glShader);)
                    
                    GLint success;
                    CallAndCheck(::GLES::glGetShaderiv(glShader, GL_COMPILE_STATUS, &success);)
                    if (!success) {
                        GLchar infoLog[512];
                        CallAndCheck(::GLES::glGetShaderInfoLog(glShader, 512, nullptr, infoLog);)
                        MG_Util::Debug::LogE("Shader compilation failed: %s", infoLog);
                    }
                    CallAndCheck(::GLES::glAttachShader(glProgram, glShader);)
                }
                
                CallAndCheck(::GLES::glLinkProgram(glProgram);)
                GLint linkStatus;
                CallAndCheck(::GLES::glGetProgramiv(glProgram, GL_LINK_STATUS, &linkStatus);)
                if (!linkStatus) {
                    GLchar infoLog[512];
                    CallAndCheck(::GLES::glGetProgramInfoLog(glProgram, 512, nullptr, infoLog);)
                    MG_Util::Debug::LogE("Program linking failed: %s", infoLog);
                }
                s_programMap[currentProgram] = glProgram;
            }
            CallAndCheck(::GLES::glUseProgram(s_programMap[currentProgram]);)
            lastBoundProgram = currentProgram;
            
            // Uniform
            ProgramObject& mgProgram = programState->programs_[currentProgram];
            for (auto& [name, uniform] : mgProgram.uniformValues) {
                GLint location = ::GLES::glGetUniformLocation(s_programMap[currentProgram], name.c_str());
                if (location == -1) { 
                    MG_Util::Debug::LogE("Uniform location not found: %s", name.c_str());
                    continue;
                }
                MG_Util::Debug::LogD("Uniform %s(type: %u) location: %u(GLES) -- %u(MG)", name.c_str(), uniform.type, location, mgProgram.uniformLocations[name]);

                switch (uniform.type) {
                    case GL_FLOAT: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 1);
                        CallAndCheck(::GLES::glUniform1fv(location, num, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_VEC2: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 2);
                        CallAndCheck(::GLES::glUniform2fv(location, num, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_VEC3: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 3);
                        CallAndCheck(::GLES::glUniform3fv(location, num, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_VEC4: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 4);
                        CallAndCheck(::GLES::glUniform4fv(location, num, uniform.floatData.data());)
                        break;
                    }

                    case GL_INT:
                    case GL_BOOL:
                    case GL_SAMPLER_1D:
                    case GL_SAMPLER_2D:
                    case GL_SAMPLER_3D:
                    case GL_SAMPLER_CUBE:
                    case GL_SAMPLER_1D_SHADOW:
                    case GL_SAMPLER_2D_SHADOW: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 1);
                        CallAndCheck(::GLES::glUniform1iv(location, num, uniform.intData.data());)
                        break;
                    }
                    case GL_INT_VEC2:
                    case GL_BOOL_VEC2: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 2);
                        CallAndCheck(::GLES::glUniform2iv(location, num, uniform.intData.data());)
                        break;
                    }
                    case GL_INT_VEC3:
                    case GL_BOOL_VEC3: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 3);
                        CallAndCheck(::GLES::glUniform3iv(location, num, uniform.intData.data());)
                        break;
                    }
                    case GL_INT_VEC4:
                    case GL_BOOL_VEC4: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 4);
                        CallAndCheck(::GLES::glUniform4iv(location, num, uniform.intData.data());)
                        break;
                    }

                    case GL_UNSIGNED_INT: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 1);
                        CallAndCheck(::GLES::glUniform1uiv(location, num, uniform.uintData.data());)
                        break;
                    }
                    case GL_UNSIGNED_INT_VEC2: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 2);
                        CallAndCheck(::GLES::glUniform2uiv(location, num, uniform.uintData.data());)
                        break;
                    }
                    case GL_UNSIGNED_INT_VEC3: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 3);
                        CallAndCheck(::GLES::glUniform3uiv(location, num, uniform.uintData.data());)
                        break;
                    }
                    case GL_UNSIGNED_INT_VEC4: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 4);
                        CallAndCheck(::GLES::glUniform4uiv(location, num, uniform.uintData.data());)
                        break;
                    }

                    case GL_FLOAT_MAT2: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 4);
                        CallAndCheck(::GLES::glUniformMatrix2fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT3: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 9);
                        CallAndCheck(::GLES::glUniformMatrix3fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT4: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 16);
                        CallAndCheck(::GLES::glUniformMatrix4fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT2x3: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 6);
                        CallAndCheck(::GLES::glUniformMatrix2x3fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT2x4: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 8);
                        CallAndCheck(::GLES::glUniformMatrix2x4fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT3x2: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 6);
                        CallAndCheck(::GLES::glUniformMatrix3x2fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT3x4: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 12);
                        CallAndCheck(::GLES::glUniformMatrix3x4fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT4x2: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 8);
                        CallAndCheck(::GLES::glUniformMatrix4x2fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT4x3: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 12);
                        CallAndCheck(::GLES::glUniformMatrix4x3fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                }

            }
        }
        
        // Framebuffer
        RealizeFBOState(GL_DRAW_FRAMEBUFFER);
//        GLuint currentFBO = fbState->currentBindings_[GL_DRAW_FRAMEBUFFER];
//        if (currentFBO != lastBoundFBO[0]) {
//            GLuint glFBO = 0;
//
//            if (currentFBO == 0) {
//                CallAndCheck(::GLES::glBindFramebuffer(GL_FRAMEBUFFER, 0);)
//                glFBO = 0;
//            } else {
//                bool isNewGlesFBO = false;
//
//                if (s_framebufferMap.find(currentFBO) == s_framebufferMap.end()) {
//                    CallAndCheck(::GLES::glGenFramebuffers(1, &glFBO);)
//                    s_framebufferMap[currentFBO] = glFBO;
//                    CallAndCheck(::GLES::glBindFramebuffer(GL_FRAMEBUFFER, glFBO);)
//                    MG_Util::Debug::LogD("Generated and bound new GLES FBO %u for MobileGL FBO %u", glFBO, currentFBO);
//                    GLenum status = ::GLES::glCheckFramebufferStatus(GL_FRAMEBUFFER);
//                    if (status != GL_FRAMEBUFFER_COMPLETE) {
//                        MG_Util::Debug::LogE("Framebuffer %u (GLES FBO %u) is not complete after creation! Status: 0x%X (%s)",
//                                             currentFBO, glFBO, status, MG_Util::Debug::GLEnumToString(status));
//                    }
//                    isNewGlesFBO = true;
//                } else {
//                    glFBO = s_framebufferMap[currentFBO];
//                    CallAndCheck(::GLES::glBindFramebuffer(GL_FRAMEBUFFER, glFBO);)
//                    MG_Util::Debug::LogD("Bound existing GLES FBO %u for MobileGL FBO %u", glFBO, currentFBO);
//                }
//
//                if (glFBO != 0) {
//                    FramebufferObject* mgFBO = fbState->GetCurrentFBO(GL_DRAW_FRAMEBUFFER);
//                    if (mgFBO) {
//                        MG_Util::Debug::LogD("Checking/Syncing attachments for GLES FBO %u (MobileGL FBO %u)", glFBO, currentFBO);
//
//                        for (auto const& [mgAttachmentPoint, mgAtt] : mgFBO->attachments) {
//
//                            if (mgAtt.type != GL_TEXTURE_2D) {
//                                MG_Util::Debug::LogW("Skipping non-TEXTURE_2D attachment 0x%X for FBO %u", mgAttachmentPoint, currentFBO);
//                                continue;
//                            }
//
//                            GLuint expectedGLTexId = 0;
//                            if (mgAtt.handle != 0) {
//                                if (s_textureMap.count(mgAtt.handle)) {
//                                    expectedGLTexId = s_textureMap[mgAtt.handle];
//                                } else {
//                                    MG_Util::Debug::LogE("MobileGL Texture %u for FBO %u attachment 0x%X not found in s_textureMap during FBO sync!", mgAtt.handle, currentFBO, mgAttachmentPoint);
//                                    for (auto const& [key, val] : s_textureMap)
//                                        MG_Util::Debug::LogW("  key: %d, val: %d", key, val);
//                                    continue;
//                                }
//                            }
//
//                            GLint glesAttachedType = 0;
//                            GLint glesAttachedName = 0;
//
//                            CallAndCheck(::GLES::glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, mgAttachmentPoint, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &glesAttachedType);)
//
//                            if (glesAttachedType == GL_TEXTURE) {
//                                CallAndCheck(::GLES::glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, mgAttachmentPoint, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &glesAttachedName);)
//                            } else if (glesAttachedType != GL_NONE) {
//                                MG_Util::Debug::LogW("GLES FBO %u attachment 0x%X has non-texture type 0x%X (expected GL_TEXTURE or GL_NONE)", glFBO, mgAttachmentPoint, glesAttachedType);
//                            }
//
//                            if ((GLuint)glesAttachedName != expectedGLTexId) {
//                                MG_Util::Debug::LogD("Syncing FBO %u attachment 0x%X: Expected GLES TexID %u, Found GLES ObjName %d. Attaching/Detaching...",
//                                                     currentFBO, mgAttachmentPoint, expectedGLTexId, glesAttachedName);
//
//                                CallAndCheck(::GLES::glFramebufferTexture2D(
//                                        GL_FRAMEBUFFER,
//                                        mgAttachmentPoint,
//                                        GL_TEXTURE_2D,
//                                        expectedGLTexId,
//                                        mgAtt.mipLevel
//                                );)
//                            }
//                        }
//                        GLenum status = ::GLES::glCheckFramebufferStatus(GL_FRAMEBUFFER);
//                        if (status != GL_FRAMEBUFFER_COMPLETE) {
//                            MG_Util::Debug::LogE("Framebuffer %u (GLES FBO %u) is not complete after sync! Status: 0x%X (%s)",
//                                                 currentFBO, glFBO, status, MG_Util::Debug::GLEnumToString(status));
//                        }
//
//                    } else {
//                        MG_Util::Debug::LogW("Could not get MobileGL FBO object for ID %u during attachment sync.", currentFBO);
//                    }
//                }
//            }
//
//            lastBoundFBO[0] = currentFBO;
//        }



        if (curvao->elementBuffer != 0 || indices != nullptr) {
            CallAndCheck(::GLES::glDrawElements(
                    mode,
                    count,
                    type,
                    indices
            );)
        }
    }

    void BlitFramebuffer(GLint srcX0,
                         GLint srcY0,
                         GLint srcX1,
                         GLint srcY1,
                         GLint dstX0,
                         GLint dstY0,
                         GLint dstX1,
                         GLint dstY1,
                         GLbitfield mask,
                         GLenum filter) {
        MG_Util::Debug::LogD("BlitFramebuffer, srcX0=%d, srcY0=%d, srcX1=%d, srcY1=%d, dstX0=%d, dstY0=%d, dstX1=%d, dstY1=%d, mask=0x%x, filter=%s",
                             srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, MG_Util::Debug::GLEnumToString(filter));
        TextureState* textureState = MG_State_T::textureState;

        // Texture
        SyncAllTexturesToGLES(textureState);

        // Realize FBO states
        RealizeFBOState(GL_DRAW_FRAMEBUFFER);
        RealizeFBOState(GL_READ_FRAMEBUFFER);
        ::GLES::glBlitFramebuffer(srcX0,
                                  srcY0,
                                  srcX1,
                                  srcY1,
                                  dstX0,
                                  dstY0,
                                  dstX1,
                                  dstY1,
                                  mask,
                                  filter);
    }

    void ClearSHITTILY(GLbitfield mask) {
        CommonState* commonState = MG_State_T::commonState;
        FramebufferState* fbState = MG_State_T::framebufferState;

//        GLuint currentFBO = fbState->currentBindings_[GL_DRAW_FRAMEBUFFER];
//        if (currentFBO != lastBoundFBO[0]) {
//            GLuint glFBO = 0;
//
//            if (currentFBO == 0) {
//                CallAndCheck(::GLES::glBindFramebuffer(GL_FRAMEBUFFER, 0);)
//                glFBO = 0;
//            } else {
//                bool isNewGlesFBO = false;
//
//                if (s_framebufferMap.find(currentFBO) == s_framebufferMap.end()) {
//                    CallAndCheck(::GLES::glGenFramebuffers(1, &glFBO);)
//                    s_framebufferMap[currentFBO] = glFBO;
//                    CallAndCheck(::GLES::glBindFramebuffer(GL_FRAMEBUFFER, glFBO);)
//                    MG_Util::Debug::LogD("Generated and bound new GLES FBO %u for MobileGL FBO %u", glFBO, currentFBO);
//                    isNewGlesFBO = true;
//                } else {
//                    glFBO = s_framebufferMap[currentFBO];
//                    CallAndCheck(::GLES::glBindFramebuffer(GL_FRAMEBUFFER, glFBO);)
//                    MG_Util::Debug::LogD("Bound existing GLES FBO %u for MobileGL FBO %u", glFBO, currentFBO);
//                }
//
//                if (glFBO != 0) {
//                    FramebufferObject* mgFBO = fbState->GetCurrentFBO(GL_DRAW_FRAMEBUFFER);
//                    if (mgFBO) {
//                        MG_Util::Debug::LogD("Checking/Syncing attachments for GLES FBO %u (MobileGL FBO %u)", glFBO, currentFBO);
//
//                        for (auto const& [mgAttachmentPoint, mgAtt] : mgFBO->attachments) {
//
//                            if (mgAtt.type != GL_TEXTURE_2D) {
//                                MG_Util::Debug::LogW("Skipping non-TEXTURE_2D attachment 0x%X for FBO %u", mgAttachmentPoint, currentFBO);
//                                continue;
//                            }
//
//                            GLuint expectedGLTexId = 0;
//                            if (mgAtt.handle != 0) {
//                                if (s_textureMap.count(mgAtt.handle)) {
//                                    expectedGLTexId = s_textureMap[mgAtt.handle];
//                                } else {
//                                    MG_Util::Debug::LogE("MobileGL Texture %u for FBO %u attachment 0x%X not found in s_textureMap during FBO sync!", mgAtt.handle, currentFBO, mgAttachmentPoint);
//                                    for (auto const& [key, val] : s_textureMap)
//                                        MG_Util::Debug::LogW("  key: %d, val: %d", key, val);
//                                    continue;
//                                }
//                            }
//
//                            GLint glesAttachedType = 0;
//                            GLint glesAttachedName = 0;
//
//                            CallAndCheck(::GLES::glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, mgAttachmentPoint, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &glesAttachedType);)
//
//                            if (glesAttachedType == GL_TEXTURE) {
//                                CallAndCheck(::GLES::glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, mgAttachmentPoint, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &glesAttachedName);)
//                            } else if (glesAttachedType != GL_NONE) {
//                                MG_Util::Debug::LogW("GLES FBO %u attachment 0x%X has non-texture type 0x%X (expected GL_TEXTURE or GL_NONE)", glFBO, mgAttachmentPoint, glesAttachedType);
//                            }
//
//                            if ((GLuint)glesAttachedName != expectedGLTexId) {
//                                MG_Util::Debug::LogD("Syncing FBO %u attachment 0x%X: Expected GLES TexID %u, Found GLES ObjName %d. Attaching/Detaching...",
//                                                     currentFBO, mgAttachmentPoint, expectedGLTexId, glesAttachedName);
//
//                                CallAndCheck(::GLES::glFramebufferTexture2D(
//                                        GL_FRAMEBUFFER,
//                                        mgAttachmentPoint,
//                                        GL_TEXTURE_2D,
//                                        expectedGLTexId,
//                                        mgAtt.mipLevel
//                                );)
//                            }
//                        }
//
//                        GLenum status = ::GLES::glCheckFramebufferStatus(GL_FRAMEBUFFER);
//                        if (status != GL_FRAMEBUFFER_COMPLETE) {
//                            MG_Util::Debug::LogE("Framebuffer %u (GLES FBO %u) is not complete after sync! Status: 0x%X (%s)",
//                                                 currentFBO, glFBO, status, MG_Util::Debug::GLEnumToString(status));
//                        }
//
//                    } else {
//                        MG_Util::Debug::LogW("Could not get MobileGL FBO object for ID %u during attachment sync.", currentFBO);
//                    }
//                }
//            }
//
//            lastBoundFBO[0] = currentFBO;
//        }

        TextureState* textureState = MG_State_T::textureState;

        // Texture
        SyncAllTexturesToGLES(textureState);

        RealizeFBOState(GL_DRAW_FRAMEBUFFER);

        static GLfloat lastClearColor[4] = {-1.0f, -1.0f, -1.0f, -1.0f};
        if (memcmp(lastClearColor, commonState->clearColor, sizeof(lastClearColor)) != 0) {
            CallAndCheck(::GLES::glClearColor(
                    commonState->clearColor[0],
                    commonState->clearColor[1],
                    commonState->clearColor[2],
                    commonState->clearColor[3]
            );)
            memcpy(lastClearColor, commonState->clearColor, sizeof(lastClearColor));
        }

        static GLfloat lastClearDepth = -1.0f;
        if (lastClearDepth != commonState->clearDepth) {
            CallAndCheck(::GLES::glClearDepthf(commonState->clearDepth);)
            lastClearDepth = commonState->clearDepth;
        }

        static GLint lastClearStencil = -1;
        GLint currentStencil = 0;
        if (lastClearStencil != currentStencil) {
            CallAndCheck(::GLES::glClearStencil(currentStencil);)
            lastClearStencil = currentStencil;
        }
        
        const GLint* viewport = commonState->viewport;
        if (viewport[2] > 0 && viewport[3] > 0) {
            CallAndCheck(::GLES::glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);)
        }
        
        if (commonState->capabilities[GL_BLEND]) {
            CallAndCheck(::GLES::glEnable(GL_BLEND);)
            CallAndCheck(::GLES::glBlendFuncSeparate(
                    commonState->blendSrcRGB, commonState->blendDstRGB,
                    commonState->blendSrcAlpha, commonState->blendDstAlpha
            );)
        } else {
            CallAndCheck(::GLES::glDisable(GL_BLEND);)
        }
        
        if (commonState->capabilities[GL_DEPTH_TEST]) {
            CallAndCheck(::GLES::glEnable(GL_DEPTH_TEST);)
            CallAndCheck(::GLES::glDepthMask(commonState->depthMask);)
            CallAndCheck(::GLES::glDepthFunc(commonState->depthFunc);)
        } else {
            CallAndCheck(::GLES::glDisable(GL_DEPTH_TEST);)
        }
        
        if (commonState->capabilities[GL_CULL_FACE]) {
            CallAndCheck(::GLES::glEnable(GL_CULL_FACE);)
        } else {
            CallAndCheck(::GLES::glDisable(GL_CULL_FACE);)
        }
        
        CallAndCheck(::GLES::glColorMask(
                commonState->colorMask[0], commonState->colorMask[1],
                commonState->colorMask[2], commonState->colorMask[3]
        );)

        CallAndCheck(::GLES::glClear(mask);)

    }
    
    void Clear(GLbitfield mask) {
        MG_Util::Debug::LogD("glClear, mask: 0x%X", mask);
        GLenum result = MG_State::glClear(mask);
        if (result == GL_NO_ERROR) {
            ClearSHITTILY(mask);
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error clearing buffers: %s", MG_Util::Debug::GLEnumToString(result));
    }
    
    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices) {
        DrawElementsSHITTILY(mode, count, type, indices);
    }
    
    void DrawArrays(GLenum mode, GLint first, GLsizei count) {
        DrawArraysSHITTILY(mode, first, count);
    }
}