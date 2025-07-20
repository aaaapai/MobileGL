cmake_minimum_required(VERSION 3.24)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(glslang_DIR $ENV{VULKAN_SDK}/macOS/lib/cmake/glslang)
    set(SPIRV-Tools_DIR $ENV{VULKAN_SDK}/macOS/lib/cmake/SPIRV-Tools)
    set(SPIRV-Tools-opt_DIR $ENV{VULKAN_SDK}/macOS/lib/cmake/SPIRV-Tools-opt)
    set(spirv_cross_cpp_DIR $ENV{VULKAN_SDK}/macOS/lib/cmake/spirv_cross_cpp/cmake)
    set(spirv_cross_reflect_DIR $ENV{VULKAN_SDK}/macOS/lib/cmake/spirv_cross_reflect/cmake)
    set(spirv_cross_core_DIR $ENV{VULKAN_SDK}/macOS/lib/cmake/spirv_cross_core/cmake)
    set(spirv_cross_glsl_DIR $ENV{VULKAN_SDK}/macOS/lib/cmake/spirv_cross_glsl/cmake)
    set(spirv_cross_hlsl_DIR $ENV{VULKAN_SDK}/macOS/lib/cmake/spirv_cross_hlsl/cmake)
    set(spirv_cross_msl_DIR $ENV{VULKAN_SDK}/macOS/lib/cmake/spirv_cross_msl/cmake)
    set(spirv_cross_c_DIR $ENV{VULKAN_SDK}/macOS/lib/cmake/spirv_cross_c/cmake)
endif()

find_package(glslang CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_hlsl CONFIG REQUIRED)
find_package(spirv_cross_msl CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)
find_package(spirv_cross_c CONFIG REQUIRED)

add_executable(
        ProgramTest
        ProgramTest.cpp

        ${MGL_ROOT}/MobileGL/MG_Util/Converters/GLToStr/GLEnumConverter.cpp
#        ${MGL_ROOT}/MobileGL/MG_Util/Pipelines/ShaderCompilationPipeline.cpp
#        ${MGL_ROOT}/MobileGL/MG_Util/Pipelines/ProgramLinkingPipeline.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/GLToGlslang/GLShaderLangConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/ShaderTranspiler/ShaderCompiler.cpp
)

target_include_directories(ProgramTest PRIVATE
        ${MGL_ROOT}/include
        ${MGL_ROOT}/MobileGL
)

target_link_libraries(
        ProgramTest
        GTest::gtest_main
        glslang::glslang
        spirv-cross-c
)

include(GoogleTest)
gtest_discover_tests(ProgramTest)