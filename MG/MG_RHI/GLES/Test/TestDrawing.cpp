//
// Created by BZLZHH on 2025/4/26.
//

#include "TestDrawing.h"

namespace MG_RHI::GLES::Test {
    bool IsTextureComplete(const TextureObject& tex) {
        if (tex.params.mipmapData.empty()) return false;
        return true;
    }

    void DrawAllTextures() {
        struct ScreenResources {
            GLuint program = 0;
            GLuint quadVAO = 0;
            GLuint quadVBO = 0;
            std::unordered_map<GLuint, GLuint> textureCache;  // [state_texID] => GL_texID
        };

        static ScreenResources sRes;
        static glm::ivec2 lastScreenSize{0, 0};

        GLint viewport[4];
        ::GLES::glGetIntegerv(GL_VIEWPORT, viewport);
        glm::ivec2 screenSize(viewport[2], viewport[3]);

        if (screenSize != lastScreenSize) {
            if (sRes.quadVAO) {
                ::GLES::glDeleteVertexArrays(1, &sRes.quadVAO);
                ::GLES::glDeleteBuffers(1, &sRes.quadVBO);
                sRes.quadVAO = sRes.quadVBO = 0;
            }
            lastScreenSize = screenSize;
        }

        if (!sRes.program) {
            const char* vsSrc = R"(#version 320 es
            layout(location=0) in vec2 aPos;
            layout(location=1) in vec2 aTexUV;
            out vec2 vUV;
            uniform mat4 uMVP;
            void main() {
                gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
                vUV = aTexUV;
            })";

            const char* fsSrc = R"(#version 320 es
            precision mediump float;
            in vec2 vUV;
            uniform sampler2D uTex;
            out vec4 FragColor;
            void main() {
                FragColor = texture(uTex, vUV);
            })";

            GLuint vs = glCreateShader(GL_VERTEX_SHADER);
            GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
            ::GLES::glShaderSource(vs, 1, &vsSrc, NULL);
            ::GLES::glShaderSource(fs, 1, &fsSrc, NULL);
            ::GLES::glCompileShader(vs);
            ::GLES::glCompileShader(fs);
            sRes.program = glCreateProgram();
            ::GLES::glAttachShader(sRes.program, vs);
            ::GLES::glAttachShader(sRes.program, fs);
            ::GLES::glLinkProgram(sRes.program);
            ::GLES::glDeleteShader(vs);
            ::GLES::glDeleteShader(fs);
        }

        if (!sRes.quadVAO) {
            const float margin = 0.02f;
            const float gridSize = (1.0f - 2*margin) / 6.0f;

            std::vector<float> vertices;

            for (int col = 0; col < 6; ++col) {
                for (int row = 0; row < 5; ++row) {
                    float xStart = -1.0f + margin + col * gridSize;
                    float yStart = -1.0f + margin + row * gridSize;
                    float xEnd = xStart + gridSize - margin;
                    float yEnd = yStart + gridSize - margin;

                    vertices.insert(vertices.end(), {
                            xStart, yStart,    0.0f, 0.0f,
                            xEnd,   yStart,    1.0f, 0.0f,
                            xEnd,   yEnd,      1.0f, 1.0f,
                            xStart, yEnd,      0.0f, 1.0f
                    });
                }
            }

            ::GLES::glGenVertexArrays(1, &sRes.quadVAO);
            ::GLES::glGenBuffers(1, &sRes.quadVBO);
            ::GLES::glBindVertexArray(sRes.quadVAO);
            ::GLES::glBindBuffer(GL_ARRAY_BUFFER, sRes.quadVBO);
            ::GLES::glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            ::GLES::glEnableVertexAttribArray(0);
            ::GLES::glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

            ::GLES::glEnableVertexAttribArray(1);
            ::GLES::glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,4 * sizeof(float), (void*)(2 * sizeof(float)));

            ::GLES::glBindVertexArray(0);
        }

        auto& texState = TextureState::GetInstance();
        std::vector<std::pair<GLuint, TextureObject*>> sortedTextures;

        for (auto&& [stateID, texObj] : texState.textures) {
            if (!texObj.generated || texObj.target != GL_TEXTURE_2D) continue;
            if (texObj.params.mipmapData.empty()) continue;
            sortedTextures.emplace_back(stateID, &texObj);
        }
        std::sort(sortedTextures.begin(), sortedTextures.end(), [](auto& a, auto& b) {
            return a.second->createTimestamp > b.second->createTimestamp;
        });

        sortedTextures.resize(std::min<size_t>(sortedTextures.size(), 30));

        for (auto&& [stateID, texObj] : sortedTextures) {
            bool needCreate = sRes.textureCache.find(stateID) == sRes.textureCache.end();
            bool needUpdate = true;

            GLuint glTexID = 0;
            if (needCreate) {
                ::GLES::glGenTextures(1, &glTexID);
                sRes.textureCache[stateID] = glTexID;
                needUpdate = true;
            } else {
                glTexID = sRes.textureCache[stateID];
            }

            const auto& mip0 = texObj->params.mipmapData.begin()->second;
            if (mip0.hasData) {
                ::GLES::glActiveTexture(GL_TEXTURE0);
                ::GLES::glBindTexture(GL_TEXTURE_2D, glTexID);

                if (needUpdate || needCreate) {
                    ::GLES::glTexImage2D(GL_TEXTURE_2D, 0, mip0.internalFormat,
                                 mip0.width, mip0.height, 0,
                                 mip0.format, mip0.type, mip0.pixelData.data());

                    ::GLES::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    ::GLES::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    ::GLES::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    ::GLES::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                    if (texObj->params.mipmapData.size() > 1) {
                        ::GLES::glGenerateMipmap(GL_TEXTURE_2D);
                    }
                }
            }
        }

        GLboolean origDepthTest = glIsEnabled(GL_DEPTH_TEST);
        GLint origProgram;
        GLint origViewport[4];
        ::GLES::glGetIntegerv(GL_VIEWPORT, origViewport);
        ::GLES::glGetIntegerv(GL_CURRENT_PROGRAM, &origProgram);

        ::GLES::glDisable(GL_DEPTH_TEST);
        ::GLES::glUseProgram(sRes.program);
        GLint uTexLoc = ::GLES::glGetUniformLocation(sRes.program, "uTex");
        ::GLES::glUniform1i(uTexLoc, 0);
        
        ::GLES::glBindVertexArray(sRes.quadVAO);

        glm::mat4 proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);
        GLint uMVPLoc = glGetUniformLocation(sRes.program, "uMVP");
        glm::mat4 identity(1.0f);
        ::GLES::glUniformMatrix4fv(uMVPLoc, 1, GL_FALSE, &identity[0][0]);

        for (size_t i = 0; i < sortedTextures.size(); ++i) {
            GLuint glTexID = sRes.textureCache[sortedTextures[i].first];

            ::GLES::glActiveTexture(GL_TEXTURE0);
            ::GLES::glBindTexture(GL_TEXTURE_2D, glTexID);

            GLint uTexLoc = ::GLES::glGetUniformLocation(sRes.program, "uTex");
            ::GLES::glUniform1i(uTexLoc, 0);

            ::GLES::glDrawArrays(GL_TRIANGLE_FAN, static_cast<GLint>(i*4), 4);
        }
    }
}
