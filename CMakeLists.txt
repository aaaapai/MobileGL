cmake_minimum_required(VERSION 3.22.1)

project("MobileGL")

enable_language(CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -fvisibility=hidden -funwind-tables -g -D_THREAD_SAFE -fPIC -stdlib=libc++")

set(PROFILING OFF)

find_library(GLSLANG_LIB glslang PATHS ${CMAKE_SOURCE_DIR}/libraries/arm64-v8a/)

add_library(${CMAKE_PROJECT_NAME} SHARED 
    MobileGL/Init.cpp

    MobileGL/MG_Util/Debug/Log.cpp

    MobileGL/MG_Util/Converters/GLToStr/GLEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToStr/GLExtensionConverter.cpp

    MobileGL/MG_Impl/GLXImpl/Exporting/Definitions.cpp
    MobileGL/MG_Impl/GLXImpl/LookUp/LookUp.cpp

    MobileGL/MG_Impl/EGLImpl/Exporting/Definitions.cpp
    MobileGL/MG_Impl/EGLImpl/Temporary/TemporaryEGLImpl.cpp
    
    MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp
    MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp

    MobileGL/MG_Backend/Init.cpp 

    MobileGL/MG_Util/Pipelines/Shader/GLSLtoSPIRVPipeline.cpp
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