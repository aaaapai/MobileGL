cmake_minimum_required(VERSION 3.22.1)

project("MobileGL")

enable_language(CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_library(GLSLANG_LIB glslang PATHS ${CMAKE_SOURCE_DIR}/libraries/arm64-v8a/)

set(SOURCE_FILES
    MobileGL/Init.cpp

    MobileGL/MG_Util/Debug/Log.cpp

    MobileGL/MG_Util/Converters/GLToStr/GLEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToStr/DataTypeConverter.cpp
    MobileGL/MG_Util/Converters/MGToStr/GLExtensionConverter.cpp
    MobileGL/MG_Util/Converters/MGToStr/BufferEnumConverter.cpp
    MobileGL/MG_Util/Converters/GLToGlslang/GLShaderLangConverter.cpp
    MobileGL/MG_Util/Converters/MGToGL/ErrorCodeConverter.cpp
    MobileGL/MG_Util/Converters/MGToGL/BufferEnumConverter.cpp
    MobileGL/MG_Util/Converters/MGToGL/DataTypeConverter.cpp
    MobileGL/MG_Util/Converters/GLToMG/BufferEnumConverter.cpp
    MobileGL/MG_Util/Converters/GLToMG/DataTypeConverter.cpp

    MobileGL/MG_Impl/GLXImpl/Exporting/Definitions.cpp
    MobileGL/MG_Impl/GLXImpl/LookUp/LookUp.cpp

    MobileGL/MG_Impl/EGLImpl/Exporting/Definitions.cpp
    MobileGL/MG_Impl/EGLImpl/Temporary/TemporaryEGLImpl.cpp
    
    # @INSERTION_POINT:SOURCE_FILE_GLIMPL@ #
    MobileGL/MG_Impl/GLImpl/Texture/GL_Texture.cpp
    MobileGL/MG_Impl/GLImpl/VertexArray/Validators.cpp
    MobileGL/MG_Impl/GLImpl/VertexArray/GL_VertexArray.cpp
    MobileGL/MG_Impl/GLImpl/Exporting/Definitions.cpp
    MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp
    MobileGL/MG_Impl/GLImpl/Buffer/GL_Buffer.cpp
    MobileGL/MG_Impl/GLImpl/Buffer/Validators.cpp

    MobileGL/MG_Backend/Init.cpp

    MobileGL/MG_Util/ShaderTranspiler/ShaderCompiler.cpp

    MobileGL/MG_State/GLState/Core.cpp
    MobileGL/MG_State/GLState/ErrorState/Error.cpp
    MobileGL/MG_State/GLState/BufferState/BufferState.cpp
    MobileGL/MG_State/GLState/BufferState/BufferObject.cpp
    MobileGL/MG_State/GLState/VertexArrayState/VertexArrayState.cpp
    MobileGL/MG_State/GLState/VertexArrayState/VertexArrayObject.cpp
    MobileGL/MG_State/GLState/TextureState/TextureObject.cpp
    MobileGL/MG_State/GLState/TextureState/TextureUnit.cpp
    MobileGL/MG_State/GLState/TextureState/TextureState.cpp
)

add_library(${CMAKE_PROJECT_NAME} SHARED 
    ${SOURCE_FILES}
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

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/MobileGL
)

target_link_libraries(MobileGL
    PRIVATE
    # Diligent-Common
    # Diligent-GraphicsEngineOpenGL-shared
    # Diligent-GraphicsEngineVk-shared
)
