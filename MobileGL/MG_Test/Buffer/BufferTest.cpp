//
// Created by Swung 0x48 on 2025/7/17.
//
#include <gtest/gtest.h>

#include "Includes.h"

using namespace MobileGL;

class BufferTest : public ::testing::Test {
protected:
    MG_State::GLState::GLContext glContext;
};

TEST_F(BufferTest, Binding) {
    auto bufferNames = glContext.GenBufferNames(3);
    auto& arraySlot = glContext.GetBufferBindingSlot(BufferTarget::Vertex);
    auto& indexSlot = glContext.GetBufferBindingSlot(BufferTarget::Index);

    auto obj0 = glContext.CreateBufferObject(bufferNames[0]);
    auto obj1 = glContext.CreateBufferObject(bufferNames[1]);
    auto obj2 = glContext.CreateBufferObject(bufferNames[2]);

    arraySlot.Bind(obj0);
    indexSlot.Bind(obj1);

    ASSERT_TRUE(arraySlot.GetBoundObject() == obj0);
    ASSERT_TRUE(indexSlot.GetBoundObject() == obj1);

    arraySlot.Bind(obj2);
    indexSlot.Bind(obj2);
    ASSERT_TRUE(arraySlot.GetBoundObject() == obj2);
    ASSERT_TRUE(indexSlot.GetBoundObject() == obj2);
}