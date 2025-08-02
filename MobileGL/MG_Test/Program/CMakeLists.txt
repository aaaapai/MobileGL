cmake_minimum_required(VERSION 3.24)

# Vulkan SDK's cmake config is fundamentally broken. Directly find libs instead
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_package(Vulkan REQUIRED)
    get_filename_component(VULKAN_LIB_DIR ${Vulkan_LIBRARY} PATH)

    set(LINK_LIBRARIES
            ${VULKAN_LIB_DIR}/libglslang.a
            ${VULKAN_LIB_DIR}/libspirv-cross-c-shared.dylib
            ${VULKAN_LIB_DIR}/libGenericCodeGen.a
            ${VULKAN_LIB_DIR}/libglslang-default-resource-limits.a
            ${VULKAN_LIB_DIR}/libMachineIndependent.a
            ${VULKAN_LIB_DIR}/libOSDependent.a
            ${VULKAN_LIB_DIR}/libSPIRV-Tools-diff.a
            ${VULKAN_LIB_DIR}/libSPIRV-Tools-link.a
            ${VULKAN_LIB_DIR}/libSPIRV-Tools-lint.a
            ${VULKAN_LIB_DIR}/libSPIRV-Tools-opt.a
            ${VULKAN_LIB_DIR}/libSPIRV-Tools-reduce.a
            ${VULKAN_LIB_DIR}/libSPIRV-Tools.a
            ${VULKAN_LIB_DIR}/libSPIRV.a
            ${VULKAN_LIB_DIR}/libSPVRemapper.a)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    find_package(Vulkan REQUIRED)
    get_filename_component(VULKAN_LIB_DIR ${Vulkan_LIBRARY} PATH)

    set(LINK_LIBRARIES
            ${VULKAN_LIB_DIR}/glslang$<$<CONFIG:Debug>:d>.lib
            ${VULKAN_LIB_DIR}/spirv-cross-c-shared$<$<CONFIG:Debug>:d>.lib
            ${VULKAN_LIB_DIR}/GenericCodeGen$<$<CONFIG:Debug>:d>.lib
            ${VULKAN_LIB_DIR}/glslang-default-resource-limits$<$<CONFIG:Debug>:d>.lib
            ${VULKAN_LIB_DIR}/MachineIndependent$<$<CONFIG:Debug>:d>.lib
            ${VULKAN_LIB_DIR}/OSDependent$<$<CONFIG:Debug>:d>.lib
            ${VULKAN_LIB_DIR}/SPIRV-Tools-diff$<$<CONFIG:Debug>:d>.lib
            ${VULKAN_LIB_DIR}/SPIRV-Tools-link$<$<CONFIG:Debug>:d>.lib
            ${VULKAN_LIB_DIR}/SPIRV-Tools-lint$<$<CONFIG:Debug>:d>.lib
            ${VULKAN_LIB_DIR}/SPIRV-Tools-opt$<$<CONFIG:Debug>:d>.lib
            ${VULKAN_LIB_DIR}/SPIRV-Tools-reduce$<$<CONFIG:Debug>:d>.lib
            ${VULKAN_LIB_DIR}/SPIRV-Tools$<$<CONFIG:Debug>:d>.lib
            ${VULKAN_LIB_DIR}/SPIRV$<$<CONFIG:Debug>:d>.lib
            ${VULKAN_LIB_DIR}/SPVRemapper$<$<CONFIG:Debug>:d>.lib)
else ()
    find_package(glslang CONFIG REQUIRED)
    find_package(spirv_cross_core CONFIG REQUIRED)
    find_package(spirv_cross_glsl CONFIG REQUIRED)
    find_package(spirv_cross_hlsl CONFIG REQUIRED)
    find_package(spirv_cross_msl CONFIG REQUIRED)
    find_package(spirv_cross_cpp CONFIG REQUIRED)
    find_package(spirv_cross_reflect CONFIG REQUIRED)
    find_package(spirv_cross_c CONFIG REQUIRED)
    set(LINK_LIBRARIES glslang::glslang spirv-cross-c)
endif()


add_executable(
        ProgramTest
        ProgramTest.cpp

        ${MGL_ROOT}/MobileGL/MG_Util/Converters/GLToStr/GLEnumConverter.cpp
#        ${MGL_ROOT}/MobileGL/MG_Util/Pipelines/ShaderCompilationPipeline.cpp
#        ${MGL_ROOT}/MobileGL/MG_Util/Pipelines/ProgramLinkingPipeline.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/GLToGlslang/GLShaderLangConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/ShaderTranspiler/ShaderCompiler.cpp
#        ${MGL_ROOT}/MobileGL/MG_Util/ShaderTranspiler/glslang/UniformTraverser.cpp
)

target_include_directories(ProgramTest PRIVATE
        ${MGL_ROOT}/include
        ${MGL_ROOT}/MobileGL
)

target_link_libraries(
        ProgramTest
        GTest::gtest_main
        ${LINK_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(ProgramTest)