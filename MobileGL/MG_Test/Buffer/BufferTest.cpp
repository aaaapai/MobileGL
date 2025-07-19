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

TEST_F(BufferTest, PingPong) {
    auto& readSlot = glContext.GetBufferBindingSlot(BufferTarget::CopyRead);
    auto& writeSlot = glContext.GetBufferBindingSlot(BufferTarget::CopyWrite);
    {
        auto bufferNames = glContext.GenBufferNames(1);
        auto bufObj = glContext.CreateBufferObject(bufferNames[0]);

        writeSlot.Bind(bufObj);
        readSlot.Bind(bufObj);
    }

    auto bufWrite = writeSlot.GetBoundObject();

    Vector<Int> data { 1, 2, 3, 4, 5 };

    SizeT byte_size = data.size() * sizeof(Int);
    // Write data
    bufWrite->Resize(byte_size);
    DataPtr ptr { .data = data.data(), .size = byte_size };
    bufWrite->UploadData(ptr, 0);

    // Readback
    auto bufRead = readSlot.GetBoundObject();
    void* p = bufRead->AcquireMemory(true, true, false);
    ASSERT_TRUE(memcmp(data.data(), p, byte_size) == 0);
    auto range = bufRead->GetDirtyRange();
    ASSERT_EQ(range.start, 0);
    ASSERT_EQ(range.end, byte_size);
}

TEST_F(BufferTest, GenerateManyNames_NoPrematureCreation) {
	const SizeT largeCount = 100000; // generate tons of buffer names
    auto names = glContext.GenBufferNames(largeCount);
    
	std::vector<SizeT> indices = { 0, 600, 5000, 32768, 99999 }; // only create a few buffer objects
    for (SizeT idx : indices) {
        GLuint name = names[idx];
        auto bufObj = glContext.CreateBufferObject(name);
        auto& slot = glContext.GetBufferBindingSlot(BufferTarget::Index);
        slot.Bind(bufObj);

        Vector<Int> data = { static_cast<Int>(idx + 1), static_cast<Int>(idx + 2) };
        SizeT byteSize = data.size() * sizeof(Int);
        bufObj->Resize(byteSize);
        DataPtr ptr{ .data = data.data(), .size = byteSize };
        bufObj->UploadData(ptr, 0);

        Vector<Int> actual(data.size());
        void* p = bufObj->AcquireMemory(false, true, false);
        memcpy(actual.data(), p, byteSize);
        EXPECT_EQ(actual, data);
    }
}

TEST_F(BufferTest, AcquireMemory) {
    auto& slot = glContext.GetBufferBindingSlot(BufferTarget::Uniform);
    auto bufferNames = glContext.GenBufferNames(1);
    auto bufObj = glContext.CreateBufferObject(bufferNames[0]);
    slot.Bind(bufObj);
    Vector<Int> initData{ 10, 20, 30, 40, 50 };
    SizeT byteSize = initData.size() * sizeof(Int);
    bufObj->Resize(byteSize);
    DataPtr ptr{ .data = initData.data(), .size = byteSize };
    bufObj->UploadData(ptr, 0);
    bufObj->ClearDirty();
    Int* mappedPtr = static_cast<Int*>(bufObj->AcquireMemory(true, true, true));
    mappedPtr[0] = 100;
    mappedPtr[1] = 200;
    mappedPtr[2] = 300;
    bufObj->ReleaseMemory();
    Vector<Int> expected{ 100, 200, 300, 40, 50 };
    Vector<Int> actual(5);
    void* p = bufObj->AcquireMemory(false, true, false);
    memcpy(actual.data(), p, byteSize);
    ASSERT_EQ(actual, expected);
    auto dirty = bufObj->GetDirtyRange();

    ASSERT_EQ(dirty.start, 0);
	ASSERT_EQ(dirty.end, sizeof(Int) * 5);
}

TEST_F(BufferTest, AcquireMemoryRangeWithoutExplicit) {
    auto& slot = glContext.GetBufferBindingSlot(BufferTarget::Uniform);
    auto bufferNames = glContext.GenBufferNames(1);
    auto bufObj = glContext.CreateBufferObject(bufferNames[0]);
    slot.Bind(bufObj);
    Vector<Int> initData{ 10, 20, 30, 40, 50 };
    SizeT byteSize = initData.size() * sizeof(Int);
    bufObj->Resize(byteSize);
    DataPtr ptr{ .data = initData.data(), .size = byteSize };
    bufObj->UploadData(ptr, 0);
    bufObj->ClearDirty();

    Range1D mapRange{
        .start = sizeof(Int),
		.end = sizeof(Int) * 4
    };
    Int* mappedPtr = static_cast<Int*>(bufObj->AcquireMemoryRange(
        mapRange,
        BufferMappingAccessBit::Write
    ));
    mappedPtr[0] = 200;
    mappedPtr[1] = 300;
    bufObj->ReleaseMemory();
    Vector<Int> expected{ 10, 200, 300, 40, 50 };
    Vector<Int> actual(5);
    void* p = bufObj->AcquireMemory(false, true, false);
    memcpy(actual.data(), p, byteSize);
    ASSERT_EQ(actual, expected);
    auto dirty = bufObj->GetDirtyRange();
    ASSERT_EQ(dirty.start, sizeof(Int));
	ASSERT_EQ(dirty.end, sizeof(Int) * 4);
}

TEST_F(BufferTest, AcquireMemoryRangeWithExplicit) {
    auto& slot = glContext.GetBufferBindingSlot(BufferTarget::Uniform);
    auto bufferNames = glContext.GenBufferNames(1);
    auto bufObj = glContext.CreateBufferObject(bufferNames[0]);
    slot.Bind(bufObj);

    Vector<Int> initData{ 10, 20, 30, 40, 50 };
    SizeT byteSize = initData.size() * sizeof(Int);
    bufObj->Resize(byteSize);
    DataPtr ptr{ .data = initData.data(), .size = byteSize };
    bufObj->UploadData(ptr, 0);

    bufObj->ClearDirty();

    Range1D mapRange{
        .start = sizeof(Int),
        .end = sizeof(Int) * 4
    };
    Int* mappedPtr = static_cast<Int*>(bufObj->AcquireMemoryRange(
        mapRange,
        BufferMappingAccessBit::Write | BufferMappingAccessBit::FlushExplicit
    ));

    mappedPtr[0] = 200;
    mappedPtr[1] = 300;

    bufObj->FlushMemoryRange(0, sizeof(Int));
    auto dirty = bufObj->GetDirtyRange();
	ASSERT_EQ(dirty.start, sizeof(Int));
	ASSERT_EQ(dirty.end, sizeof(Int) * 2);

    bufObj->ReleaseMemory();

    dirty = bufObj->GetDirtyRange();
    ASSERT_EQ(dirty.start, sizeof(Int));
    ASSERT_EQ(dirty.end, sizeof(Int) * 2);

    Vector<Int> expected{ 10, 200, 30, 40, 50 };
    Vector<Int> actual(5);
    void* p = bufObj->AcquireMemory(false, true, false);
    memcpy(actual.data(), p, byteSize);
    ASSERT_EQ(actual, expected);

    dirty = bufObj->GetDirtyRange();
    ASSERT_EQ(dirty.start, sizeof(Int));
    ASSERT_EQ(dirty.end, sizeof(Int) * 2); 
}

TEST_F(BufferTest, CopyBufferSubData) {
    auto& srcSlot = glContext.GetBufferBindingSlot(BufferTarget::CopyRead);
    auto& dstSlot = glContext.GetBufferBindingSlot(BufferTarget::CopyWrite);

    auto srcNames = glContext.GenBufferNames(1);
    auto srcObj = glContext.CreateBufferObject(srcNames[0]);
    srcSlot.Bind(srcObj);

    Vector<Int> srcData{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    SizeT srcSize = srcData.size() * sizeof(Int);
    srcObj->Resize(srcSize);
    DataPtr srcPtr{ .data = srcData.data(), .size = srcSize };
    srcObj->UploadData(srcPtr, 0);

    auto dstNames = glContext.GenBufferNames(1);
    auto dstObj = glContext.CreateBufferObject(dstNames[0]);
    dstSlot.Bind(dstObj);

    Vector<Int> dstData(15, 0);
    SizeT dstSize = dstData.size() * sizeof(Int);
    dstObj->Resize(dstSize);
    DataPtr dstPtr{ .data = dstData.data(), .size = dstSize };
    dstObj->UploadData(dstPtr, 0);

    srcObj->ClearDirty();
    dstObj->ClearDirty();

    dstObj->CopyDataFrom(srcObj,
        2 * sizeof(Int),
        5 * sizeof(Int),
        4 * sizeof(Int));

    Vector<Int> expected{
        0, 0, 0, 0, 0,
        3, 4, 5, 6,
        0, 0, 0, 0, 0, 0 
    };

    Vector<Int> actual(15);
    void* p = dstObj->AcquireMemory(false, true, false);
    memcpy(actual.data(), p, dstSize);

    ASSERT_EQ(actual, expected);

    auto dirty = dstObj->GetDirtyRange();
    ASSERT_EQ(dirty.start, 5 * sizeof(Int));
    ASSERT_EQ(dirty.end, 9 * sizeof(Int));
}

TEST_F(BufferTest, WriteWhileMapped) {
    auto& slot = glContext.GetBufferBindingSlot(BufferTarget::ShaderStorage);
    auto bufferNames = glContext.GenBufferNames(1);
    auto bufObj = glContext.CreateBufferObject(bufferNames[0]);
    slot.Bind(bufObj);

    Vector<Int> initData(10, 0);
    SizeT byteSize = initData.size() * sizeof(Int);
    bufObj->Resize(byteSize);

    Int* mappedPtr = static_cast<Int*>(bufObj->AcquireMemory(true, true, true));

    for (int i = 0; i < 10; i++) {
        mappedPtr[i] = i * 10;
    }

    bufObj->ReleaseMemory();

    Vector<Int> expected{ 0, 10, 20, 30, 40, 50, 60, 70, 80, 90 };
    Vector<Int> actual(10);
    void* p = bufObj->AcquireMemory(false, true, false);
    memcpy(actual.data(), p, byteSize);

    ASSERT_EQ(actual, expected);

    auto dirty = bufObj->GetDirtyRange();
    ASSERT_EQ(dirty.start, 0);
    ASSERT_EQ(dirty.end, byteSize);
}

TEST_F(BufferTest, PartialUpdate) {
    auto& slot = glContext.GetBufferBindingSlot(BufferTarget::Vertex);
    auto bufferNames = glContext.GenBufferNames(1);
    auto bufObj = glContext.CreateBufferObject(bufferNames[0]);
    slot.Bind(bufObj);

    Vector<Int> initData{ 100, 200, 300, 400, 500 };
    SizeT byteSize = initData.size() * sizeof(Int);
    bufObj->Resize(byteSize);
    DataPtr ptr{ .data = initData.data(), .size = byteSize };
    bufObj->UploadData(ptr, 0);
	bufObj->ClearDirty();

    Vector<Int> update{ 999, 888 };
    bufObj->UploadSubData(
        sizeof(Int),
        update.size() * sizeof(Int),
        update.data()
    );

    Vector<Int> expected{ 100, 999, 888, 400, 500 };
    Vector<Int> actual(5);
    void* p = bufObj->AcquireMemory(false, true, false);
    memcpy(actual.data(), p, byteSize);

    ASSERT_EQ(actual, expected);

    auto dirty = bufObj->GetDirtyRange();
    ASSERT_EQ(dirty.start, sizeof(Int));
    ASSERT_EQ(dirty.end, 3 * sizeof(Int));
}
