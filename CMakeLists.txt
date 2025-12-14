cmake_minimum_required(VERSION 3.22.1)

project("MobileGL")

option(MOBILEGL_FORCE_RELEASE_OPT "Enable Release optimization flags in Debug build" ON)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug" OR MOBILEGL_FORCE_RELEASE_OPT)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT LTOSupported OUTPUT LTOError)
    if (LTOSupported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()

    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT MATCHES "AppleClang")
        add_compile_options(-O3 -ffunction-sections -fdata-sections)
        add_link_options(-Wl,--gc-sections)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
#        add_compile_options(/O2)
    else ()
        add_compile_options(-O2)
    endif()
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/Zc:preprocessor)
endif()

enable_language(CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (MSVC)
    set(CMAKE_CXX_FLAGS "/EHsc ${CMAKE_CXX_FLAGS}")
endif()

# Generate MGGitHash.h
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH_FULL
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(SUBSTRING "${GIT_COMMIT_HASH_FULL}" 0 7 GIT_COMMIT_HASH_SHORT)

configure_file(
    ${CMAKE_SOURCE_DIR}/MobileGL/MG_Util/Miscellany/MGGitHash.h.in
    ${CMAKE_BINARY_DIR}/generated/MGGitHash.h
    @ONLY
)

include_directories(${CMAKE_BINARY_DIR}/generated)

set(ENABLE_RTTI                ON  CACHE BOOL "Enables RTTI (will be took by glslang)" FORCE)

set(ENABLE_GLSLANG_BINARIES    OFF CACHE BOOL "Disable glslangValidator/spirv-remap" FORCE)
set(ENABLE_SPVREMAPPER         OFF CACHE BOOL "Disable SPVRemapper" FORCE)
set(ENABLE_OPT                 OFF CACHE BOOL "Disable SPIRV-Tools opt usage in glslang" FORCE)
set(BUILD_EXTERNAL             OFF CACHE BOOL "Don't build external deps in External/" FORCE)
set(ENABLE_GLSLANG_INSTALL     OFF CACHE BOOL "Don't install glslang targets" FORCE)

set(SPIRV_CROSS_C_API          ON  CACHE BOOL "Enable C API" FORCE)
set(SPIRV_CROSS_ENABLE_GLSL    ON  CACHE BOOL "Enable GLSL backend" FORCE)
set(SPIRV_CROSS_ENABLE_HLSL    OFF CACHE BOOL "Disable HLSL backend" FORCE)
set(SPIRV_CROSS_ENABLE_MSL     OFF CACHE BOOL "Disable MSL backend" FORCE)
set(SPIRV_CROSS_ENABLE_CPP     OFF CACHE BOOL "Disable C++ API target" FORCE)
set(SPIRV_CROSS_CLI            OFF CACHE BOOL "Disable CLI binary" FORCE)
set(SPIRV_CROSS_STATIC         ON  CACHE BOOL "Prefer static libs" FORCE)

# add_subdirectory(3rdparty/DiligentCore)
add_subdirectory(3rdparty/glslang)
add_subdirectory(3rdparty/SPIRV-Cross)

option(TRACY_ENABLE            "Enable Tracy" OFF)

if (TRACY_ENABLE)
    set(TRACY_ON_DEMAND            ON  CACHE BOOL "Enable profiling only connected" FORCE)
    set(TRACY_NO_EXIT              OFF CACHE BOOL "Don't exit Tracy until connected" FORCE)
    set(TRACY_DELAYED_INIT         ON  CACHE BOOL "Don't init Tracy on library load" FORCE)
    set(TRACY_MANUAL_LIFETIME      ON  CACHE BOOL "Manually control Tracy lifetime" FORCE)
    set(TRACY_NO_CRASH_HANDLER     ON  CACHE BOOL "Disable crash handling" FORCE)

    add_subdirectory(3rdparty/tracy)
endif ()

set(SOURCE_FILES
    MobileGL/Init.cpp

    MobileGL/MG_Util/Debug/Log.cpp

    MobileGL/MG_Util/Metrics/TextureMetrics.cpp
    MobileGL/MG_Util/Metrics/BufferMetrics.cpp

    MobileGL/MG_Util/Converters/GLToStr/GLEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToStr/DataTypeConverter.cpp
    MobileGL/MG_Util/Converters/MGToStr/RenderStateEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToStr/GLExtensionConverter.cpp
    MobileGL/MG_Util/Converters/MGToStr/BufferEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToStr/FramebufferEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToStr/TextureEnumConverter.cpp
    MobileGL/MG_Util/Converters/GLToGlslang/ProgramEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToGL/ErrorCodeConverter.cpp
    MobileGL/MG_Util/Converters/MGToGL/BufferEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToGL/FramebufferEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToGL/TextureEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToGL/DataTypeConverter.cpp
    MobileGL/MG_Util/Converters/MGToGL/RenderStateEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToGL/ProgramEnumConverter.cpp
    MobileGL/MG_Util/Converters/GLToMG/BufferEnumConverter.cpp
    MobileGL/MG_Util/Converters/GLToMG/FramebufferEnumConverter.cpp
    MobileGL/MG_Util/Converters/GLToMG/TextureEnumConverter.cpp
    MobileGL/MG_Util/Converters/GLToMG/DataTypeConverter.cpp
    MobileGL/MG_Util/Converters/GLToMG/RenderStateEnumConverter.cpp
    MobileGL/MG_Util/Converters/GLToMG/ProgramEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToMG/TextureEnumConverter.cpp

    MobileGL/MG_Util/Classifiers/TextureEnumClassifier.cpp

    MobileGL/MG_Util/ShaderTranspiler/ShaderCompiler.cpp
    MobileGL/MG_Util/ShaderTranspiler/SpvcSession.cpp
    MobileGL/MG_Util/ShaderTranspiler/ShaderSourceProcessor.cpp
    MobileGL/MG_Util/ShaderTranspiler/glslang/TMglGlslIoResolver.cpp

    MobileGL/MG_Util/BackendLoaders/OpenGL/Loader.cpp

    MobileGL/MG_Util/Texture/PixelStoreProcessor.cpp

    MobileGL/MG_Impl/GLXImpl/Exporting/Definitions.cpp
    MobileGL/MG_Impl/GLXImpl/LookUp/LookUp.cpp

    MobileGL/MG_Impl/EGLImpl/Exporting/Definitions.cpp
    MobileGL/MG_Impl/EGLImpl/Temporary/TemporaryEGLImpl.cpp
    MobileGL/MG_Impl/EGLImpl/EGLWrapper/EGLWrapper.cpp
    
    # @INSERTION_POINT:SOURCE_FILE_GLIMPL@ #
    MobileGL/MG_Impl/GLImpl/Sampler/Validators.cpp
    MobileGL/MG_Impl/GLImpl/Sampler/GL_Sampler.cpp
    MobileGL/MG_Impl/GLImpl/Drawing/GL_Drawing.cpp
    MobileGL/MG_Impl/GLImpl/RenderState/GL_RenderState.cpp
    MobileGL/MG_Impl/GLImpl/Framebuffer/Validators.cpp
    MobileGL/MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.cpp
    MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp
    MobileGL/MG_Impl/GLImpl/Texture/GL_Texture.cpp
    MobileGL/MG_Impl/GLImpl/Texture/Validators.cpp
    MobileGL/MG_Impl/GLImpl/Texture/ProxyTexture.cpp
    MobileGL/MG_Impl/GLImpl/VertexArray/GL_VertexArray.cpp
    MobileGL/MG_Impl/GLImpl/VertexArray/Validators.cpp
    MobileGL/MG_Impl/GLImpl/Buffer/GL_Buffer.cpp
    MobileGL/MG_Impl/GLImpl/Buffer/Validators.cpp
    MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp
    MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp
    MobileGL/MG_Impl/GLImpl/Sync/GL_Sync.cpp

    MobileGL/MG_Impl/Init.cpp

    MobileGL/MG_Backend/Init.cpp

    MobileGL/MG_Backend/DirectGLES/DirectGLES.cpp
    MobileGL/MG_Backend/DirectGLES/Utils.cpp
    MobileGL/MG_Backend/DirectGLES/Managers.cpp

    MobileGL/MG_State/GLState/Core.cpp
    MobileGL/MG_State/GLState/ErrorState/Error.cpp
    MobileGL/MG_State/GLState/BufferState/BufferState.cpp
    MobileGL/MG_State/GLState/BufferState/BufferObject.cpp
    MobileGL/MG_State/GLState/VertexArrayState/VertexArrayState.cpp
    MobileGL/MG_State/GLState/VertexArrayState/VertexArrayObject.cpp
    MobileGL/MG_State/GLState/TextureState/MipmapStorage.cpp
    MobileGL/MG_State/GLState/TextureState/TextureObject.cpp
    MobileGL/MG_State/GLState/TextureState/TextureObject1D.cpp
    MobileGL/MG_State/GLState/TextureState/TextureObject2D.cpp
    MobileGL/MG_State/GLState/TextureState/TextureObject2DCube.cpp
    MobileGL/MG_State/GLState/TextureState/TextureObject3D.cpp
    MobileGL/MG_State/GLState/TextureState/TextureObjectBuffer.cpp
    MobileGL/MG_State/GLState/TextureState/TextureUnit.cpp
    MobileGL/MG_State/GLState/TextureState/TextureState.cpp
    MobileGL/MG_State/GLState/ProgramState/ProgramObject.cpp
    MobileGL/MG_State/GLState/ProgramState/ShaderObject.cpp
    MobileGL/MG_State/GLState/ProgramState/ProgramState.cpp
    MobileGL/MG_State/GLState/RenderState/RenderState.cpp
    MobileGL/MG_State/GLState/FramebufferState/FramebufferObject.cpp
    MobileGL/MG_State/GLState/FramebufferState/FramebufferState.cpp
    MobileGL/MG_State/GLState/SamplerState/SamplerObject.cpp
    MobileGL/MG_State/GLState/SamplerState/SamplerState.cpp
    MobileGL/MG_State/GLState/RenderbufferState/RenderbufferObject.cpp
    MobileGL/MG_State/GLState/RenderbufferState/RenderbufferState.cpp
)

add_library(${CMAKE_PROJECT_NAME} SHARED 
    ${SOURCE_FILES}
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        C_VISIBILITY_PRESET default
        CXX_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
    )
else()
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/MobileGL
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
    # Diligent-Common
    # Diligent-GraphicsEngineOpenGL-shared
    # Diligent-GraphicsEngineVk-shared
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
    glslang::glslang
    spirv-cross-c
)

add_library(${CMAKE_PROJECT_NAME}_s STATIC 
    ${SOURCE_FILES}
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(${CMAKE_PROJECT_NAME}_s PROPERTIES
        C_VISIBILITY_PRESET default
        CXX_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
    )
else()
    set_target_properties(${CMAKE_PROJECT_NAME}_s PROPERTIES
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()

target_include_directories(${CMAKE_PROJECT_NAME}_s PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/MobileGL
)

target_link_libraries(${CMAKE_PROJECT_NAME}_s
    PRIVATE
    # Diligent-Common
    # Diligent-GraphicsEngineOpenGL-shared
    # Diligent-GraphicsEngineVk-shared
)

target_link_libraries(${CMAKE_PROJECT_NAME}_s
    PRIVATE
    glslang::glslang
    spirv-cross-c
)

if (TRACY_ENABLE)
    target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC Tracy::TracyClient)
    target_link_libraries(${CMAKE_PROJECT_NAME}_s PUBLIC Tracy::TracyClient)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC -DTRACY_ENABLE)
    target_compile_definitions(${CMAKE_PROJECT_NAME}_s PUBLIC -DTRACY_ENABLE)
endif ()

if (ANDROID)
    target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
        android
        log
    )
    target_link_libraries(${CMAKE_PROJECT_NAME}_s PUBLIC
        android
        log)
endif()

if (NOT ANDROID)

    set(BUILD_TEST ON CACHE BOOL "Build the tests")
    set(BUILD_BENCHMARK ON CACHE BOOL "Build the benchmarks")

    add_subdirectory(MobileGL/MG_Test)
    add_subdirectory(MobileGL/MG_Benchmark)

endif()
