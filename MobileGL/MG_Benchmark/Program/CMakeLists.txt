cmake_minimum_required(VERSION 3.24)

# Add the directory containing our custom Find modules
list(APPEND CMAKE_MODULE_PATH "${MGL_ROOT}/buildsystem/cmake")

# Find the required libraries using our custom Find modules
find_package(glslang REQUIRED)
find_package(SPIRVCross REQUIRED)
find_package(SPIRVTools)

message(STATUS "glslang_LIBRARIES: ${glslang_LIBRARIES}")
message(STATUS "SPIRVTools_LIBRARIES: ${SPIRVTools_LIBRARIES}")
message(STATUS "SPIRVCross_LIBRARIES: ${SPIRVCross_LIBRARIES}")

# Set the libraries to link against
set(LINK_LIBRARIES 
    ${glslang_LIBRARIES}
    ${SPIRVCross_LIBRARIES}
)

if (SPIRVTools_FOUND)
  list(APPEND LINK_LIBRARIES ${SPIRVTools_LIBRARIES})
endif()

add_executable(
        ProgramBench
        ProgramBench.cpp
	
	${MGL_ROOT}/MobileGL/MG_State/GLState/ProgramState/ProgramObject.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/ProgramState/ShaderObject.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/ProgramState/ProgramState.cpp
        ${MGL_ROOT}/MobileGL/MG_Impl/GLImpl/Program/GL_Program.cpp

        ${MGL_ROOT}/MobileGL/MG_Util/Converters/GLToStr/GLEnumConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/GLToGlslang/GLShaderLangConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/ShaderTranspiler/ShaderCompiler.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/ShaderTranspiler/SpvcSession.cpp

        ${MGL_ROOT}/MobileGL/MG_Util/ShaderTranspiler/glslang/UniformTraverser.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/SPIRVCrossToGL/SpvcTypeConverter.cpp

        ${MGL_ROOT}/MobileGL/MG_State/GLState/Core.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/BufferState/BufferState.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/BufferState/BufferObject.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/VertexArrayState/VertexArrayObject.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/VertexArrayState/VertexArrayState.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/TextureState/TextureObject.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/TextureState/TextureUnit.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/TextureState/TextureState.cpp
        ${MGL_ROOT}/MobileGL/MG_State/GLState/ErrorState/Error.cpp
        ${MGL_ROOT}/MobileGL/MG_Impl/GLImpl/Buffer/GL_Buffer.cpp
        ${MGL_ROOT}/MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.cpp
        ${MGL_ROOT}/MobileGL/MG_Impl/GLImpl/Buffer/Validators.cpp
        ${MGL_ROOT}/MobileGL/MG_Impl/GLImpl/VertexArray/Validators.cpp
        ${MGL_ROOT}/MobileGL/MG_Impl/GLImpl/VertexArray/GL_VertexArray.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/GLToStr/GLEnumConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/MGToStr/GLExtensionConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/MGToStr/BufferEnumConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/MGToStr/DataTypeConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/GLToGlslang/GLShaderLangConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/MGToGL/ErrorCodeConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/MGToGL/BufferEnumConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/MGToGL/DataTypeConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/GLToMG/BufferEnumConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Converters/GLToMG/DataTypeConverter.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Metrics/BufferMetrics.cpp
        ${MGL_ROOT}/MobileGL/MG_Backend/Init.cpp
        ${MGL_ROOT}/MobileGL/MG_Util/Debug/Log.cpp
)

target_include_directories(ProgramBench PRIVATE
        ${MGL_ROOT}/include
        ${MGL_ROOT}/MobileGL
)

target_link_libraries(
        ProgramBench
        benchmark::benchmark
        ${LINK_LIBRARIES}
)

add_test(NAME ProgramBench COMMAND ProgramBench --benchmark_counters_tabular=true)