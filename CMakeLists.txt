cmake_minimum_required(VERSION 3.22.1)

project("MobileGL")

enable_language(CXX)


set(CAMKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -pipe -integrated-as -mllvm -polly -mllvm -polly-vectorizer=stripmine -mllvm -polly-invariant-load-hoisting -mllvm -polly-run-inliner -mllvm -polly-run-dce -mllvm -polly-invariant-load-hoisting -flto=thin -mllvm -polly-run-inliner -mllvm -polly-run-dce -mllvm -polly-parallel -mllvm -polly-scheduling=dynamic -flto=thin -fno-emulated-tls -fwhole-program-vtables -fdata-sections -ffunction-sections -fmerge-all-constants -mllvm -polly-detect-keep-going -mllvm -polly-ast-use-context -march=armv8-a+simd -fvisibility=hidden -Wall -Wextra -fPIC -DPIC -ferror-limit=0 -Wno-return-type -Wno-unused-parameter -Wno-c++98-compat -Wno-c++98-compat-pedantic -DGLM_FORCE_INTRINSICS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pipe -integrated-as -mllvm -polly -mllvm -polly-vectorizer=stripmine -mllvm -polly-invariant-load-hoisting -mllvm -polly-run-inliner -mllvm -polly-run-dce -mllvm -polly-invariant-load-hoisting -flto=thin -mllvm -polly-run-inliner -mllvm -polly-run-dce -mllvm -polly-parallel -mllvm -polly-scheduling=dynamic -flto=thin -fno-emulated-tls -fwhole-program-vtables -fdata-sections -ffunction-sections -fmerge-all-constants -mllvm -polly-detect-keep-going -mllvm -polly-ast-use-context -march=armv8-a+simd -fPIC -DPIC -Wall -Wextra -ferror-limit=0 -Wno-return-type -Wno-unused-parameter -Wno-c++98-compat -Wno-c++98-compat-pedantic -DGLM_FORCE_INTRINSICS")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -flto=thin -Wl,-plugin-opt=-emulated-tls=0")

set(CMAKE_ANDROID_STL_TYPE system)

set(CMAKE_BUILD_TYPE Release)

set(PROFILING OFF)

find_library(GLSLANG_LIB glslang PATHS ${CMAKE_SOURCE_DIR}/libraries/arm64-v8a/)

add_library(${CMAKE_PROJECT_NAME} SHARED 
        MG/Init.cpp

        # MG_GL/Implementations/GL
        MG/MG_GL/Implementations/GL/GLFuncsDefinitions/GLFuncsDefinitions.cpp
        
        MG/MG_GL/Implementations/GL/Getter/GL_Getter.cpp
        MG/MG_GL/Implementations/GL/Getter/BackendInfo.cpp

        MG/MG_GL/Implementations/GL/Framebuffer/GL_Framebuffer.cpp

        MG/MG_GL/Implementations/GL/Common/GL_Common.cpp

        MG/MG_GL/Implementations/GL/Buffer/GL_Buffer.cpp

        MG/MG_GL/Implementations/GL/Drawing/GL_Drawing.cpp

        MG/MG_GL/Implementations/GL/Program/GL_Program.cpp

        MG/MG_GL/Implementations/GL/Texture/GL_Texture.cpp

        MG/MG_GL/Implementations/GL/VertexArray/GL_VertexArray.cpp

        # MG_GL/Implementations/GLX
        MG/MG_GL/Implementations/GLX/LookUp/LookUp.cpp
        
        # MG_GL/Implementations/EGL
        MG/MG_GL/Implementations/EGL/VK/EGL_EMU.cpp

        MG/MG_GL/Implementations/EGL/GLES/EGL_WRAP.cpp
        
        # MG_GL/State
        MG/MG_GL/State/Core/GLState.cpp
        
        MG/MG_GL/State/Common/CommonState.cpp

        MG/MG_GL/State/Texture/TextureState.cpp
        
        MG/MG_GL/State/VertexArray/VertexArrayState.cpp
        
        MG/MG_GL/State/Buffer/BufferState.cpp

        MG/MG_GL/State/Program/ShaderObject.cpp
        MG/MG_GL/State/Program/ProgramState.cpp
        
        MG/MG_GL/State/Framebuffer/FramebufferState.cpp

        # MG_GL/BackendLoader
        MG/MG_GL/BackendLoader/GLES/Loader.cpp
        
        # MG_UTIL/Debug
        MG/MG_UTIL/Debug/Debug.cpp
        
        # MG_UTIL/Program
        MG/MG_UTIL/Program/DebugTool.cpp
        MG/MG_UTIL/Program/GLSLTool.cpp
        
        # MG_RHI/GLES
        MG/MG_RHI/GLES/Test/TestDrawing.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ./includes)

target_link_libraries(${CMAKE_PROJECT_NAME}
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libglslang.a
        ${CMAKE_SOURCE_DIR}/library/arm64-v8a/libspirv-cross-c-shared.so
        #${CMAKE_SOURCE_DIR}/library/arm64-v8a/libshaderconv.so
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

target_include_directories(MobileGL PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        )


if (PROFILING)
        find_package(Threads)
        include_directories(${CMAKE_SOURCE_DIR}/3rdparty/perfetto/sdk)
        add_library(perfetto STATIC ${CMAKE_SOURCE_DIR}/3rdparty/perfetto/sdk/perfetto.cc)
        target_link_libraries(MobileGL perfetto ${CMAKE_THREAD_LIBS_INIT})
        target_compile_definitions(MobileGL PUBLIC PROFILING=1)
endif()
