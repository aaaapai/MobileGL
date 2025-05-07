//
// Created by Swung 0x48 on 2025/5/6.
//

#include "RHI.h"
#undef MOBILEGL_PROGRAM_DEBUGTOOL_H
#include "../../MG_UTIL/Program/DebugTool.h"

namespace MG_RHI::GLES {
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

    void RHI::CheckError() {
        while (GLenum err = ::GLES::glGetError() != GL_NO_ERROR) {
            MG_Util::Debug::LogE("-> GLES Error: %s", MG_Util::Debug::GLEnumToString(err));
        }
    }

    void RHI::SyncAllTexturesToGLES(TextureState* textureState) {
        MG_Util::Debug::LogD("Syncing all textures to GLES...");
        for (auto& [mgTexId, texObj] : textureState->textures) {
            if (!texObj.generated) continue;

            if (textureMap_.find(mgTexId) == textureMap_.end()) {
                GLuint glTexId;
                MGL_TRY(::GLES::glGenTextures(1, &glTexId);)
                textureMap_[mgTexId] = glTexId;
                GLenum target = texObj.target;
                MGL_TRY(::GLES::glBindTexture(target, glTexId);)

                for (auto& [pname, param] : texObj.params.texPropertiesInt) {
                    MGL_TRY(::GLES::glTexParameteri(target, pname, param);)
                }
                for (auto& [pname, param] : texObj.params.texPropertiesFloat) {
                    MGL_TRY(::GLES::glTexParameterf(target, pname, param);)
                }

                for (auto& [level, mip] : texObj.params.mipmapData) {
                    const void* data = !mip.pixelData.empty() ? mip.pixelData.data() : nullptr;
                    switch (target) {
                        case GL_TEXTURE_2D:
                            MGL_TRY(::GLES::glTexImage2D(
                                    target, level, mip.internalFormat,
                                    mip.width, mip.height, 0,
                                    mip.format, mip.type, data
                            );)
                            textureLevelUploaded_[mgTexId][level] = true;
                            MG_Util::Debug::LogD("Initial upload texture %u level %d (size=%zu)", mgTexId, level, mip.pixelData.size());
                            break;
                        default:
                            MG_Util::Debug::LogE("Unsupported target: %s", MG_Util::Debug::GLEnumToString(target));
                    }
                }
                MG_Util::Debug::LogD("Created and synced new GLES texture %u (MobileGL ID)", mgTexId);
            } else {
                GLuint glTexId = textureMap_[mgTexId];
                GLenum target = texObj.target;
                MGL_TRY(::GLES::glBindTexture(target, glTexId);)

                for (auto& [level, mip] : texObj.params.mipmapData) {
                    if (!textureLevelUploaded_[mgTexId][level]) {
                        bool levelInitialized = textureLevelUploaded_[mgTexId].count(level);
                        const void* data = !mip.pixelData.empty() ? mip.pixelData.data() : nullptr;

                        switch (target) {
                            case GL_TEXTURE_2D:
                                if (levelInitialized) {
                                    MGL_TRY(::GLES::glTexSubImage2D(
                                            target, level,
                                            0, 0,
                                            mip.width, mip.height,
                                            mip.format, mip.type, data
                                    );)
                                    textureLevelUploaded_[mgTexId][level] = true;
                                    MG_Util::Debug::LogD("Updated texture %u level %d with glTexSubImage2D", mgTexId, level);
                                } else {
                                    MGL_TRY(::GLES::glTexImage2D(
                                            target, level, mip.internalFormat,
                                            mip.width, mip.height, 0,
                                            mip.format, mip.type, data
                                    );)
                                    textureLevelUploaded_[mgTexId][level] = true;
                                    MG_Util::Debug::LogD("Initialized texture %u level %d with glTexImage2D", mgTexId, level);
                                }
                                break;
                            default:
                                MG_Util::Debug::LogE("Unsupported target: %s", MG_Util::Debug::GLEnumToString(target));
                        }
                    }
                }
                MG_Util::Debug::LogD("Updated existing GLES texture %u (MobileGL ID)", mgTexId);
            }
        }
        MGL_TRY(::GLES::glBindTexture(GL_TEXTURE_2D, 0);)
    }

    void RHI::SyncAllBuffersToGLES(BufferState* bufferState) {
        GLint currentVBO = 0, currentEBO = 0;
        MGL_TRY(::GLES::glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);)
        MGL_TRY(::GLES::glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &currentEBO);)
        for (auto& [mgBufferId, bufferObj] : bufferState->buffers_) {
            if (!bufferObj.generated) continue;

            // TODO: Check if the buffer changes rather than always update it.
            if (bufferMap_.find(mgBufferId) == bufferMap_.end()) {
                if (bufferMap_.find(mgBufferId) == bufferMap_.end()) {
                    GLuint glBuffer;
                    MGL_TRY(::GLES::glGenBuffers(1, &glBuffer);)
                    bufferMap_[mgBufferId] = glBuffer;
                }
                MGL_TRY(::GLES::glBindBuffer(bufferObj.target, bufferMap_[mgBufferId]);)
                MGL_TRY(::GLES::glBufferData(
                        bufferObj.target,
                        bufferObj.data.size(),
                        bufferObj.data.data(),
                        bufferObj.usage
                );)
            }
        }
        MGL_TRY(::GLES::glBindBuffer(GL_ARRAY_BUFFER, currentVBO);)
        MGL_TRY(::GLES::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentEBO);)
    }

    void RHI::TransitionTo(State state) {
        currentState_ = state;

        CommonState* commonState = MG_State_T::commonState;
        TextureState* textureState = MG_State_T::textureState;
        BufferState* bufferState = MG_State_T::bufferState;
        VertexArrayState* vaState = MG_State_T::vertexArrayState;
        ProgramState* programState = MG_State_T::programState;
        FramebufferState* fbState = MG_State_T::framebufferState;

        const GLint* viewport = commonState->viewport;
        if (viewport[2] > 0 && viewport[3] > 0) {
            MGL_TRY(::GLES::glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);)
        }

        if (commonState->capabilities[GL_BLEND]) {
            MGL_TRY(::GLES::glEnable(GL_BLEND);)
            MGL_TRY(::GLES::glBlendFuncSeparate(
                    commonState->blendSrcRGB, commonState->blendDstRGB,
                    commonState->blendSrcAlpha, commonState->blendDstAlpha
            );)
        } else {
            MGL_TRY(::GLES::glDisable(GL_BLEND);)
        }

        if (commonState->capabilities[GL_DEPTH_TEST]) {
            MGL_TRY(::GLES::glEnable(GL_DEPTH_TEST);)
            MGL_TRY(::GLES::glDepthMask(commonState->depthMask);)
            MGL_TRY(::GLES::glDepthFunc(commonState->depthFunc);)
        } else {
            MGL_TRY(::GLES::glDisable(GL_DEPTH_TEST);)
        }

        if (commonState->capabilities[GL_CULL_FACE]) {
            MGL_TRY(::GLES::glEnable(GL_CULL_FACE);)
        } else {
            MGL_TRY(::GLES::glDisable(GL_CULL_FACE);)
        }

        MGL_TRY(::GLES::glColorMask(
                commonState->colorMask[0], commonState->colorMask[1],
                commonState->colorMask[2], commonState->colorMask[3]);)

        // Texture
        SyncAllTexturesToGLES(textureState);
        for (size_t unitIdx = 0; unitIdx < textureState->textureUnits_.size(); ++unitIdx) {
            TextureUnitState& mgUnit = textureState->textureUnits_[unitIdx];
            GLenum glUnit = GL_TEXTURE0 + unitIdx;

            if (lastBoundTextures[unitIdx] != mgUnit.boundTextures[GL_TEXTURE_2D]) {
                MGL_TRY(::GLES::glActiveTexture(glUnit);)
                for (auto& [target, texId] : mgUnit.boundTextures) {
                    if (texId == 0) {
                        MGL_TRY(::GLES::glBindTexture(target, 0);)
                        lastBoundTextures[unitIdx] = 0;
                        continue;
                    }

                    if (textureMap_.find(texId) == textureMap_.end()) {
                        GLuint glTexId;
                        MGL_TRY(::GLES::glGenTextures(1, &glTexId);)
                        textureMap_[texId] = glTexId;

                        MGL_TRY(::GLES::glBindTexture(target, glTexId);)
                        TextureObject& mgTex = textureState->textures[texId];

                        for (auto& [pname, param] : mgTex.params.texPropertiesInt) {
                            MGL_TRY(::GLES::glTexParameteri(target, pname, param);)
                        }
                        for (auto& [pname, param] : mgTex.params.texPropertiesFloat) {
                            MGL_TRY(::GLES::glTexParameterf(target, pname, param);)
                        }

                        for (auto& [level, mip] : mgTex.params.mipmapData) {
                            MGL_TRY(::GLES::glTexImage2D(
                                    target, level, mip.internalFormat,
                                    mip.width, mip.height, 0,
                                    mip.format, mip.type, mip.pixelData.data()
                            );)
                        }
                    }
                    MGL_TRY(::GLES::glBindTexture(target, textureMap_[texId]);)
                    lastBoundTextures[unitIdx] = textureMap_[texId];
                }
            }
        }

        // Buffer
        SyncAllBuffersToGLES(bufferState);

        // VAO
        GLuint mgVAOId = vaState->currentVao_;
        VertexArrayObject* vao = &vaState->vaos_[mgVAOId];
        for (auto& [mgVAOId, mgVAO] : vaState->vaos_) {
            MG_Util::Debug::LogD("Uploading VAO: %d", mgVAOId);
            if (!mgVAO.generated) continue;

            // TODO: Check is the VAO changes rather than always update it.
            GLuint glVAO;
            if (vaoMap_.find(mgVAOId) == vaoMap_.end()) {
                MGL_TRY(::GLES::glGenVertexArrays(1, &glVAO);)
                MG_Util::Debug::LogD("Saving VAO: %d -> %d", mgVAOId, glVAO);
                vaoMap_[mgVAOId] = glVAO;
            } else {
                glVAO = vaoMap_[mgVAOId];
            }
            MGL_TRY(::GLES::glBindVertexArray(glVAO);)

            if (mgVAO.elementBuffer != 0 && bufferMap_.count(mgVAO.elementBuffer)) {
                MGL_TRY(::GLES::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferMap_[mgVAO.elementBuffer]);)
            }

            for (auto& [index, attrib] : mgVAO.attribs) {
                MG_Util::Debug::LogD("attrib #%d, type = %s, size = %d, stride = %d, pointer = %p",
                                     index, MG_Util::Debug::GLEnumToString(attrib.type), attrib.size, attrib.stride, attrib.pointer);
                if (attrib.buffer != 0 && bufferMap_.count(attrib.buffer)) {
                    MGL_TRY(::GLES::glBindBuffer(GL_ARRAY_BUFFER, bufferMap_[attrib.buffer]);)

                    if (attrib.isInteger) {
                        MGL_TRY(::GLES::glVertexAttribIPointer(
                                index, attrib.size, attrib.type,
                                attrib.stride, attrib.pointer
                        );)
                    } else {
                        MGL_TRY(::GLES::glVertexAttribPointer(
                                index, attrib.size, attrib.type,
                                attrib.normalized ? GL_TRUE : GL_FALSE,
                                attrib.stride, attrib.pointer
                        );)
                    }

                    if (attrib.enabled) {
                        MGL_TRY(::GLES::glEnableVertexAttribArray(index);)
                    } else {
                        MGL_TRY(::GLES::glDisableVertexAttribArray(index);)
                    }
                }
            }
            MGL_TRY(::GLES::glBindVertexArray(0);)
            //}
        }
//        GLuint currentMgVAO = vaState->currentVao_;
        if (vaoMap_.find(mgVAOId) != vaoMap_.end()) {
            MGL_TRY(::GLES::glBindVertexArray(vaoMap_[mgVAOId]);)

            VertexArrayObject& currentVAO = vaState->vaos_[mgVAOId];
            for (auto& [index, attrib] : currentVAO.attribs) {
                if (attrib.enabled) {
                    MGL_TRY(::GLES::glEnableVertexAttribArray(index);)
                } else {
                    MGL_TRY(::GLES::glDisableVertexAttribArray(index);)
                }
            }
        } else {
            MGL_TRY(::GLES::glBindVertexArray(0);)
        }


        // EBO
        if (vao->elementBuffer != 0) {
            if (bufferMap_.count(vao->elementBuffer)) {
                MGL_TRY(::GLES::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferMap_[vao->elementBuffer]);)
            }
        }

        // Program
        GLuint currentProgram = programState->GetCurrentProgram();
        if (currentProgram != lastBoundProgram) {
            if (programMap_.find(currentProgram) == programMap_.end()) {
                GLuint glProgram = ::GLES::glCreateProgram();
                ProgramObject& mgProgram = programState->programs_[currentProgram];

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
                    MGL_TRY(::GLES::glShaderSource(glShader, 1, s, nullptr);)
                    MGL_TRY(::GLES::glCompileShader(glShader);)

                    GLint success;
                    MGL_TRY(::GLES::glGetShaderiv(glShader, GL_COMPILE_STATUS, &success);)
                    if (!success) {
                        GLchar infoLog[512];
                        MGL_TRY(::GLES::glGetShaderInfoLog(glShader, 512, nullptr, infoLog);)
                        MG_Util::Debug::LogE("Shader compilation failed: %s", infoLog);
                    }
                    MGL_TRY(::GLES::glAttachShader(glProgram, glShader);)
                }

                MGL_TRY(::GLES::glLinkProgram(glProgram);)
                GLint linkStatus;
                MGL_TRY(::GLES::glGetProgramiv(glProgram, GL_LINK_STATUS, &linkStatus);)
                if (!linkStatus) {
                    GLchar infoLog[512];
                    MGL_TRY(::GLES::glGetProgramInfoLog(glProgram, 512, nullptr, infoLog);)
                    MG_Util::Debug::LogE("Program linking failed: %s", infoLog);
                }
                programMap_[currentProgram] = glProgram;
            }
            MGL_TRY(::GLES::glUseProgram(programMap_[currentProgram]);)
            lastBoundProgram = currentProgram;

            // Uniform
            ProgramObject& mgProgram = programState->programs_[currentProgram];
            for (auto& [name, uniform] : mgProgram.uniformValues) {
                GLint location = ::GLES::glGetUniformLocation(programMap_[currentProgram], name.c_str());
                if (location == -1) {
                    MG_Util::Debug::LogE("Uniform location not found: %s", name.c_str());
                    continue;
                }
                MG_Util::Debug::LogD("Uniform %s(type: %u) location: %u(GLES) -- %u(MG)", name.c_str(), uniform.type, location, mgProgram.uniformLocations[name]);

                switch (uniform.type) {
                    case GL_FLOAT: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 1);
                        MGL_TRY(::GLES::glUniform1fv(location, num, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_VEC2: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 2);
                        MGL_TRY(::GLES::glUniform2fv(location, num, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_VEC3: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 3);
                        MGL_TRY(::GLES::glUniform3fv(location, num, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_VEC4: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 4);
                        MGL_TRY(::GLES::glUniform4fv(location, num, uniform.floatData.data());)
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
                        MGL_TRY(::GLES::glUniform1iv(location, num, uniform.intData.data());)
                        break;
                    }
                    case GL_INT_VEC2:
                    case GL_BOOL_VEC2: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 2);
                        MGL_TRY(::GLES::glUniform2iv(location, num, uniform.intData.data());)
                        break;
                    }
                    case GL_INT_VEC3:
                    case GL_BOOL_VEC3: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 3);
                        MGL_TRY(::GLES::glUniform3iv(location, num, uniform.intData.data());)
                        break;
                    }
                    case GL_INT_VEC4:
                    case GL_BOOL_VEC4: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 4);
                        MGL_TRY(::GLES::glUniform4iv(location, num, uniform.intData.data());)
                        break;
                    }

                    case GL_UNSIGNED_INT: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 1);
                        MGL_TRY(::GLES::glUniform1uiv(location, num, uniform.uintData.data());)
                        break;
                    }
                    case GL_UNSIGNED_INT_VEC2: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 2);
                        MGL_TRY(::GLES::glUniform2uiv(location, num, uniform.uintData.data());)
                        break;
                    }
                    case GL_UNSIGNED_INT_VEC3: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 3);
                        MGL_TRY(::GLES::glUniform3uiv(location, num, uniform.uintData.data());)
                        break;
                    }
                    case GL_UNSIGNED_INT_VEC4: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 4);
                        MGL_TRY(::GLES::glUniform4uiv(location, num, uniform.uintData.data());)
                        break;
                    }

                    case GL_FLOAT_MAT2: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 4);
                        MGL_TRY(::GLES::glUniformMatrix2fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT3: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 9);
                        MGL_TRY(::GLES::glUniformMatrix3fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT4: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 16);
                        MGL_TRY(::GLES::glUniformMatrix4fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT2x3: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 6);
                        MGL_TRY(::GLES::glUniformMatrix2x3fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT2x4: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 8);
                        MGL_TRY(::GLES::glUniformMatrix2x4fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT3x2: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 6);
                        MGL_TRY(::GLES::glUniformMatrix3x2fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT3x4: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 12);
                        MGL_TRY(::GLES::glUniformMatrix3x4fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT4x2: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 8);
                        MGL_TRY(::GLES::glUniformMatrix4x2fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                    case GL_FLOAT_MAT4x3: {
                        GLsizei num = static_cast<GLsizei>(uniform.count / 12);
                        MGL_TRY(::GLES::glUniformMatrix4x3fv(location, num, GL_FALSE, uniform.floatData.data());)
                        break;
                    }
                }
            }

            // Framebuffer
            GLuint currentFBO = fbState->currentBindings_[GL_DRAW_FRAMEBUFFER];
            if (currentFBO != lastBoundFBO) {
                GLuint glFBO = 0;

                if (currentFBO == 0) {
                    MGL_TRY(::GLES::glBindFramebuffer(GL_FRAMEBUFFER, 0);)
                    glFBO = 0;
                } else {
                    bool isNewGlesFBO = false;

                    if (framebufferMap_.find(currentFBO) == framebufferMap_.end()) {
                        MGL_TRY(::GLES::glGenFramebuffers(1, &glFBO);)
                        framebufferMap_[currentFBO] = glFBO;
                        MGL_TRY(::GLES::glBindFramebuffer(GL_FRAMEBUFFER, glFBO);)
                        MG_Util::Debug::LogD("Generated and bound new GLES FBO %u for MobileGL FBO %u", glFBO, currentFBO);
                        GLenum status = ::GLES::glCheckFramebufferStatus(GL_FRAMEBUFFER);
                        if (status != GL_FRAMEBUFFER_COMPLETE) {
                            MG_Util::Debug::LogE("Framebuffer %u (GLES FBO %u) is not complete after creation! Status: 0x%X (%s)",
                                                 currentFBO, glFBO, status, MG_Util::Debug::GLEnumToString(status));
                        }
                        isNewGlesFBO = true;
                    } else {
                        glFBO = framebufferMap_[currentFBO];
                        MGL_TRY(::GLES::glBindFramebuffer(GL_FRAMEBUFFER, glFBO);)
                        MG_Util::Debug::LogD("Bound existing GLES FBO %u for MobileGL FBO %u", glFBO, currentFBO);
                    }

                    if (glFBO != 0) {
                        FramebufferObject* mgFBO = fbState->GetCurrentFBO(GL_DRAW_FRAMEBUFFER);
                        if (mgFBO) {
                            MG_Util::Debug::LogD("Checking/Syncing attachments for GLES FBO %u (MobileGL FBO %u)", glFBO, currentFBO);

                            for (auto const& [mgAttachmentPoint, mgAtt] : mgFBO->attachments) {

                                if (mgAtt.type != GL_TEXTURE_2D) {
                                    MG_Util::Debug::LogW("Skipping non-TEXTURE_2D attachment 0x%X for FBO %u", mgAttachmentPoint, currentFBO);
                                    continue;
                                }

                                GLuint expectedGLTexId = 0;
                                if (mgAtt.handle != 0) {
                                    if (textureMap_.count(mgAtt.handle)) {
                                        expectedGLTexId = textureMap_[mgAtt.handle];
                                    } else {
                                        MG_Util::Debug::LogE("MobileGL Texture %u for FBO %u attachment 0x%X not found in textureMap_ during FBO sync!", mgAtt.handle, currentFBO, mgAttachmentPoint);
                                        for (auto const& [key, val] : textureMap_)
                                            MG_Util::Debug::LogW("  key: %d, val: %d", key, val);
                                        continue;
                                    }
                                }

                                GLint glesAttachedType = 0;
                                GLint glesAttachedName = 0;

                                MGL_TRY(::GLES::glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, mgAttachmentPoint, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &glesAttachedType);)

                                if (glesAttachedType == GL_TEXTURE) {
                                    MGL_TRY(::GLES::glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, mgAttachmentPoint, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &glesAttachedName);)
                                } else if (glesAttachedType != GL_NONE) {
                                    MG_Util::Debug::LogW("GLES FBO %u attachment 0x%X has non-texture type 0x%X (expected GL_TEXTURE or GL_NONE)", glFBO, mgAttachmentPoint, glesAttachedType);
                                }

                                if ((GLuint)glesAttachedName != expectedGLTexId) {
                                    MG_Util::Debug::LogD("Syncing FBO %u attachment 0x%X: Expected GLES TexID %u, Found GLES ObjName %d. Attaching/Detaching...",
                                                         currentFBO, mgAttachmentPoint, expectedGLTexId, glesAttachedName);

                                    MGL_TRY(::GLES::glFramebufferTexture2D(
                                            GL_FRAMEBUFFER,
                                            mgAttachmentPoint,
                                            GL_TEXTURE_2D,
                                            expectedGLTexId,
                                            mgAtt.mipLevel
                                    );)
                                }
                            }
                            GLenum status = ::GLES::glCheckFramebufferStatus(GL_FRAMEBUFFER);
                            if (status != GL_FRAMEBUFFER_COMPLETE) {
                                MG_Util::Debug::LogE("Framebuffer %u (GLES FBO %u) is not complete after sync! Status: 0x%X (%s)",
                                                     currentFBO, glFBO, status, MG_Util::Debug::GLEnumToString(status));
                            }

                        } else {
                            MG_Util::Debug::LogW("Could not get MobileGL FBO object for ID %u during attachment sync.", currentFBO);
                        }
                    }
                }

                lastBoundFBO = currentFBO;
            }
        }
    }
}