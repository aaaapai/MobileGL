// MobileGL - MobileGL/MG_Test/Texture/VkClearManagerTest.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include <gtest/gtest.h>

#include "Includes.h"
#include "Init.h"
#include <MG_Backend/DirectVulkan/Renderer/VkClearManager.h>
#include <MG_Impl/GLImpl/Texture/GL_Texture.h>
#include <MG_State/GLState/Core.h>

using namespace MobileGL;
using namespace MobileGL::MG_Backend::DirectVulkan;

class VkClearManagerTest : public ::testing::Test {
protected:
    void SetUp() override { MobileGL::Initialize(); }
};

TEST_F(VkClearManagerTest, CollectGarbageRemovesExpiredTexturesAndTheirPendingClears) {
    VkClearManager clearManager;
    ASSERT_TRUE(clearManager.Initialize());

    constexpr SizeT kTextureCount = 64;
    Vector<GLuint> textureNames(kTextureCount, 0);
    Vector<PendingClearKey> pendingKeys;
    pendingKeys.reserve(kTextureCount);

    const ClearAttachmentPayload clearPayload{
        .mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
        .color = FloatVec4(0.25f, 0.5f, 0.75f, 1.0f),
        .depth = 0.5f,
    };

    for (SizeT i = 0; i < kTextureCount; ++i) {
        MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_2D, 1, &textureNames[i]);
        ASSERT_NE(textureNames[i], 0u);

        {
            const auto textureObject = MG_State::pGLContext->GetTextureObject(textureNames[i]);
            ASSERT_NE(textureObject, nullptr);

            const PendingClearKey key = VkClearManager::MakePendingClearKey(textureObject.get());
            clearManager.QueueClear(clearPayload, textureObject);

            EXPECT_TRUE(clearManager.HasPendingClear(key));
            pendingKeys.emplace_back(key);
        }
    }

    MG_Impl::GLImpl::DeleteTextures(static_cast<GLsizei>(textureNames.size()), textureNames.data());

    for (Int i = 0; i < 255; ++i) {
        EXPECT_EQ(clearManager.CollectGarbage(), 0u);
    }
    EXPECT_EQ(clearManager.CollectGarbage(), kTextureCount);

    ClearAttachmentPayload outPayload{};
    for (const auto& key : pendingKeys) {
        EXPECT_FALSE(clearManager.HasPendingClear(key));
        EXPECT_FALSE(clearManager.GetPendingClear(key, outPayload));
    }

    GLuint freshTexture = 0;
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_2D, 1, &freshTexture);
    ASSERT_NE(freshTexture, 0u);

    const auto freshTextureObject = MG_State::pGLContext->GetTextureObject(freshTexture);
    ASSERT_NE(freshTextureObject, nullptr);
    EXPECT_FALSE(clearManager.HasPendingClear(freshTextureObject.get()));

    MG_Impl::GLImpl::DeleteTextures(1, &freshTexture);
    clearManager.Shutdown();
}

TEST_F(VkClearManagerTest, StalePendingClearsAreRejectedBeforePeriodicGarbageCollection) {
    VkClearManager clearManager;
    ASSERT_TRUE(clearManager.Initialize());

    GLuint texture = 0;
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_2D, 1, &texture);
    ASSERT_NE(texture, 0u);

    PendingClearKey key{};
    {
        const auto textureObject = MG_State::pGLContext->GetTextureObject(texture);
        ASSERT_NE(textureObject, nullptr);

        key = VkClearManager::MakePendingClearKey(textureObject.get());
        clearManager.QueueClear(ClearAttachmentPayload{
            .mask = GL_COLOR_BUFFER_BIT,
            .color = FloatVec4(1.0f, 0.25f, 0.5f, 0.75f),
        }, textureObject);
        EXPECT_TRUE(clearManager.HasPendingClear(key));
    }

    MG_Impl::GLImpl::DeleteTextures(1, &texture);

    ClearAttachmentPayload outPayload{};
    EXPECT_FALSE(clearManager.HasPendingClear(key));
    EXPECT_FALSE(clearManager.GetPendingClear(key, outPayload));
    EXPECT_FALSE(clearManager.HasPendingClear(key));

    clearManager.Shutdown();
}
