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

    std::ifstream logFile(logPath);
    ASSERT_TRUE(logFile.good());

    const std::string contents((std::istreambuf_iterator<char>(logFile)), std::istreambuf_iterator<char>());
    EXPECT_NE(contents.find(message), std::string::npos);

    fs::remove(logPath);
}
