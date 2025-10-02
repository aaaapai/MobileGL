//
// Created by Swung 0x48 on 2025/7/17.
//
#include <gtest/gtest.h>

#include "Includes.h"
#include <MG_State/GLState/Core.h>

#include <MG_Impl/GLImpl/Buffer/GL_Buffer.h>
#include <MG_Impl/GLImpl/Getter/GL_Getter.h>

using namespace MobileGL;

class BufferTest : public ::testing::Test {
protected:
    MG_State::GLState::GLContext glContext;
};

TEST_F(BufferTest, Binding) {
    auto bufferNames = glContext.GenBufferNames(3);
    auto& arraySlot = glContext.GetBufferBindingSlot(BufferTarget::Vertex);
    auto& indexSlot = glContext.GetBufferBindingSlot(BufferTarget::Uniform);

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

    Vector<Int> data{1, 2, 3, 4, 5};

    SizeT byteSize = data.size() * sizeof(Int);
    // Write data
    bufWrite->Resize(byteSize);
    DataPtr ptr{.data = data.data(), .size = byteSize};
    bufWrite->UploadData(ptr, 0);

    // Readback
    auto bufRead = readSlot.GetBoundObject();
    void* p = bufRead->AcquireMemory(true, true, false);
    Vector<Int> bufdata(data.size());
    memcpy(bufdata.data(), p, byteSize);
    ASSERT_EQ(data, bufdata);
    auto range = bufRead->GetDirtyRange();
    ASSERT_EQ(range.start, 0);
    ASSERT_EQ(range.end, byteSize);
}

TEST_F(BufferTest, GenerateManyNames_NoPrematureCreation) {
    const SizeT largeCount = 100000; // generate tons of buffer names
    auto names = glContext.GenBufferNames(largeCount);

    std::vector<SizeT> indices = {0, 600, 5000, 32768, 99999}; // only create a few buffer objects
    for (SizeT idx : indices) {
        GLuint name = names[idx];
        auto bufObj = glContext.CreateBufferObject(name);
        auto& slot = glContext.GetBufferBindingSlot(BufferTarget::Uniform);
        slot.Bind(bufObj);

        Vector<Int> data = {static_cast<Int>(idx + 1), static_cast<Int>(idx + 2)};
        SizeT byteSize = data.size() * sizeof(Int);
        bufObj->Resize(byteSize);
        DataPtr ptr{.data = data.data(), .size = byteSize};
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
    Vector<Int> initData{10, 20, 30, 40, 50};
    SizeT byteSize = initData.size() * sizeof(Int);
    bufObj->Resize(byteSize);
    DataPtr ptr{.data = initData.data(), .size = byteSize};
    bufObj->UploadData(ptr, 0);
    bufObj->ClearDirty();
    Int* mappedPtr = static_cast<Int*>(bufObj->AcquireMemory(true, true, true));
    mappedPtr[0] = 100;
    mappedPtr[1] = 200;
    mappedPtr[2] = 300;
    bufObj->ReleaseMemory();
    Vector<Int> expected{100, 200, 300, 40, 50};
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
    Vector<Int> initData{10, 20, 30, 40, 50};
    SizeT byteSize = initData.size() * sizeof(Int);
    bufObj->Resize(byteSize);
    DataPtr ptr{.data = initData.data(), .size = byteSize};
    bufObj->UploadData(ptr, 0);
    bufObj->ClearDirty();

    Range1D mapRange{.start = sizeof(Int), .end = sizeof(Int) * 4};
    Int* mappedPtr = static_cast<Int*>(bufObj->AcquireMemoryRange(mapRange, BufferMappingAccessBit::Write));
    mappedPtr[0] = 200;
    mappedPtr[1] = 300;
    bufObj->ReleaseMemory();
    Vector<Int> expected{10, 200, 300, 40, 50};
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

    Vector<Int> initData{10, 20, 30, 40, 50};
    SizeT byteSize = initData.size() * sizeof(Int);
    bufObj->Resize(byteSize);
    DataPtr ptr{.data = initData.data(), .size = byteSize};
    bufObj->UploadData(ptr, 0);

    bufObj->ClearDirty();

    Range1D mapRange{.start = sizeof(Int), .end = sizeof(Int) * 4};
    Int* mappedPtr = static_cast<Int*>(
        bufObj->AcquireMemoryRange(mapRange, BufferMappingAccessBit::Write | BufferMappingAccessBit::FlushExplicit));

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

    Vector<Int> expected{10, 200, 30, 40, 50};
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

    Vector<Int> srcData{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    SizeT srcSize = srcData.size() * sizeof(Int);
    srcObj->Resize(srcSize);
    DataPtr srcPtr{.data = srcData.data(), .size = srcSize};
    srcObj->UploadData(srcPtr, 0);

    auto dstNames = glContext.GenBufferNames(1);
    auto dstObj = glContext.CreateBufferObject(dstNames[0]);
    dstSlot.Bind(dstObj);

    Vector<Int> dstData(15, 0);
    SizeT dstSize = dstData.size() * sizeof(Int);
    dstObj->Resize(dstSize);
    DataPtr dstPtr{.data = dstData.data(), .size = dstSize};
    dstObj->UploadData(dstPtr, 0);

    srcObj->ClearDirty();
    dstObj->ClearDirty();

    dstObj->CopyDataFrom(srcObj, 2 * sizeof(Int), 5 * sizeof(Int), 4 * sizeof(Int));

    Vector<Int> expected{0, 0, 0, 0, 0, 3, 4, 5, 6, 0, 0, 0, 0, 0, 0};

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

    Vector<Int> expected{0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
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

    Vector<Int> initData{100, 200, 300, 400, 500};
    SizeT byteSize = initData.size() * sizeof(Int);
    bufObj->Resize(byteSize);
    DataPtr ptr{.data = initData.data(), .size = byteSize};
    bufObj->UploadData(ptr, 0);
    bufObj->ClearDirty();

    Vector<Int> update{999, 888};
    bufObj->UploadSubData({(void*)(update.data()), (SizeT)(update.size() * sizeof(Int))}, sizeof(Int));

    Vector<Int> expected{100, 999, 888, 400, 500};
    Vector<Int> actual(5);
    void* p = bufObj->AcquireMemory(false, true, false);
    memcpy(actual.data(), p, byteSize);

    ASSERT_EQ(actual, expected);

    auto dirty = bufObj->GetDirtyRange();
    ASSERT_EQ(dirty.start, sizeof(Int));
    ASSERT_EQ(dirty.end, 3 * sizeof(Int));
}

TEST_F(BufferTest, DeleteBufferObject) {
    auto bufferNames = glContext.GenBufferNames(1);
    auto& slot = glContext.GetBufferBindingSlot(BufferTarget::Vertex);
    auto bufObj = glContext.CreateBufferObject(bufferNames[0]);
    slot.Bind(bufObj);
    ASSERT_TRUE(slot.GetBoundObject() == bufObj);
    glContext.MarkBufferObjectForDeletion(bufferNames[0]);
    ASSERT_TRUE(slot.GetBoundObject() == nullptr);
    ASSERT_FALSE(glContext.GetBufferObject(bufferNames[0]));
}

using namespace MobileGL::MG_Impl::GLImpl;

class GeneralBufferTest : public ::testing::Test {
protected:
    void SetUp() override { MG_State::pGLContext = new MG_State::GLState::GLContext(); }

    GLuint CreateBoundBuffer(GLenum target, GLsizeiptr size, GLenum usage) {
        GLuint buffer;
        GenBuffers(1, &buffer);
        BindBuffer(target, buffer);
        BufferData(target, size, nullptr, usage);
        EXPECT_EQ(GetError(), GL_NO_ERROR);
        return buffer;
    }
};

TEST_F(GeneralBufferTest, General_BufferLifecycle) {
    GLuint buffers[3];

    GenBuffers(3, buffers);
    EXPECT_NE(buffers[0], 0);
    EXPECT_NE(buffers[1], 0);
    EXPECT_NE(buffers[2], 0);
    EXPECT_NE(buffers[0], buffers[1]);

    GLuint deleteBuf = buffers[1];
    DeleteBuffers(1, &deleteBuf);

    BindBuffer(GL_ARRAY_BUFFER, deleteBuf);
    EXPECT_EQ(GetError(), GL_INVALID_OPERATION);

    EXPECT_EQ(GetError(), GL_NO_ERROR);
}

TEST_F(GeneralBufferTest, General_BufferDataOperations) {
    GLuint buffer = CreateBoundBuffer(GL_ARRAY_BUFFER, 100, GL_STATIC_DRAW);

    const char initData[100] = {0};
    char readBack[100];
    void* mapped = MapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
    ASSERT_NE(mapped, nullptr);
    memcpy(readBack, mapped, 100);
    EXPECT_EQ(memcmp(initData, readBack, 100), 0);
    UnmapBuffer(GL_ARRAY_BUFFER);

    const char subData[] = "TEST";
    BufferSubData(GL_ARRAY_BUFFER, 10, sizeof(subData), subData);
    mapped = MapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
    memcpy(readBack, (char*)mapped + 10, sizeof(subData));
    EXPECT_STREQ(readBack, "TEST");
    UnmapBuffer(GL_ARRAY_BUFFER);

    EXPECT_EQ(GetError(), GL_NO_ERROR);
}

TEST_F(GeneralBufferTest, General_BufferCopy) {
    GLuint src = CreateBoundBuffer(GL_COPY_READ_BUFFER, 50, GL_STATIC_READ);
    GLuint dst = CreateBoundBuffer(GL_COPY_WRITE_BUFFER, 50, GL_STATIC_DRAW);

    const char srcData[] = "SOURCE_BUFFER_DATA";
    BufferSubData(GL_COPY_READ_BUFFER, 0, sizeof(srcData), srcData);

    void* srcMappedData = MapBuffer(GL_COPY_READ_BUFFER, GL_READ_ONLY);
    EXPECT_STREQ((char*)srcMappedData, "SOURCE_BUFFER_DATA");
    UnmapBuffer(GL_COPY_READ_BUFFER);

    CopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 10, sizeof(srcData));

    char result[50];
    void* mapped = MapBuffer(GL_COPY_WRITE_BUFFER, GL_READ_ONLY);
    memcpy(result, (char*)mapped + 10, sizeof(srcData));
    EXPECT_STREQ(result, "SOURCE_BUFFER_DATA");
    UnmapBuffer(GL_COPY_WRITE_BUFFER);

    EXPECT_EQ(GetError(), GL_NO_ERROR);
}

TEST_F(GeneralBufferTest, General_BufferMapping) {
    GLuint buffer = 0;
    GenBuffers(1, &buffer);
    BindBuffer(GL_ARRAY_BUFFER, buffer);
    Vector<char> data;
    data.resize(100, '\0');
    BufferData(GL_ARRAY_BUFFER, data.size(), data.data(), GL_DYNAMIC_DRAW);

    void* fullMap = MapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    ASSERT_NE(fullMap, nullptr);
    strcpy((char*)fullMap, "   Full mapping test");
    EXPECT_TRUE(UnmapBuffer(GL_ARRAY_BUFFER));

    void* partialMap = MapBufferRange(GL_ARRAY_BUFFER, 0, 30, GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
    ASSERT_NE(partialMap, nullptr);
    strcpy((char*)partialMap, "Partial (not valid value)");

    FlushMappedBufferRange(GL_ARRAY_BUFFER, 0, 7);
    EXPECT_TRUE(UnmapBuffer(GL_ARRAY_BUFFER));

    partialMap = MapBufferRange(GL_ARRAY_BUFFER, 20, 40, GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
    ASSERT_NE(partialMap, nullptr);
    strcpy((char*)partialMap, ": modified data (not valid value)");

    FlushMappedBufferRange(GL_ARRAY_BUFFER, 0, 15);
    EXPECT_TRUE(UnmapBuffer(GL_ARRAY_BUFFER));

    char verify[40];
    void* verifyMap = MapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
    memcpy(verify, (char*)verifyMap, 40);
    EXPECT_STREQ(verify, "Partial mapping test: modified data");
    UnmapBuffer(GL_ARRAY_BUFFER);

    EXPECT_EQ(GetError(), GL_NO_ERROR);
}

TEST_F(GeneralBufferTest, General_InvalidOperations) {
    EXPECT_EQ(MapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY), nullptr); // GL_INVALID_OPERATION
    EXPECT_EQ(GetError(), GL_INVALID_OPERATION);

    GLuint buffer = CreateBoundBuffer(GL_ARRAY_BUFFER, 50, GL_STREAM_READ);

    EXPECT_EQ(MapBuffer(0xFFFFFFFF, GL_READ_ONLY), nullptr); // GL_INVALID_ENUM

    void* mapped = MapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    EXPECT_TRUE(UnmapBuffer(GL_ARRAY_BUFFER));
    EXPECT_FALSE(UnmapBuffer(GL_ARRAY_BUFFER)); // GL_INVALID_OPERATION

    EXPECT_EQ(GetError(), GL_INVALID_ENUM);
    EXPECT_EQ(GetError(), GL_INVALID_OPERATION);

    EXPECT_EQ(GetError(), GL_NO_ERROR);
}

TEST_F(GeneralBufferTest, General_MapFlags) {
    GLuint buffer = CreateBoundBuffer(GL_ARRAY_BUFFER, 100, GL_DYNAMIC_COPY);

    void* roMap = MapBufferRange(GL_ARRAY_BUFFER, 0, 100, GL_MAP_READ_BIT);
    ASSERT_NE(roMap, nullptr);

    EXPECT_TRUE(UnmapBuffer(GL_ARRAY_BUFFER));

    void* nosyncMap = MapBufferRange(GL_ARRAY_BUFFER, 0, 100, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    ASSERT_NE(nosyncMap, nullptr);
    memset(nosyncMap, 0xAA, 100);
    EXPECT_TRUE(UnmapBuffer(GL_ARRAY_BUFFER));

    EXPECT_EQ(GetError(), GL_NO_ERROR);
}

TEST_F(GeneralBufferTest, General_GeneralTest_1) {
    GLuint buffers[3];
    GenBuffers(3, buffers);
    const GLuint vbo = buffers[0];
    const GLuint ibo = buffers[1];
    const GLuint staging = buffers[2];

    BindBuffer(GL_ARRAY_BUFFER, vbo);
    BufferData(GL_ARRAY_BUFFER, 64, nullptr, GL_STATIC_DRAW);

    const float vertexData[] = {0.1f, 0.2f, 0.3f, 1.0f};
    BufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexData), vertexData);

    BindBuffer(GL_UNIFORM_BUFFER, ibo);
    const uint16_t indexData[] = {0, 1, 2, 3, 0};
    BufferData(GL_UNIFORM_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

    const uint16_t newIndices[] = {4, 5};
    BufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(uint16_t), sizeof(newIndices), newIndices);

    BindBuffer(GL_COPY_READ_BUFFER, staging);
    const char stagingData[] = "StagingBufferData";
    BufferData(GL_COPY_READ_BUFFER, sizeof(stagingData), stagingData, GL_STREAM_COPY);

    CopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 40, sizeof(stagingData));

    BindBuffer(GL_UNIFORM_BUFFER, ibo);
    void* fullMap = MapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE);
    ASSERT_NE(fullMap, nullptr);

    uint16_t* indices = static_cast<uint16_t*>(fullMap);
    indices[0] = 10;

    EXPECT_TRUE(UnmapBuffer(GL_UNIFORM_BUFFER));

    BindBuffer(GL_ARRAY_BUFFER, vbo);
    void* partialMap = MapBufferRange(GL_ARRAY_BUFFER, 20, 8, GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
    ASSERT_NE(partialMap, nullptr);

    const char partialWriteData[] = "PARTIAL"; // 7 chars + '\0' = 8 bytes
    memcpy(partialMap, partialWriteData, sizeof(partialWriteData));
    FlushMappedBufferRange(GL_ARRAY_BUFFER, 0, sizeof(partialWriteData));

    EXPECT_TRUE(UnmapBuffer(GL_ARRAY_BUFFER));

    BindBuffer(GL_ARRAY_BUFFER, vbo);
    void* verifyMap = MapBufferRange(GL_ARRAY_BUFFER, 0, 64, GL_MAP_READ_BIT);
    ASSERT_NE(verifyMap, nullptr);

    const float* verts = static_cast<const float*>(verifyMap);
    EXPECT_FLOAT_EQ(verts[0], 0.1f);
    EXPECT_FLOAT_EQ(verts[1], 0.2f);

    const char* partialData = static_cast<const char*>(verifyMap) + 20;
    EXPECT_STREQ(partialData, "PARTIAL");

    const char* copiedData = static_cast<const char*>(verifyMap) + 40;
    EXPECT_STREQ(copiedData, "StagingBufferData");

    EXPECT_TRUE(UnmapBuffer(GL_ARRAY_BUFFER));

    BindBuffer(GL_UNIFORM_BUFFER, ibo);
    void* iboMap = MapBuffer(GL_UNIFORM_BUFFER, GL_READ_ONLY);
    const uint16_t* finalIndices = static_cast<const uint16_t*>(iboMap);
    EXPECT_EQ(finalIndices[0], 10);
    EXPECT_EQ(finalIndices[2], 4);
    EXPECT_TRUE(UnmapBuffer(GL_UNIFORM_BUFFER));

    DeleteBuffers(1, &staging);

    BindBuffer(GL_COPY_READ_BUFFER, staging);
    GLenum err = GetError();
    EXPECT_EQ(err, GL_INVALID_OPERATION);

    GLuint toDelete[] = {vbo, ibo};
    DeleteBuffers(2, toDelete);

    EXPECT_EQ(GetError(), GL_NO_ERROR);
}