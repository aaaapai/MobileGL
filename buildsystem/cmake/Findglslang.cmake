# Findglslang.cmake
# ------------------
#
# Find glslang library
#
# This module defines the following variables:
#   glslang_FOUND - True if glslang library is found
#   glslang_INCLUDE_DIRS - Include directories for glslang
#   glslang_LIBRARIES - Libraries for glslang
#
# This module reads hints about search locations from:
#   glslang_ROOT - Preferred installation prefix

# First try to find glslang using CONFIG mode (for vcpkg, conan, etc.)
if (NOT glslang_FOUND)
    find_package(glslang CONFIG QUIET)
    if (glslang_FOUND)
        message(STATUS "Found glslang via CONFIG mode: ${glslang_DIR}")
        set(glslang_LIBRARY glslang::glslang)
    endif()
endif()

# If CONFIG mode failed, try to find Vulkan SDK
if (NOT glslang_FOUND)
    find_package(Vulkan QUIET)
    if (Vulkan_FOUND)
        get_filename_component(VULKAN_SDK_DIR ${Vulkan_INCLUDE_DIRS} DIRECTORY)
        message(STATUS "Found Vulkan SDK: ${VULKAN_SDK_DIR}")
        
        # Try to find headers in Vulkan SDK
        find_path(glslang_INCLUDE_DIR 
            NAMES glslang/Public/ShaderLang.h
            HINTS ${VULKAN_SDK_DIR}
            PATH_SUFFIXES include
            NO_DEFAULT_PATH
        )
        
        # Try to find libraries in Vulkan SDK (separate debug/release searches)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            find_library(glslang_LIBRARY
                NAMES glslangd libglslangd
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(glslang_OSDependent_LIBRARY
                NAMES OSDependentd libOSDependentd
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(glslang_MachineIndependent_LIBRARY
                NAMES MachineIndependentd libMachineIndependentd
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(glslang_GenericCodeGen_LIBRARY
                NAMES GenericCodeGend libGenericCodeGend
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(glslang_SPIRV_LIBRARY
                NAMES SPIRVd libSPIRVd
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(glslang_glslang_default_resource_limits_LIBRARY
                NAMES glslang-default-resource-limitsd libglslang-default-resource-limitsd
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
        else()
            find_library(glslang_LIBRARY
                NAMES glslang libglslang
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(glslang_OSDependent_LIBRARY
                NAMES OSDependent libOSDependent
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(glslang_MachineIndependent_LIBRARY
                NAMES MachineIndependent libMachineIndependent
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(glslang_GenericCodeGen_LIBRARY
                NAMES GenericCodeGen libGenericCodeGen
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(glslang_SPIRV_LIBRARY
                NAMES SPIRV libSPIRV
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(glslang_glslang_default_resource_limits_LIBRARY
                NAMES glslang-default-resource-limits libglslang-default-resource-limits
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
        endif()
    endif()
endif()

# If Vulkan SDK search failed, try standard search
if (NOT glslang_FOUND)
    # Look for the header file
    find_path(glslang_INCLUDE_DIR 
        NAMES glslang/Public/ShaderLang.h
        HINTS ${glslang_ROOT} ENV glslang_ROOT
        PATH_SUFFIXES include
    )

    # Look for the libraries (separate debug/release searches)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        find_library(glslang_LIBRARY
            NAMES glslangd libglslangd
            HINTS ${glslang_ROOT} ENV glslang_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(glslang_OSDependent_LIBRARY
            NAMES OSDependentd libOSDependentd
            HINTS ${glslang_ROOT} ENV glslang_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(glslang_MachineIndependent_LIBRARY
            NAMES MachineIndependentd libMachineIndependentd
            HINTS ${glslang_ROOT} ENV glslang_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(glslang_GenericCodeGen_LIBRARY
            NAMES GenericCodeGend libGenericCodeGend
            HINTS ${glslang_ROOT} ENV glslang_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(glslang_SPIRV_LIBRARY
            NAMES SPIRVd libSPIRVd
            HINTS ${glslang_ROOT} ENV glslang_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(glslang_glslang_default_resource_limits_LIBRARY
            NAMES glslang-default-resource-limitsd libglslang-default-resource-limitsd
            HINTS ${glslang_ROOT} ENV glslang_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )
    else()
        find_library(glslang_LIBRARY
            NAMES glslang libglslang
            HINTS ${glslang_ROOT} ENV glslang_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(glslang_OSDependent_LIBRARY
            NAMES OSDependent libOSDependent
            HINTS ${glslang_ROOT} ENV glslang_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(glslang_MachineIndependent_LIBRARY
            NAMES MachineIndependent libMachineIndependent
            HINTS ${glslang_ROOT} ENV glslang_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(glslang_GenericCodeGen_LIBRARY
            NAMES GenericCodeGen libGenericCodeGen
            HINTS ${glslang_ROOT} ENV glslang_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(glslang_SPIRV_LIBRARY
            NAMES SPIRV libSPIRV
            HINTS ${glslang_ROOT} ENV glslang_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(glslang_glslang_default_resource_limits_LIBRARY
            NAMES glslang-default-resource-limits libglslang-default-resource-limits
            HINTS ${glslang_ROOT} ENV glslang_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )
    endif()
endif()

# Set the include directory and libraries
set(glslang_INCLUDE_DIRS ${glslang_INCLUDE_DIR})
set(glslang_LIBRARIES 
    ${glslang_LIBRARY}
    ${glslang_OSDependent_LIBRARY}
    ${glslang_MachineIndependent_LIBRARY}
    ${glslang_GenericCodeGen_LIBRARY}
    ${glslang_SPIRV_LIBRARY}
    ${glslang_glslang_default_resource_limits_LIBRARY}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glslang 
    REQUIRED_VARS glslang_LIBRARIES
)

mark_as_advanced(
    glslang_INCLUDE_DIR
    glslang_LIBRARY
    glslang_OSDependent_LIBRARY
    glslang_MachineIndependent_LIBRARY
    glslang_GenericCodeGen_LIBRARY
    glslang_SPIRV_LIBRARY
    glslang_glslang_default_resource_limits_LIBRARY
)