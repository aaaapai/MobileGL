cmake_minimum_required(VERSION 3.14)

add_executable(
        BufferTest
        BufferTest.cpp

        ${MGL_ROOT}/MobileGL/MG_State/GLState/Core.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/BufferState/BufferState.cpp
)

target_include_directories(BufferTest PRIVATE
        ${MGL_ROOT}/include
        ${MGL_ROOT}/MobileGL
)

target_link_libraries(
        BufferTest
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(BufferTest)