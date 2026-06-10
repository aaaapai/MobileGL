// MobileGL - MobileGL/MG_Test/Texture/TextureTest.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include <gtest/gtest.h>

#include "Includes.h"
#include "Init.h"
#include <MG_Impl/GLImpl/Getter/GL_Getter.h>
#include <MG_Impl/GLImpl/RenderState/GL_RenderState.h>
#include <MG_Impl/GLImpl/Texture/GL_Texture.h>
#include <MG_State/GLState/Core.h>
#include <MG_State/GLState/TextureState/TextureObject.h>
#include <MG_Util/Texture/TextureFormatProcessor.h>

using namespace MobileGL;

class TextureTest : public ::testing::Test {
protected:
    void SetUp() override { MobileGL::Initialize(); }
};

TEST_F(TextureTest, CreateTexturesCreatesObjectsWithoutBinding) {
    GLuint texture = 0;
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_2D, 1, &texture);

    EXPECT_NE(texture, 0u);
    EXPECT_TRUE(MG_State::pGLContext->ValidateTextureObject(texture));

    auto& unit = MG_State::pGLContext->GetTextureUnitObject(MG_State::pGLContext->GetActiveTextureUnit());
    EXPECT_EQ(unit.GetBindingSlot(TextureTarget::Texture2D).GetBoundObject(), nullptr);
    EXPECT_EQ(MG_Impl::GLImpl::GetError(), GL_NO_ERROR);
}

TEST_F(TextureTest, TextureStorageAndSubImageModifyNamedObjectOnly) {
    GLuint namedTexture = 0;
    GLuint boundTexture = 0;
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_2D, 1, &namedTexture);
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_2D, 1, &boundTexture);
    MG_Impl::GLImpl::BindTextureUnit(0, boundTexture);

    const auto boundObjectBefore =
        MG_State::pGLContext->GetTextureUnitObject(0).GetBindingSlot(TextureTarget::Texture2D).GetBoundObject();

    MG_Impl::GLImpl::TextureStorage2D(namedTexture, 2, GL_RGBA8, 2, 2);
    const Uint8 pixels[] = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16,
    };
    MG_Impl::GLImpl::TextureSubImage2D(namedTexture, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    const auto namedObject = MG_State::pGLContext->GetTextureObject(namedTexture);
    auto* mipmapObject = static_cast<MG_State::GLState::TextureObjectMipmap*>(namedObject.get());
    EXPECT_EQ(mipmapObject->GetMipmapTexelSize(TextureUploadTarget::Texture2D, 0), IntVec3(2, 2, 1));
    EXPECT_EQ(mipmapObject->GetMipmapTexelSize(TextureUploadTarget::Texture2D, 1), IntVec3(1, 1, 1));
    EXPECT_TRUE(mipmapObject->IsStorageDirty(TextureUploadTarget::Texture2D, 0));
    EXPECT_FALSE(mipmapObject->IsStorageDirty(TextureUploadTarget::Texture2D, 1));

    EXPECT_EQ(MG_State::pGLContext->GetTextureUnitObject(0).GetBindingSlot(TextureTarget::Texture2D).GetBoundObject(),
              boundObjectBefore);
    EXPECT_EQ(MG_Impl::GLImpl::GetError(), GL_NO_ERROR);
}

TEST_F(TextureTest, BoundTexSubImage2DUsesCompactRowsAfterUnpackProcessing) {
    GLuint texture = 0;
    MG_Impl::GLImpl::GenTextures(1, &texture);
    MG_Impl::GLImpl::BindTexture(GL_TEXTURE_2D, texture);

    const Uint8 initialPixels[2 * 16] = {};
    MG_Impl::GLImpl::TexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 5, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, initialPixels);

    const Uint8 subImageWithGuard[] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9,
        101, 102, 103,
        10, 11, 12, 13, 14, 15, 16, 17, 18,
        201, 202, 203, 204, 205, 206,
    };
    MG_Impl::GLImpl::PixelStorei(GL_UNPACK_ALIGNMENT, 4);
    MG_Impl::GLImpl::TexSubImage2D(GL_TEXTURE_2D, 0, 1, 0, 3, 2, GL_RGB, GL_UNSIGNED_BYTE, subImageWithGuard);

    const auto textureObject = MG_State::pGLContext->GetTextureObject(texture);
    auto* mipmapObject = static_cast<MG_State::GLState::TextureObjectMipmap*>(textureObject.get());
    const auto* stored = static_cast<const Uint8*>(
        mipmapObject->MapMipmapData(TextureUploadTarget::Texture2D, 0));

    const Uint8 expected[] = {
        0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0,
        0, 0, 0, 10, 11, 12, 13, 14, 15, 16, 17, 18, 0, 0, 0,
    };
    for (SizeT i = 0; i < sizeof(expected); ++i) {
        EXPECT_EQ(stored[i], expected[i]) << "byte " << i;
    }
    EXPECT_EQ(MG_Impl::GLImpl::GetError(), GL_NO_ERROR);
}

TEST_F(TextureTest, BoundTexStorage2DAllocatesRedTextureForSubImageUpdates) {
    GLuint texture = 0;
    MG_Impl::GLImpl::GenTextures(1, &texture);
    MG_Impl::GLImpl::BindTexture(GL_TEXTURE_2D, texture);

    MG_Impl::GLImpl::TexStorage2D(GL_TEXTURE_2D, 1, GL_R8, 32, 32);

    const auto textureObject = MG_State::pGLContext->GetTextureObject(texture);
    auto* mipmapObject = static_cast<MG_State::GLState::TextureObjectMipmap*>(textureObject.get());
    ASSERT_NE(mipmapObject, nullptr);
    EXPECT_EQ(mipmapObject->GetMipmapTexelSize(TextureUploadTarget::Texture2D, 0), IntVec3(32, 32, 1));
    EXPECT_TRUE(textureObject->IsComplete());

    const Uint8 pixels[4 * 4] = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16,
    };
    MG_Impl::GLImpl::TexSubImage2D(GL_TEXTURE_2D, 0, 20, 28, 4, 4, GL_RED, GL_UNSIGNED_BYTE, pixels);

    EXPECT_EQ(MG_Impl::GLImpl::GetError(), GL_NO_ERROR);
}

TEST_F(TextureTest, TextureStorage2DMultisampleTracksNamedObjectState) {
    GLuint texture = 0;
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &texture);

    MG_Impl::GLImpl::TextureStorage2DMultisample(texture, 4, GL_RGBA8, 8, 6, GL_TRUE);

    const auto textureObject = MG_State::pGLContext->GetTextureObject(texture);
    ASSERT_NE(textureObject, nullptr);
    EXPECT_EQ(textureObject->GetTarget(), TextureTarget::Texture2DMultisample);
    EXPECT_EQ(textureObject->GetSamples(), 4);
    EXPECT_TRUE(textureObject->HasFixedSampleLocations());

    auto* textureMipmapObject = static_cast<MG_State::GLState::TextureObjectMipmap*>(textureObject.get());
    ASSERT_NE(textureMipmapObject, nullptr);
    EXPECT_EQ(textureMipmapObject->GetMipmapLevelCount(), 1u);
    EXPECT_EQ(textureMipmapObject->GetMipmapTexelSize(TextureUploadTarget::Texture2DMultisample, 0), IntVec3(8, 6, 1));
    EXPECT_FALSE(textureMipmapObject->IsStorageDirty(TextureUploadTarget::Texture2DMultisample, 0));

    GLint samples = 0;
    GLint fixed = 0;
    MG_Impl::GLImpl::GetTextureLevelParameteriv(texture, 0, GL_TEXTURE_SAMPLES, &samples);
    MG_Impl::GLImpl::GetTextureLevelParameteriv(texture, 0, GL_TEXTURE_FIXED_SAMPLE_LOCATIONS, &fixed);
    EXPECT_EQ(samples, 4);
    EXPECT_EQ(fixed, GL_TRUE);
    EXPECT_EQ(MG_Impl::GLImpl::GetError(), GL_NO_ERROR);
}

TEST_F(TextureTest, GetTextureImageReadsNamedObjectWithoutBinding) {
    GLuint texture = 0;
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_2D, 1, &texture);
    MG_Impl::GLImpl::TextureStorage2D(texture, 1, GL_RGBA8, 2, 1);

    const Uint8 pixels[] = {
        21, 22, 23, 24,
        31, 32, 33, 34,
    };
    MG_Impl::GLImpl::TextureSubImage2D(texture, 0, 0, 0, 2, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    Uint8 output[sizeof(pixels)] = {};
    MG_Impl::GLImpl::GetTextureImage(texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, sizeof(output), output);

    EXPECT_EQ(std::memcmp(output, pixels, sizeof(pixels)), 0);
    EXPECT_EQ(MG_Impl::GLImpl::GetError(), GL_NO_ERROR);
}

TEST_F(TextureTest, GetTextureSubImageReadsFullNamedLevelWithoutBinding) {
    GLuint texture = 0;
    GLuint boundTexture = 0;
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_2D, 1, &texture);
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_2D, 1, &boundTexture);
    MG_Impl::GLImpl::BindTextureUnit(0, boundTexture);

    const auto boundObjectBefore =
        MG_State::pGLContext->GetTextureUnitObject(0).GetBindingSlot(TextureTarget::Texture2D).GetBoundObject();

    MG_Impl::GLImpl::TextureStorage2D(texture, 1, GL_RGBA8, 2, 1);
    const Uint8 pixels[] = {
        41, 42, 43, 44,
        51, 52, 53, 54,
    };
    MG_Impl::GLImpl::TextureSubImage2D(texture, 0, 0, 0, 2, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    Uint8 output[sizeof(pixels)] = {};
    MG_Impl::GLImpl::GetTextureSubImage(texture, 0, 0, 0, 0, 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                                        sizeof(output), output);

    EXPECT_EQ(std::memcmp(output, pixels, sizeof(pixels)), 0);
    EXPECT_EQ(MG_State::pGLContext->GetTextureUnitObject(0).GetBindingSlot(TextureTarget::Texture2D).GetBoundObject(),
              boundObjectBefore);
    EXPECT_EQ(MG_Impl::GLImpl::GetError(), GL_NO_ERROR);
}

TEST_F(TextureTest, GetTextureSubImageRejectsPartialReadbackForNow) {
    GLuint texture = 0;
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_2D, 1, &texture);
    MG_Impl::GLImpl::TextureStorage2D(texture, 1, GL_RGBA8, 2, 2);

    Uint8 output[4] = {};
    MG_Impl::GLImpl::GetTextureSubImage(texture, 0, 0, 0, 0, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                                        sizeof(output), output);

    EXPECT_EQ(MG_Impl::GLImpl::GetError(), GL_INVALID_OPERATION);
}

TEST_F(TextureTest, TextureParameteriAndBindTextureUnitAreDirectStateAccess) {
    GLuint texture = 0;
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_2D, 1, &texture);

    MG_Impl::GLImpl::TextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GLint minFilter = 0;
    MG_Impl::GLImpl::GetTextureParameteriv(texture, GL_TEXTURE_MIN_FILTER, &minFilter);
    EXPECT_EQ(minFilter, GL_NEAREST);

    MG_Impl::GLImpl::BindTextureUnit(3, texture);
    EXPECT_EQ(MG_State::pGLContext->GetActiveTextureUnit(), 0);
    EXPECT_EQ(MG_State::pGLContext->GetTextureUnitObject(3)
                  .GetBindingSlot(TextureTarget::Texture2D)
                  .GetBoundObject()
                  ->GetExternalIndex(),
              texture);
    EXPECT_EQ(MG_Impl::GLImpl::GetError(), GL_NO_ERROR);
}

TEST_F(TextureTest, TextureParameterfModifiesNamedObjectWithoutBinding) {
    GLuint namedTexture = 0;
    GLuint boundTexture = 0;
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_2D, 1, &namedTexture);
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_2D, 1, &boundTexture);
    MG_Impl::GLImpl::BindTextureUnit(0, boundTexture);

    const auto boundObjectBefore =
        MG_State::pGLContext->GetTextureUnitObject(0).GetBindingSlot(TextureTarget::Texture2D).GetBoundObject();

    MG_Impl::GLImpl::TextureParameterf(namedTexture, GL_TEXTURE_MIN_FILTER, static_cast<GLfloat>(GL_LINEAR));
    MG_Impl::GLImpl::TextureParameterf(namedTexture, GL_TEXTURE_MAG_FILTER, static_cast<GLfloat>(GL_NEAREST));
    MG_Impl::GLImpl::TextureParameterf(namedTexture, GL_DEPTH_STENCIL_TEXTURE_MODE,
                                       static_cast<GLfloat>(GL_DEPTH_COMPONENT));

    GLint namedMinFilter = 0;
    GLint namedMagFilter = 0;
    GLint boundMinFilter = 0;
    GLint boundMagFilter = 0;
    MG_Impl::GLImpl::GetTextureParameteriv(namedTexture, GL_TEXTURE_MIN_FILTER, &namedMinFilter);
    MG_Impl::GLImpl::GetTextureParameteriv(namedTexture, GL_TEXTURE_MAG_FILTER, &namedMagFilter);
    MG_Impl::GLImpl::GetTextureParameteriv(boundTexture, GL_TEXTURE_MIN_FILTER, &boundMinFilter);
    MG_Impl::GLImpl::GetTextureParameteriv(boundTexture, GL_TEXTURE_MAG_FILTER, &boundMagFilter);

    EXPECT_EQ(namedMinFilter, GL_LINEAR);
    EXPECT_EQ(namedMagFilter, GL_NEAREST);
    EXPECT_EQ(boundMinFilter, GL_NEAREST_MIPMAP_LINEAR);
    EXPECT_EQ(boundMagFilter, GL_LINEAR);
    EXPECT_EQ(MG_State::pGLContext->GetTextureUnitObject(0).GetBindingSlot(TextureTarget::Texture2D).GetBoundObject(),
              boundObjectBefore);
    EXPECT_EQ(MG_Impl::GLImpl::GetError(), GL_NO_ERROR);
}

TEST_F(TextureTest, TextureStorage1DAndSubImageModifyNamedObjectOnly) {
    GLuint namedTexture = 0;
    GLuint boundTexture = 0;
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_1D, 1, &namedTexture);
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_1D, 1, &boundTexture);
    MG_Impl::GLImpl::BindTextureUnit(0, boundTexture);

    const auto boundObjectBefore =
        MG_State::pGLContext->GetTextureUnitObject(0).GetBindingSlot(TextureTarget::Texture1D).GetBoundObject();

    MG_Impl::GLImpl::TextureStorage1D(namedTexture, 2, GL_RGBA8, 4);
    const Uint8 pixels[] = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16,
    };
    MG_Impl::GLImpl::TextureSubImage1D(namedTexture, 0, 0, 4, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    const auto textureObject = MG_State::pGLContext->GetTextureObject(namedTexture);
    auto* mipmapObject = static_cast<MG_State::GLState::TextureObjectMipmap*>(textureObject.get());
    ASSERT_NE(mipmapObject, nullptr);
    EXPECT_EQ(mipmapObject->GetMipmapTexelSize(TextureUploadTarget::Texture1D, 0), IntVec3(4, 1, 1));
    EXPECT_EQ(mipmapObject->GetMipmapTexelSize(TextureUploadTarget::Texture1D, 1), IntVec3(2, 1, 1));
    EXPECT_TRUE(mipmapObject->IsStorageDirty(TextureUploadTarget::Texture1D, 0));

    const auto* stored = static_cast<const Uint8*>(mipmapObject->MapMipmapData(TextureUploadTarget::Texture1D, 0));
    ASSERT_NE(stored, nullptr);
    EXPECT_EQ(std::memcmp(stored, pixels, sizeof(pixels)), 0);

    EXPECT_EQ(MG_State::pGLContext->GetTextureUnitObject(0).GetBindingSlot(TextureTarget::Texture1D).GetBoundObject(),
              boundObjectBefore);
    EXPECT_EQ(MG_Impl::GLImpl::GetError(), GL_NO_ERROR);
}

TEST_F(TextureTest, TextureStorage3DAndSubImageModifyNamedObjectOnly) {
    GLuint texture = 0;
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_3D, 1, &texture);
    MG_Impl::GLImpl::TextureStorage3D(texture, 2, GL_R8, 2, 2, 2);

    const Uint8 pixels[] = {
        1, 2, 3, 4,
        5, 6, 7, 8,
    };
    MG_Impl::GLImpl::TextureSubImage3D(texture, 0, 0, 0, 0, 2, 2, 2, GL_RED, GL_UNSIGNED_BYTE, pixels);

    const auto textureObject = MG_State::pGLContext->GetTextureObject(texture);
    auto* mipmapObject = static_cast<MG_State::GLState::TextureObjectMipmap*>(textureObject.get());
    ASSERT_NE(mipmapObject, nullptr);
    EXPECT_EQ(mipmapObject->GetMipmapTexelSize(TextureUploadTarget::Texture3D, 0), IntVec3(2, 2, 2));
    EXPECT_EQ(mipmapObject->GetMipmapTexelSize(TextureUploadTarget::Texture3D, 1), IntVec3(1, 1, 1));
    EXPECT_TRUE(mipmapObject->IsStorageDirty(TextureUploadTarget::Texture3D, 0));

    const auto* stored = static_cast<const Uint8*>(mipmapObject->MapMipmapData(TextureUploadTarget::Texture3D, 0));
    ASSERT_NE(stored, nullptr);
    EXPECT_EQ(std::memcmp(stored, pixels, sizeof(pixels)), 0);
    EXPECT_EQ(MG_Impl::GLImpl::GetError(), GL_NO_ERROR);
}

TEST_F(TextureTest, NamedTextureVectorParametersAndGettersWorkWithoutBinding) {
    GLuint texture = 0;
    MG_Impl::GLImpl::CreateTextures(GL_TEXTURE_2D, 1, &texture);

    const GLfloat borderColor[] = {0.25f, 0.5f, 0.75f, 1.0f};
    const GLint swizzle[] = {GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA};
    MG_Impl::GLImpl::TextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, borderColor);
    MG_Impl::GLImpl::TextureParameterIiv(texture, GL_TEXTURE_SWIZZLE_RGBA, swizzle);

    GLfloat reportedBorder[4] = {};
    GLint reportedSwizzle[4] = {};
    MG_Impl::GLImpl::GetTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, reportedBorder);
    MG_Impl::GLImpl::GetTextureParameterIiv(texture, GL_TEXTURE_SWIZZLE_RGBA, reportedSwizzle);

    EXPECT_FLOAT_EQ(reportedBorder[0], borderColor[0]);
    EXPECT_FLOAT_EQ(reportedBorder[1], borderColor[1]);
    EXPECT_FLOAT_EQ(reportedBorder[2], borderColor[2]);
    EXPECT_FLOAT_EQ(reportedBorder[3], borderColor[3]);
    EXPECT_EQ(reportedSwizzle[0], GL_BLUE);
    EXPECT_EQ(reportedSwizzle[1], GL_GREEN);
    EXPECT_EQ(reportedSwizzle[2], GL_RED);
    EXPECT_EQ(reportedSwizzle[3], GL_ALPHA);
    EXPECT_EQ(MG_Impl::GLImpl::GetError(), GL_NO_ERROR);
}

TEST_F(TextureTest, GetInternalformativReportsBasicTextureMetadata) {
    GLint params[4] = {};

    MG_Impl::GLImpl::GetInternalformativ(GL_TEXTURE_2D, GL_RGBA8, GL_INTERNALFORMAT_SUPPORTED, 1, params);
    EXPECT_EQ(params[0], GL_TRUE);

    MG_Impl::GLImpl::GetInternalformativ(GL_TEXTURE_2D, GL_RGBA8, GL_TEXTURE_IMAGE_FORMAT, 1, params);
    EXPECT_EQ(params[0], GL_RGBA);

    MG_Impl::GLImpl::GetInternalformativ(GL_TEXTURE_2D, GL_RGBA8, GL_TEXTURE_IMAGE_TYPE, 1, params);
    EXPECT_EQ(params[0], GL_UNSIGNED_BYTE);

    MG_Impl::GLImpl::GetInternalformativ(GL_TEXTURE_3D, GL_DEPTH24_STENCIL8, GL_FRAMEBUFFER_RENDERABLE_LAYERED, 1,
                                         params);
    EXPECT_EQ(params[0], GL_FULL_SUPPORT);
    EXPECT_EQ(MG_Impl::GLImpl::GetError(), GL_NO_ERROR);
}

TEST_F(TextureTest, NormalizeDepth24Stencil8UsesPackedDepthStencilType) {
    GLenum internalFormat = 0;
    GLenum format = 0;
    GLenum type = 0;
    MG_Util::TextureFormatProcessor::NormalizePixelFormat(GL_DEPTH24_STENCIL8,
                                                          PixelFormatNormalizeOptionBit::None,
                                                          &internalFormat, &format, &type);

    EXPECT_EQ(internalFormat, GL_DEPTH24_STENCIL8);
    EXPECT_EQ(format, GL_DEPTH_STENCIL);
    EXPECT_EQ(type, GL_UNSIGNED_INT_24_8);
}
