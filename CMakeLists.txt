cmake_minimum_required(VERSION 3.22.1)

project("MobileGL")

enable_language(CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -pipe -integrated-as -mllvm -polly -mllvm -polly-vectorizer=stripmine -mllvm -polly-invariant-load-hoisting -mllvm -polly-run-inliner -mllvm -polly-run-dce -mllvm -polly-invariant-load-hoisting -flto=thin -mllvm -polly-run-inliner -mllvm -polly-run-dce -mllvm -polly-parallel -mllvm -polly-scheduling=dynamic -fno-emulated-tls -fwhole-program-vtables -fdata-sections -ffunction-sections -fmerge-all-constants -mllvm -polly-detect-keep-going -mllvm -polly-ast-use-context -march=armv8-a+simd -fvisibility=hidden -Wall -Wextra -ferror-limit=0 -Wno-return-type -Wno-unused-parameter -Wno-c++98-compat -Wno-c++98-compat-pedantic -DGLM_FORCE_INTRINSICS -Wno-extra-qualification -D_THREAD_SAFE -fvisibility=hidden -Wno-sign-compare")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pipe -integrated-as -mllvm -polly -mllvm -polly-vectorizer=stripmine -mllvm -polly-invariant-load-hoisting -mllvm -polly-run-inliner -mllvm -polly-run-dce -mllvm -polly-invariant-load-hoisting -flto=thin -mllvm -polly-run-inliner -mllvm -polly-run-dce -mllvm -polly-parallel -mllvm -polly-scheduling=dynamic -fno-emulated-tls -fwhole-program-vtables -fdata-sections -ffunction-sections -fmerge-all-constants -mllvm -polly-detect-keep-going -mllvm -polly-ast-use-context -march=armv8-a+simd -Wall -Wextra -ferror-limit=0 -Wno-return-type -Wno-unused-parameter -Wno-c++98-compat -Wno-c++98-compat-pedantic -DGLM_FORCE_INTRINSICS -fexceptions -fno-rtti -Wno-extra-qualification -D_THREAD_SAFE -stdlib=libc++ -fvisibility=hidden -Wno-sign-compare")
set(CMAKE_ANDROID_STL_TYPE c++_static)
#set(CMAKE_BUILD_TYPE Release)

set(PROFILING OFF)

find_library(GLSLANG_LIB glslang PATHS ${CMAKE_SOURCE_DIR}/libraries/arm64-v8a/)

add_library(${CMAKE_PROJECT_NAME} SHARED 
        
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libglslang.a
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libspirv-cross-c-shared.so
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libGenericCodeGen.a
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libglslang-default-resource-limits.a
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libMachineIndependent.a
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libOSDependent.a
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libSPIRV-Tools-diff.a
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libSPIRV-Tools-link.a
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libSPIRV-Tools-lint.a
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libSPIRV-Tools-opt.a
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libSPIRV-Tools-reduce.a
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libSPIRV-Tools.a
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libSPIRV.a
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libSPVRemapper.a
        android
        log
        EGL
        vulkan
)

# add_subdirectory(3rdparty/DiligentCore)

target_include_directories(MobileGL PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(MobileGL
        PRIVATE
        # Diligent-Common
        # Diligent-GraphicsEngineOpenGL-shared
        # Diligent-GraphicsEngineVk-shared
)
