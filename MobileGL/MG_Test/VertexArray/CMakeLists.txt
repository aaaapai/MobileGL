cmake_minimum_required(VERSION 3.14)

add_executable(
        VertexArrayTest
        VertexArrayTest.cpp

        ${MGL_ROOT}/MobileGL/MG_State/GLState/Core.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/BufferState/BufferState.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/BufferState/BufferObject.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/VertexArrayState/VertexArrayObject.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/VertexArrayState/VertexArrayState.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/ErrorState/Error.cpp
        ${MGL_ROOT}/MobileGL/MG_Impl/GLImpl/Buffer/GL_Buffer.cpp
        ${MGL_ROOT}/MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp
        ${MGL_ROOT}/MobileGL/MG_Impl/GLImpl/Buffer/Validators.cpp
        ${MGL_ROOT}/MobileGL/MG_Impl/GLImpl/VertexArray/Validators.cpp
        ${MGL_ROOT}/MobileGL/MG_Impl/GLImpl/VertexArray/GL_VertexArray.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/GLToStr/GLEnumConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/MGToStr/GLExtensionConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/MGToStr/BufferEnumConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/GLToGlslang/GLShaderLangConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/MGToGL/ErrorCodeConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/MGToGL/BufferEnumConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/MGToGL/DataTypeConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/GLToMG/BufferEnumConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/GLToMG/DataTypeConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Backend/Init.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Debug/Log.cpp
)

target_include_directories(VertexArrayTest PRIVATE
        ${MGL_ROOT}/include
        ${MGL_ROOT}/MobileGL
)

target_link_libraries(
        VertexArrayTest
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(VertexArrayTest)