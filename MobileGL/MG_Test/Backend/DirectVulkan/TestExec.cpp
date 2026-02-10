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
#endif
#include "MG_Backend/DirectVulkan/DirectVulkan.h"
#include "MG_Backend/DirectVulkan/Renderer/VulkanRenderer.h"

#include <GLFW/glfw3native.h>

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
#ifdef _WIN32
    EGLNativeWindowType nativewindow = glfwGetWin32Window(window);
#endif
    EGLSurface surface = eglCreateWindowSurface(display, config, nativewindow, nullptr);
    eglMakeCurrent(display, surface, surface, context);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        // MobileGL::MG_Backend::DirectVulkan::pVulkanRenderer->RenderFrame();
        eglSwapBuffers(display, surface);
    }

    glfwDestroyWindow(window);

    glfwTerminate();
}
