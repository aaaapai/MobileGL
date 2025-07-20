cmake_minimum_required(VERSION 3.24)

set(glslang_DIR /Users/swung0x48/VulkanSDK/1.4.321.0/macOS/lib/cmake/glslang)
set(SPIRV-Tools_DIR /Users/swung0x48/VulkanSDK/1.4.321.0/macOS/lib/cmake/SPIRV-Tools)
set(SPIRV-Tools-opt_DIR /Users/swung0x48/VulkanSDK/1.4.321.0/macOS/lib/cmake/SPIRV-Tools-opt)
#find_package(SPIRV-Tools CONFIG REQUIRED)
#find_package(SPIRV-Tools-opt CONFIG REQUIRED)
find_package(glslang CONFIG)

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
)

include(GoogleTest)
gtest_discover_tests(ProgramTest)