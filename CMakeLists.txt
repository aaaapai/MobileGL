cmake_minimum_required(VERSION 3.22.1)

project("MobileGL")

enable_language(CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (MSVC)
    set(CMAKE_CXX_FLAGS "/EHsc ${CMAKE_CXX_FLAGS}")
endif()

set(SPIRV_CROSS_STATIC ON CACHE BOOL "Build SPIRV-Cross as static library")
set(SPIRV_CROSS_CLI OFF CACHE BOOL "Build SPIRV-Cross command line tool")

# add_subdirectory(3rdparty/DiligentCore)
add_subdirectory(3rdparty/glslang)
add_subdirectory(3rdparty/SPIRV-Cross)

set(SOURCE_FILES
    MobileGL/Init.cpp

    MobileGL/MG_Util/Debug/Log.cpp

    MobileGL/MG_Util/Metrics/TextureMetrics.cpp
    MobileGL/MG_Util/Metrics/BufferMetrics.cpp

    MobileGL/MG_Util/Converters/GLToStr/GLEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToStr/DataTypeConverter.cpp
    MobileGL/MG_Util/Converters/MGToStr/GLExtensionConverter.cpp
    MobileGL/MG_Util/Converters/MGToStr/BufferEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToStr/TextureEnumConverter.cpp
    MobileGL/MG_Util/Converters/GLToGlslang/GLShaderLangConverter.cpp
    MobileGL/MG_Util/Converters/MGToGL/ErrorCodeConverter.cpp
    MobileGL/MG_Util/Converters/MGToGL/BufferEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToGL/TextureEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToGL/DataTypeConverter.cpp
    MobileGL/MG_Util/Converters/GLToMG/BufferEnumConverter.cpp
    MobileGL/MG_Util/Converters/GLToMG/TextureEnumConverter.cpp
    MobileGL/MG_Util/Converters/GLToMG/DataTypeConverter.cpp
    MobileGL/MG_Util/Converters/MGToMG/TextureEnumConverter.cpp

    MobileGL/MG_Util/ShaderTranspiler/ShaderCompiler.cpp
    MobileGL/MG_Util/ShaderTranspiler/SpvcSession.cpp

    MobileGL/MG_Impl/GLXImpl/Exporting/Definitions.cpp
    MobileGL/MG_Impl/GLXImpl/LookUp/LookUp.cpp

    MobileGL/MG_Impl/EGLImpl/Exporting/Definitions.cpp
    MobileGL/MG_Impl/EGLImpl/Temporary/TemporaryEGLImpl.cpp
    
    # @INSERTION_POINT:SOURCE_FILE_GLIMPL@ #
    MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp
    MobileGL/MG_Impl/GLImpl/Texture/GL_Texture.cpp
    MobileGL/MG_Impl/GLImpl/Texture/Validators.cpp
    MobileGL/MG_Impl/GLImpl/VertexArray/GL_VertexArray.cpp
    MobileGL/MG_Impl/GLImpl/VertexArray/Validators.cpp
    MobileGL/MG_Impl/GLImpl/Buffer/GL_Buffer.cpp
    MobileGL/MG_Impl/GLImpl/Buffer/Validators.cpp
    MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp
    MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp

    MobileGL/MG_Backend/Init.cpp

    MobileGL/MG_State/GLState/Core.cpp
    MobileGL/MG_State/GLState/ErrorState/Error.cpp
    MobileGL/MG_State/GLState/BufferState/BufferState.cpp
    MobileGL/MG_State/GLState/BufferState/BufferObject.cpp
    MobileGL/MG_State/GLState/VertexArrayState/VertexArrayState.cpp
    MobileGL/MG_State/GLState/VertexArrayState/VertexArrayObject.cpp
    MobileGL/MG_State/GLState/TextureState/TextureObject.cpp
    MobileGL/MG_State/GLState/TextureState/TextureUnit.cpp
    MobileGL/MG_State/GLState/TextureState/TextureState.cpp
    MobileGL/MG_State/GLState/ProgramState/ProgramObject.cpp
    MobileGL/MG_State/GLState/ProgramState/ShaderObject.cpp
    MobileGL/MG_State/GLState/ProgramState/ProgramState.cpp
)

add_library(${CMAKE_PROJECT_NAME} SHARED 
    ${SOURCE_FILES}
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

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

set_target_properties(${CMAKE_PROJECT_NAME}_s PROPERTIES
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

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

if (ANDROID)
    target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
        android
        log
    )
    target_link_libraries(${CMAKE_PROJECT_NAME}_s PUBLIC
        android
        log)
endif()

set(BUILD_TEST ON CACHE BOOL "Build the tests")
set(BUILD_BENCHMARK ON CACHE BOOL "Build the benchmarks")

add_subdirectory(MobileGL/MG_Test)
add_subdirectory(MobileGL/MG_Benchmark)
