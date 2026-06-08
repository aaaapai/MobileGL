// MobileGL - MobileGL/MG_Test/SanityTest.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include <gtest/gtest.h>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include <MG_Backend/DirectGLES/BackendObject_DirectGLES.h>
#include <MG_Backend/DirectVulkan/BackendObject_DirectVulkan.h>
#include <MG_Backend/BackendObjects.h>
#include <MG_Impl/GLImpl/Getter/GL_Getter.h>
#include <MG_Impl/GLImpl/RenderState/GL_RenderState.h>
#include <MG_State/GLState/Core.h>
#include <MG_Backend/DirectVulkan/Renderer/VkRenderPassManager.h>
#include <MG_Backend/DirectVulkan/Renderer/VkTextureManager.h>
#include <MG_Util/Debug/Log.h>

namespace {
    void SetEnvVar(const char* name, const char* value) {
#if defined(_WIN32)
        _putenv_s(name, value);
#else
        setenv(name, value, 1);
#endif
    }

    void UnsetEnvVar(const char* name) {
#if defined(_WIN32)
        _putenv_s(name, "");
#else
        unsetenv(name);
#endif
    }
} // namespace

TEST(Sanity, BasicAssertions) {
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
}

TEST(DirectGLESSanity, AdvertisesDepthTextureForGlmarkShadowScenes) {
    MobileGL::MG_Backend::DirectGLES::BackendObject_DirectGLES backend;
    const auto& extensions = backend.GetRendererInfo().RendererGLInfo.Extensions;

    EXPECT_NE(std::find(extensions.begin(), extensions.end(), MobileGL::E_GL_ARB_depth_texture), extensions.end());
}

TEST(DirectVulkanSanity, AdvertisesTextureStorageForDirectStateAccess) {
    MobileGL::MG_Backend::DirectVulkan::BackendObject_DirectVulkan backend;
    const auto& extensions = backend.GetRendererInfo().RendererGLInfo.Extensions;

    EXPECT_NE(std::find(extensions.begin(), extensions.end(), MobileGL::E_GL_ARB_direct_state_access),
              extensions.end());
    EXPECT_NE(std::find(extensions.begin(), extensions.end(), MobileGL::E_GL_ARB_texture_storage), extensions.end());
}

TEST(DirectVulkanSanity, AdvertisesVoxyRequiredRenderingExtensionsWithoutRaisingGLVersion) {
    MobileGL::MG_Backend::DirectVulkan::BackendObject_DirectVulkan backend;
    const auto& rendererInfo = backend.GetRendererInfo().RendererGLInfo;
    const auto& extensions = rendererInfo.Extensions;

    EXPECT_EQ(rendererInfo.TargetGLVersion.Major, 3);
    EXPECT_EQ(rendererInfo.TargetGLVersion.Minor, 3);
    EXPECT_EQ(rendererInfo.TargetGLVersion.Patch, 0);

    EXPECT_NE(std::find(extensions.begin(), extensions.end(), MobileGL::E_GL_ARB_compute_shader),
              extensions.end());
    EXPECT_NE(std::find(extensions.begin(), extensions.end(), MobileGL::E_GL_ARB_shader_storage_buffer_object),
              extensions.end());
    EXPECT_NE(std::find(extensions.begin(), extensions.end(), MobileGL::E_GL_ARB_multi_draw_indirect),
              extensions.end());
    EXPECT_NE(std::find(extensions.begin(), extensions.end(), MobileGL::E_GL_ARB_indirect_parameters),
              extensions.end());
    EXPECT_NE(std::find(extensions.begin(), extensions.end(), MobileGL::E_GL_ARB_shader_draw_parameters),
              extensions.end());
    EXPECT_NE(std::find(extensions.begin(), extensions.end(), MobileGL::E_GL_ARB_gpu_shader_int64),
              extensions.end());
}

TEST(DirectVulkanSanity, UndefinedDepthStencilLayoutUsesDontCareForUnclearedAspects) {
    using namespace MobileGL;
    using namespace MobileGL::MG_Backend::DirectVulkan;

    auto noClear = ResolveDepthStencilAttachmentLoadInfo(VK_IMAGE_LAYOUT_UNDEFINED, false, false);
    EXPECT_EQ(noClear.depthLoadOp, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    EXPECT_EQ(noClear.stencilLoadOp, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    EXPECT_EQ(noClear.initialLayout, VK_IMAGE_LAYOUT_UNDEFINED);

    auto depthOnlyClear = ResolveDepthStencilAttachmentLoadInfo(VK_IMAGE_LAYOUT_UNDEFINED, true, false);
    EXPECT_EQ(depthOnlyClear.depthLoadOp, VK_ATTACHMENT_LOAD_OP_CLEAR);
    EXPECT_EQ(depthOnlyClear.stencilLoadOp, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    EXPECT_EQ(depthOnlyClear.initialLayout, VK_IMAGE_LAYOUT_UNDEFINED);

    auto knownLayout = ResolveDepthStencilAttachmentLoadInfo(
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, false, false);
    EXPECT_EQ(knownLayout.depthLoadOp, VK_ATTACHMENT_LOAD_OP_LOAD);
    EXPECT_EQ(knownLayout.stencilLoadOp, VK_ATTACHMENT_LOAD_OP_LOAD);
    EXPECT_EQ(knownLayout.initialLayout, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

TEST(DirectVulkanSanity, SampledDepthStencilViewUsesSingleDepthAspect) {
    using namespace MobileGL::MG_Backend::DirectVulkan;

    EXPECT_EQ(VkTextureManager::ResolveSampledImageViewAspectMask(VK_IMAGE_ASPECT_COLOR_BIT),
              VK_IMAGE_ASPECT_COLOR_BIT);
    EXPECT_EQ(VkTextureManager::ResolveSampledImageViewAspectMask(VK_IMAGE_ASPECT_DEPTH_BIT),
              VK_IMAGE_ASPECT_DEPTH_BIT);
    EXPECT_EQ(VkTextureManager::ResolveSampledImageViewAspectMask(VK_IMAGE_ASPECT_STENCIL_BIT),
              VK_IMAGE_ASPECT_STENCIL_BIT);
    EXPECT_EQ(VkTextureManager::ResolveSampledImageViewAspectMask(
                  VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT),
              VK_IMAGE_ASPECT_DEPTH_BIT);
}

TEST(RenderStateSanity, ProvokingVertexUpdatesStateAndValidatesEnum) {
    using namespace MobileGL;

    MG_State::pGLContext = MakeUnique<MG_State::GLState::GLContext>();
    MG_Backend::pActiveBackendObject = MakeUnique<MG_Backend::DirectGLES::BackendObject_DirectGLES>();

    GLint mode = 0;
    MG_Impl::GLImpl::GetIntegerv(GL_PROVOKING_VERTEX, &mode);
    EXPECT_EQ(mode, GL_LAST_VERTEX_CONVENTION);

    const Uint initialVersion = MG_State::pGLContext->GetRenderStateParametersVersion();
    MG_Impl::GLImpl::ProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
    MG_Impl::GLImpl::GetIntegerv(GL_PROVOKING_VERTEX, &mode);
    EXPECT_EQ(mode, GL_FIRST_VERTEX_CONVENTION);
    EXPECT_GT(MG_State::pGLContext->GetRenderStateParametersVersion(), initialVersion);

    const Uint updatedVersion = MG_State::pGLContext->GetRenderStateParametersVersion();
    MG_Impl::GLImpl::ProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
    EXPECT_EQ(MG_State::pGLContext->GetRenderStateParametersVersion(), updatedVersion);

    MG_Impl::GLImpl::ProvokingVertex(GL_TRIANGLES);
    EXPECT_EQ(MG_Impl::GLImpl::GetError(), GL_INVALID_ENUM);
    MG_Impl::GLImpl::GetIntegerv(GL_PROVOKING_VERTEX, &mode);
    EXPECT_EQ(mode, GL_FIRST_VERTEX_CONVENTION);

    MG_Backend::pActiveBackendObject.reset();
    MG_State::pGLContext.reset();
}

TEST(LogSanity, UsesEnvOverrideForFilePath) {
    namespace fs = std::filesystem;

    MobileGL::MG_Util::Debug::Close();

    const fs::path logPath = fs::temp_directory_path() / "mobilegl-log-env-override-test.log";
    const std::string message = "mobilegl-log-env-override-regression";
    fs::remove(logPath);

    SetEnvVar("MOBILEGL_LOG_FILE_PATH", logPath.string().c_str());
    MobileGL::MG_Util::Debug::Log("INFO", ANDROID_LOG_INFO, "%s", message.c_str());
    MobileGL::MG_Util::Debug::Close();
    UnsetEnvVar("MOBILEGL_LOG_FILE_PATH");

    {
        std::ifstream logFile(logPath);
        ASSERT_TRUE(logFile.good());

        const std::string contents((std::istreambuf_iterator<char>(logFile)), std::istreambuf_iterator<char>());
        EXPECT_NE(contents.find(message), std::string::npos);
    }

    fs::remove(logPath);
}
