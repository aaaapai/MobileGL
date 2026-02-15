// MobileGL - MobileGL/MG_Test/Backend/DirectVulkan/TestExec.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define EGLAPI
#include <EGL/egl.h>
#ifdef _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
    #define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include "MG_Backend/DirectVulkan/DirectVulkan.h"
#include "MG_Backend/DirectVulkan/Renderer/VulkanRenderer.h"

#include <GLFW/glfw3native.h>
#ifdef __APPLE__
#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#endif

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

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        MobileGL::MG_Backend::DirectVulkan::pVulkanRenderer->Render();
        eglSwapBuffers(display, surface);
    }

    glfwDestroyWindow(window);

    glfwTerminate();
}
