#include <gtest/gtest.h>
#include "Includes.h"

using namespace MobileGL;

class VertexArrayTest : public ::testing::Test {
protected:
    MG_State::GLState::GLContext glContext;

    SharedPtr<MG_State::GLState::BufferObject> CreateTestVBO() {
        auto bufferNames = glContext.GenBufferNames(1);
        auto vbo = glContext.CreateBufferObject(bufferNames[0]);
        glContext.GetBufferBindingSlot(BufferTarget::Vertex).Bind(vbo);

        Vector<float> vertexData = {
            0.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 1.0f
        };
        SizeT byteSize = vertexData.size() * sizeof(float);
        vbo->Resize(byteSize);
        DataPtr ptr{ .data = vertexData.data(), .size = byteSize };
        vbo->UploadData(ptr, 0);

        return vbo;
    }
};

TEST_F(VertexArrayTest, GenerateAndBindVAO) {
    auto vaoNames = glContext.GenVertexArrayNames(2);
    auto vao0 = glContext.CreateVertexArrayObject(vaoNames[0]);
    auto vao1 = glContext.CreateVertexArrayObject(vaoNames[1]);

    glContext.BindVertexArray(vaoNames[0]);
    ASSERT_EQ(glContext.GetBoundVertexArray(), vao0);

    glContext.BindVertexArray(vaoNames[1]);
    ASSERT_EQ(glContext.GetBoundVertexArray(), vao1);

    glContext.BindVertexArray(0);
    ASSERT_EQ(glContext.GetBoundVertexArray(), nullptr);
}

TEST_F(VertexArrayTest, VertexAttributeSetup) {
    auto vaoNames = glContext.GenVertexArrayNames(1);
    auto vao = glContext.CreateVertexArrayObject(vaoNames[0]);
    glContext.BindVertexArray(vaoNames[0]);

    auto vbo = CreateTestVBO();

    vao->EnableAttribute(0);
    vao->SetAttributeFormat(0, 4, DataType::Float32, false, 8 * sizeof(float), 0, false);
    vao->BindAttributeBuffer(0, vbo);

    vao->EnableAttribute(1);
    vao->SetAttributeFormat(1, 4, DataType::Float32, false, 8 * sizeof(float), 4 * sizeof(float), false);
    vao->BindAttributeBuffer(1, vbo);

    const auto& attr0 = vao->GetAttribute(0);
    ASSERT_TRUE(attr0.Enabled);
    ASSERT_EQ(attr0.Size, 4);
    ASSERT_EQ(attr0.Type, DataType::Float32);
    ASSERT_EQ(attr0.Stride, 8 * sizeof(float));
    ASSERT_EQ(attr0.Offset, 0);
    ASSERT_EQ(attr0.Buffer, vbo);

    const auto& attr1 = vao->GetAttribute(1);
    ASSERT_TRUE(attr1.Enabled);
    ASSERT_EQ(attr1.Offset, 4 * sizeof(float));

    vao->DisableAttribute(1);
    ASSERT_FALSE(vao->IsAttributeEnabled(1));
}

TEST_F(VertexArrayTest, ElementBufferBinding) {
    auto vaoNames = glContext.GenVertexArrayNames(1);
    auto vao = glContext.CreateVertexArrayObject(vaoNames[0]);
    glContext.BindVertexArray(vaoNames[0]);

    auto bufferNames = glContext.GenBufferNames(1);
    auto ebo = glContext.CreateBufferObject(bufferNames[0]);
    glContext.GetBufferBindingSlot(BufferTarget::Index).Bind(ebo);

    Vector<Uint> indices = { 0, 1, 2 };
    SizeT byteSize = indices.size() * sizeof(Uint);
    ebo->Resize(byteSize);
    DataPtr ptr{ .data = indices.data(), .size = byteSize };
    ebo->UploadData(ptr, 0);

    vao->BindElementBuffer(ebo);
    ASSERT_EQ(vao->GetElementBuffer(), ebo);

    auto newEboNames = glContext.GenBufferNames(1);
    auto newEbo = glContext.CreateBufferObject(newEboNames[0]);
    vao->BindElementBuffer(newEbo);
    ASSERT_EQ(vao->GetElementBuffer(), newEbo);
}

TEST_F(VertexArrayTest, DeleteVAO) {
    auto vaoNames = glContext.GenVertexArrayNames(1);
    auto vao = glContext.CreateVertexArrayObject(vaoNames[0]);

    glContext.BindVertexArray(vaoNames[0]);
    ASSERT_EQ(glContext.GetBoundVertexArray(), vao);

    glContext.MarkVertexArrayForDeletion(vaoNames[0]);

    ASSERT_FALSE(glContext.ValidateVertexArrayObject(vaoNames[0]));
    ASSERT_EQ(glContext.GetVertexArrayObject(vaoNames[0]), nullptr);
    ASSERT_EQ(glContext.GetBoundVertexArray(), nullptr);
}

TEST_F(VertexArrayTest, ValidateNamesAndObjects) {
    const Uint count = 5;
    auto vaoNames = glContext.GenVertexArrayNames(count);

    for (Uint i = 0; i < count; i++) {
        ASSERT_TRUE(glContext.ValidateVertexArrayName(vaoNames[i]));
        ASSERT_FALSE(glContext.ValidateVertexArrayObject(vaoNames[i]));
    }

    for (Uint i = 0; i < count; i += 2) {
        glContext.CreateVertexArrayObject(vaoNames[i]);
        ASSERT_TRUE(glContext.ValidateVertexArrayObject(vaoNames[i]));
    }

    for (Uint i = 1; i < count; i += 2) {
        glContext.MarkVertexArrayForDeletion(vaoNames[i]);
        ASSERT_FALSE(glContext.ValidateVertexArrayName(vaoNames[i]));
    }
}

TEST_F(VertexArrayTest, MultipleAttributes) {
    auto vaoNames = glContext.GenVertexArrayNames(1);
    auto vao = glContext.CreateVertexArrayObject(vaoNames[0]);
    glContext.BindVertexArray(vaoNames[0]);

    auto vboPos = CreateTestVBO();
    auto vboNormalNames = glContext.GenBufferNames(1);
    auto vboNormal = glContext.CreateBufferObject(vboNormalNames[0]);

    Vector<float> normals(12, 0.5f);
    SizeT byteSize = normals.size() * sizeof(float);
    vboNormal->Resize(byteSize);
    DataPtr ptr{ .data = normals.data(), .size = byteSize };
    vboNormal->UploadData(ptr, 0);

    vao->EnableAttribute(0);
    vao->SetAttributeFormat(0, 3, DataType::Float32, false, 3 * sizeof(float), 0, false);
    vao->BindAttributeBuffer(0, vboPos);

    vao->EnableAttribute(1);
    vao->SetAttributeFormat(1, 3, DataType::Float32, true, 3 * sizeof(float), 0, false);
    vao->BindAttributeBuffer(1, vboNormal);

    vao->EnableAttribute(2);
    vao->SetAttributeFormat(2, 4, DataType::Uint8, true, 4 * sizeof(Uint8), 0, false);

    const auto& attr0 = vao->GetAttribute(0);
    ASSERT_EQ(attr0.Size, 3);
    ASSERT_EQ(attr0.Buffer, vboPos);

    const auto& attr1 = vao->GetAttribute(1);
    ASSERT_TRUE(attr1.Normalized);
    ASSERT_EQ(attr1.Buffer, vboNormal);

    const auto& attr2 = vao->GetAttribute(2);
    ASSERT_EQ(attr2.Type, DataType::Uint8);
    ASSERT_EQ(attr2.Buffer, nullptr);

    vao->DisableAttribute(1);
    vao->EnableAttribute(1);
    ASSERT_TRUE(vao->IsAttributeEnabled(1));
}

TEST_F(VertexArrayTest, BoundVAOPreservesState) {
    auto vaoNames = glContext.GenVertexArrayNames(2);
    auto vao1 = glContext.CreateVertexArrayObject(vaoNames[0]);
    auto vao2 = glContext.CreateVertexArrayObject(vaoNames[1]);

    auto vbo = CreateTestVBO();

    glContext.BindVertexArray(vaoNames[0]);
    vao1->EnableAttribute(0);
    vao1->SetAttributeFormat(0, 4, DataType::Float32, false, 0, 0, false);
    vao1->BindAttributeBuffer(0, vbo);

    glContext.BindVertexArray(vaoNames[1]);
    vao2->EnableAttribute(1);
    vao2->SetAttributeFormat(1, 3, DataType::Float32, true, 0, 0, false);

    glContext.BindVertexArray(vaoNames[0]);

    const auto& attr = vao1->GetAttribute(0);
    ASSERT_TRUE(attr.Enabled);
    ASSERT_EQ(attr.Size, 4);
    ASSERT_EQ(attr.Buffer, vbo);

    ASSERT_FALSE(vao2->IsAttributeEnabled(0));
}
