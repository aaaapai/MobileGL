// MobileGL - MobileGL/MG_Test/Backend/DirectVulkan/TestExec.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include <iostream>
#include <vector>
#include <cassert>
#include <string>
#include <algorithm>
#include <cmath>

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#include <GL/glext.h>
#undef GL_GLEXT_PROTOTYPES

#ifndef EGLAPI
#define EGLAPI extern
#endif
#include <EGL/egl.h>
#ifdef _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
    #define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3native.h>
#ifdef __APPLE__
#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#endif

#define NO_GL_H
#include <Includes.h>
#undef NO_GL_H

namespace MobileGL {
    void MG_Initialize();
}

static bool CheckShaderCompile(GLuint shader, const char* label) {
    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_TRUE) {
        return true;
    }

    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    std::string log;
    if (logLength > 0) {
        log.resize(static_cast<size_t>(logLength));
        GLsizei written = 0;
        glGetShaderInfoLog(shader, logLength, &written, log.data());
        if (written >= 0 && static_cast<size_t>(written) < log.size()) {
            log.resize(static_cast<size_t>(written));
        }
    }
    std::cerr << label << " compile failed: " << log << std::endl;
    return false;
}

static bool CheckProgramLink(GLuint program) {
    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_TRUE) {
        return true;
    }

    GLint logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    std::string log;
    if (logLength > 0) {
        log.resize(static_cast<size_t>(logLength));
        GLsizei written = 0;
        glGetProgramInfoLog(program, logLength, &written, log.data());
        if (written >= 0 && static_cast<size_t>(written) < log.size()) {
            log.resize(static_cast<size_t>(written));
        }
    }
    std::cerr << "Program link failed: " << log << std::endl;
    return false;
}

int main() {
    glfwInit();

    static EGLint const attribute_list[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_NONE
    };

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);
    EGLConfig config;
    EGLint num_config;
    eglChooseConfig(display, attribute_list, &config, 1, &num_config);
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, nullptr);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "MobileGL ContextCreation", nullptr, nullptr);
    EGLNativeWindowType nativewindow = nullptr;
#ifdef _WIN32
    nativewindow = glfwGetWin32Window(window);
#elif defined(__APPLE__)
    void* cocoaWindow = glfwGetCocoaWindow(window);
    MOBILEGL_ASSERT(cocoaWindow, "glfwGetCocoaWindow returned null");
    auto msgSendObj = reinterpret_cast<id (*)(id, SEL)>(objc_msgSend);
    auto msgSendVoidObj = reinterpret_cast<void (*)(id, SEL, id)>(objc_msgSend);
    auto msgSendVoidBool = reinterpret_cast<void (*)(id, SEL, bool)>(objc_msgSend);
    id contentView = msgSendObj(static_cast<id>(cocoaWindow), sel_registerName("contentView"));
    MOBILEGL_ASSERT(contentView, "Failed to query NSWindow.contentView");
    msgSendVoidBool(contentView, sel_registerName("setWantsLayer:"), true);

    id metalLayerClass = reinterpret_cast<id>(objc_getClass("CAMetalLayer"));
    MOBILEGL_ASSERT(metalLayerClass, "Failed to lookup CAMetalLayer class");
    id metalLayer = msgSendObj(metalLayerClass, sel_registerName("layer"));
    MOBILEGL_ASSERT(metalLayer, "Failed to create CAMetalLayer");

    msgSendVoidObj(contentView, sel_registerName("setLayer:"), metalLayer);
    nativewindow = reinterpret_cast<EGLNativeWindowType>(metalLayer);
#endif
    EGLSurface surface = eglCreateWindowSurface(display, config, nativewindow, nullptr);
    eglMakeCurrent(display, surface, surface, context);

    MobileGL::MG_Initialize();

    GLuint offscreenTex = 0;
    GLuint offscreenFbo = 0;
    glGenTextures(1, &offscreenTex);
    glBindTexture(GL_TEXTURE_2D, offscreenTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &offscreenFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, offscreenFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offscreenTex, 0);
    const GLenum offscreenFboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    std::cout << "Offscreen FBO status = 0x" << std::hex << offscreenFboStatus << std::dec << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    static constexpr GLint kWindowWidth = 800;
    static constexpr GLint kWindowHeight = 600;
    glViewport(0, 0, kWindowWidth, kWindowHeight);

    static constexpr int kMaxSegments = 192;
    static constexpr float kPi = 3.14159265358979323846f;

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint shapeVbo = 0;
    glGenBuffers(1, &shapeVbo);
    glBindBuffer(GL_ARRAY_BUFFER, shapeVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>((kMaxSegments + 1) * 5 * sizeof(GLfloat)),
        nullptr,
        GL_DYNAMIC_DRAW
    );
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(5 * sizeof(GLfloat)), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(5 * sizeof(GLfloat)),
                          reinterpret_cast<void*>(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    GLuint shapeIbo = 0;
    glGenBuffers(1, &shapeIbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shapeIbo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(kMaxSegments * 3 * sizeof(GLushort)),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    static constexpr const char* kVertexShaderSource = R"(#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
out vec3 vColor;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vColor = aColor;
})";
    static constexpr const char* kFragmentShaderSource = R"(#version 330 core
in vec3 vColor;
layout(location = 0) out vec4 outColor;
void main() {
    outColor = vec4(vColor, 1.0);
})";

    const GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &kVertexShaderSource, nullptr);
    glCompileShader(vs);
    if (!CheckShaderCompile(vs, "Vertex shader")) {
        return 1;
    }

    const GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &kFragmentShaderSource, nullptr);
    glCompileShader(fs);
    if (!CheckShaderCompile(fs, "Fragment shader")) {
        return 1;
    }

    const GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    if (!CheckProgramLink(program)) {
        return 1;
    }
    glUseProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    std::vector<GLfloat> dynamicVertices(static_cast<size_t>((kMaxSegments + 1) * 5));
    std::vector<GLushort> dynamicIndices(static_cast<size_t>(kMaxSegments * 3));
    int i = 0;
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        GLint framebufferWidth = kWindowWidth;
        GLint framebufferHeight = kWindowHeight;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        glViewport(0, 0, framebufferWidth, framebufferHeight);

        const bool useOffscreenPath = ((i / 100) % 2) == 0;
        if (useOffscreenPath) {
            glBindFramebuffer(GL_FRAMEBUFFER, offscreenFbo);
            glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glBindFramebuffer(GL_READ_FRAMEBUFFER, offscreenFbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(0, 0, 256, 256, 0, 0, framebufferWidth, framebufferHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        } else {
            const int segmentCount = std::min(kMaxSegments, 3 + (i / 100));
            const float radius = 0.75f;
            const float rotation = static_cast<float>(i) * 0.01f;

            const size_t vertexFloatCount = static_cast<size_t>((segmentCount + 1) * 5);
            const size_t indexCount = static_cast<size_t>(segmentCount * 3);

            dynamicVertices[0] = 0.0f;
            dynamicVertices[1] = 0.0f;
            dynamicVertices[2] = 0.95f;
            dynamicVertices[3] = 0.95f;
            dynamicVertices[4] = 0.95f;

            for (int v = 0; v < segmentCount; ++v) {
                const float theta = rotation + (2.0f * kPi * static_cast<float>(v) / static_cast<float>(segmentCount));
                const float x = radius * std::cos(theta);
                const float y = radius * std::sin(theta);
                const size_t base = static_cast<size_t>((v + 1) * 5);
                dynamicVertices[base + 0] = x;
                dynamicVertices[base + 1] = y;
                dynamicVertices[base + 2] = 0.5f + 0.5f * std::cos(theta);
                dynamicVertices[base + 3] = 0.5f + 0.5f * std::sin(theta);
                dynamicVertices[base + 4] = 0.9f;

                const size_t ib = static_cast<size_t>(v * 3);
                dynamicIndices[ib + 0] = 0;
                dynamicIndices[ib + 1] = static_cast<GLushort>(v + 1);
                dynamicIndices[ib + 2] = static_cast<GLushort>((v + 1) % segmentCount + 1);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClearColor(0.05f, 0.08f, 0.12f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(program);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, shapeVbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(vertexFloatCount * sizeof(GLfloat)), dynamicVertices.data());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shapeIbo);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(indexCount * sizeof(GLushort)), dynamicIndices.data());
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_SHORT, nullptr);
        }

        if (i % 100 == 0) {
            std::cout << "frame=" << i
                      << " path=" << (useOffscreenPath ? "offscreen-clear+blit" : "dynamic-vbo-vao-ibo-archimedes")
                      << std::endl;
        }
        eglSwapBuffers(display, surface);
        ++i;
    }

    glDeleteProgram(program);
    glDeleteBuffers(1, &shapeVbo);
    glDeleteBuffers(1, &shapeIbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteFramebuffers(1, &offscreenFbo);
    glDeleteTextures(1, &offscreenTex);

    glfwDestroyWindow(window);

    glfwTerminate();
}
