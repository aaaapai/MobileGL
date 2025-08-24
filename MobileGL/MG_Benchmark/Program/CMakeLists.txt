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