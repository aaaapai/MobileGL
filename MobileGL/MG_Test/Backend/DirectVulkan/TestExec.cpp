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
#include <chrono>
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

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    static constexpr GLfloat kQuadPositions[] = {
        // triangle 1
        -0.6f, -0.6f,
         0.6f, -0.6f,
         0.6f,  0.6f,
        // triangle 2
         0.6f,  0.6f,
        -0.6f,  0.6f,
        -0.6f, -0.6f
    };

    static constexpr GLfloat kQuadColors[] = {
        // triangle 1: RGB
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        // triangle 2: RGB
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    GLuint positionVbo = 0;
    glGenBuffers(1, &positionVbo);
    glBindBuffer(GL_ARRAY_BUFFER, positionVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadPositions), kQuadPositions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(2 * sizeof(GLfloat)), nullptr);
    glEnableVertexAttribArray(0);

    GLuint colorVbo = 0;
    glGenBuffers(1, &colorVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadColors), kQuadColors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(3 * sizeof(GLfloat)), nullptr);
    glEnableVertexAttribArray(1);

    static constexpr GLushort kQuadIndices[] = {0, 1, 2, 3, 4, 5};
    GLuint ebo = 0;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kQuadIndices), kQuadIndices, GL_STATIC_DRAW);

static constexpr const char* kVertexShaderSource = R"(#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
out vec3 vColor;
uniform float iTime;
void main() {
    float wobble = 0.05 * sin(iTime);
    gl_Position = vec4(aPos.x, aPos.y + wobble, 0.0, 1.0);
    vColor = aColor;
})";
    static constexpr const char* kFragmentShaderSource = R"(#version 330 core
in vec3 vColor;
layout(location = 0) out vec4 outColor;
layout(std140, binding = 1) uniform ColorBlock {
    vec3 uColor;
};
uniform float iTime;
void main() {
    float pulse = 0.5 + 0.5 * sin(iTime * 1.7);
    outColor = vec4(vColor * uColor * pulse, 1.0);
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

    const GLint iTimeLocation = glGetUniformLocation(program, "iTime");
    if (iTimeLocation < 0) {
        std::cerr << "Failed to get uniform location for iTime" << std::endl;
        return 1;
    }

    const GLuint colorBlockIndex = glGetUniformBlockIndex(program, "ColorBlock");
    if (colorBlockIndex == GL_INVALID_INDEX) {
        std::cerr << "Failed to get uniform block index for ColorBlock" << std::endl;
        return 1;
    }
    glUniformBlockBinding(program, colorBlockIndex, 1);

    GLuint colorUbo = 0;
    glGenBuffers(1, &colorUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, colorUbo);
    alignas(16) GLfloat uboColorData[4] = {1.0f, 0.6f, 0.2f, 0.0f};
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uboColorData), uboColorData, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, colorUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    const auto startTime = std::chrono::steady_clock::now();

    int i = 0;
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (i % 1000 > 500)
            glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        else
            glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        if (i % 500 > 250) {
            glUseProgram(program);
            const auto now = std::chrono::steady_clock::now();
            const float t = std::chrono::duration<float>(now - startTime).count();
            glUniform1f(iTimeLocation, t);

            uboColorData[0] = 0.5f + 0.5f * std::sin(t * 0.7f);
            uboColorData[1] = 0.5f + 0.5f * std::sin(t * 1.1f + 1.2f);
            uboColorData[2] = 0.5f + 0.5f * std::sin(t * 1.5f + 2.4f);
            glBindBuffer(GL_UNIFORM_BUFFER, colorUbo);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(uboColorData), uboColorData);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
        }
        eglSwapBuffers(display, surface);
        ++i;
    }

    glDeleteProgram(program);
    glDeleteBuffers(1, &colorUbo);
    glDeleteBuffers(1, &positionVbo);
    glDeleteBuffers(1, &colorVbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);

    glfwTerminate();
}
