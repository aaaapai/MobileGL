cmake_minimum_required(VERSION 3.14)
project(MobileGLTest)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(MGL_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

add_executable(
  SanityTest
  SanityTest.cpp
)
target_link_libraries(
  SanityTest
  GTest::gtest_main
)

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

include(GoogleTest)
gtest_discover_tests(SanityTest)

add_subdirectory(Buffer)
add_subdirectory(VertexArray)
add_subdirectory(Program)
