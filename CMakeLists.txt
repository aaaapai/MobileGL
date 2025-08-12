cmake_minimum_required(VERSION 3.22.1)

project("MobileGL")

enable_language(CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_FILES
    MobileGL/Init.cpp

    MobileGL/MG_Util/Debug/Log.cpp

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

if(MACOS)
    # iOS
    set(iOS_LIB_DIR ${CMAKE_SOURCE_DIR}/library/ios)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        ${iOS_LIB_DIR}/libglslang.a
        ${iOS_LIB_DIR}/libspirv-cross-c-shared.dylib
        ${iOS_LIB_DIR}/libGenericCodeGen.a
        ${iOS_LIB_DIR}/libglslang-default-resource-limits.a
        ${iOS_LIB_DIR}/libMachineIndependent.a
        ${iOS_LIB_DIR}/libOSDependent.a
        ${iOS_LIB_DIR}/libSPIRV-Tools-diff.a
        ${iOS_LIB_DIR}/libSPIRV-Tools-link.aBuffer
        ${iOS_LIB_DIR}/libSPIRV-Tools-lint.a
        ${iOS_LIB_DIR}/libSPIRV-Tools-opt.a
        ${iOS_LIB_DIR}/libSPIRV-Tools-reduce.a
        ${iOS_LIB_DIR}/libSPIRV-Tools.a
        ${iOS_LIB_DIR}/libSPIRV.a
        ${iOS_LIB_DIR}/libSPVRemapper.a
        "-F${CMAKE_SOURCE_DIR}/external"
        "-framework libEGL"
        "-framework libGLESv2"
        "-framework Metal"
        "-framework Foundation"
        "-framework QuartzCore"
    )
else()
if(LINUX)
    # Linux
    # TODO: support more architectures.
    find_library(GLSLANG_LIB glslang PATHS ${CMAKE_SOURCE_DIR}/library/linux/x86_64)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        ${CMAKE_SOURCE_DIR}/library/linux/x86_64/libglslang.a
        ${CMAKE_SOURCE_DIR}/library/linux/x86_64/libspirv-cross-c-shared.so
        ${CMAKE_SOURCE_DIR}/library/linux/x86_64/libGenericCodeGen.a
        ${CMAKE_SOURCE_DIR}/library/linux/x86_64/libglslang-default-resource-limits.a
        ${CMAKE_SOURCE_DIR}/library/linux/x86_64/libMachineIndependent.a
        ${CMAKE_SOURCE_DIR}/library/linux/x86_64/libOSDependent.a
        ${CMAKE_SOURCE_DIR}/library/linux/x86_64/libSPIRV-Tools-diff.a
        ${CMAKE_SOURCE_DIR}/library/linux/x86_64/libSPIRV-Tools-link.a
        ${CMAKE_SOURCE_DIR}/library/linux/x86_64/libSPIRV-Tools-lint.a
        ${CMAKE_SOURCE_DIR}/library/linux/x86_64/libSPIRV-Tools-opt.a
        ${CMAKE_SOURCE_DIR}/library/linux/x86_64/libSPIRV-Tools-reduce.a
        ${CMAKE_SOURCE_DIR}/library/linux/x86_64/libSPIRV-Tools.a
        ${CMAKE_SOURCE_DIR}/library/linux/x86_64/libSPIRV.a
        ${CMAKE_SOURCE_DIR}/library/linux/x86_64/libSPVRemapper.a
    )
else()
    # Android
    find_library(GLSLANG_LIB glslang PATHS ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libglslang.a
        ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libspirv-cross-c-shared.so
        ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libGenericCodeGen.a
        ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libglslang-default-resource-limits.a
        ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libMachineIndependent.a
        ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libOSDependent.a
        ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libSPIRV-Tools-diff.a
        ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libSPIRV-Tools-link.a
        ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libSPIRV-Tools-lint.a
        ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libSPIRV-Tools-opt.a
        ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libSPIRV-Tools-reduce.a
        ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libSPIRV-Tools.a
        ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libSPIRV.a
        ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libSPVRemapper.a
        android
        log
    )
endif()
endif()

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
