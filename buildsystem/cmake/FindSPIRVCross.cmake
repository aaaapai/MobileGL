# FindSPIRVCross.cmake
# --------------------
#
# Find SPIRV-Cross library
#
# This module defines the following variables:
#   SPIRVCross_FOUND - True if SPIRV-Cross library is found
#   SPIRVCross_INCLUDE_DIRS - Include directories for SPIRV-Cross
#   SPIRVCross_LIBRARIES - Libraries for SPIRV-Cross
#
# This module reads hints about search locations from:
#   SPIRVCross_ROOT - Preferred installation prefix

# First try to find SPIRV-Cross using CONFIG mode (for vcpkg, conan, etc.)
if (NOT SPIRVCross_FOUND)
    find_package(spirv_cross_core CONFIG QUIET)
    if (spirv_cross_core_FOUND)
        message(STATUS "Found SPIRV-Cross via CONFIG mode")
        # Try to find all components
        find_package(spirv_cross_glsl CONFIG QUIET)
        find_package(spirv_cross_hlsl CONFIG QUIET)
        find_package(spirv_cross_msl CONFIG QUIET)
        find_package(spirv_cross_cpp CONFIG QUIET)
        find_package(spirv_cross_reflect CONFIG QUIET)
        find_package(spirv_cross_c CONFIG QUIET)
        
        # Set the libraries (using the CONFIG mode variables)
        set(SPIRVCross_LIBRARIES 
            spirv-cross-core
            spirv-cross-glsl
            spirv-cross-hlsl
            spirv-cross-msl
            spirv-cross-cpp
            spirv-cross-reflect
            spirv-cross-c
        )
        
        # Set the include directories
        set(SPIRVCross_INCLUDE_DIRS ${spirv_cross_core_INCLUDE_DIRS})
        
        # Mark as found
        set(SPIRVCross_FOUND TRUE)
    endif()
endif()

# If CONFIG mode failed, try to find Vulkan SDK
if (NOT SPIRVCross_FOUND)
    find_package(Vulkan QUIET)
    if (Vulkan_FOUND)
        get_filename_component(VULKAN_SDK_DIR ${Vulkan_INCLUDE_DIRS} DIRECTORY)
        message(STATUS "Found Vulkan SDK: ${VULKAN_SDK_DIR}")
        
        # Try to find headers in Vulkan SDK
        find_path(SPIRVCross_INCLUDE_DIR 
            NAMES spirv_cross/spirv_cross.hpp
            HINTS ${VULKAN_SDK_DIR}
            PATH_SUFFIXES include
            NO_DEFAULT_PATH
        )
        
        # Try to find libraries in Vulkan SDK (separate debug/release searches)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            find_library(SPIRVCross_c_LIBRARY
                NAMES spirv-cross-cd libspirv-cross-cd spirv-cross-c-sharedd libspirv-cross-c-sharedd
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(SPIRVCross_cpp_LIBRARY
                NAMES spirv-cross-cppd libspirv-cross-cppd spirv-cross-cpp-sharedd libspirv-cross-cpp-sharedd
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )

            find_library(SPIRVCross_core_LIBRARY
                NAMES spirv-cross-cored libspirv-cross-cored
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )

            find_library(SPIRVCross_reflect_LIBRARY
                NAMES spirv-cross-reflectd libspirv-cross-reflectd
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )

            find_library(SPIRVCross_glsl_LIBRARY
                NAMES spirv-cross-glsld libspirv-cross-glsld
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )

            find_library(SPIRVCross_hlsl_LIBRARY
                NAMES spirv-cross-hlsld libspirv-cross-hlsld
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )

            find_library(SPIRVCross_msl_LIBRARY
                NAMES spirv-cross-msld libspirv-cross-msld
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
        else()
            find_library(SPIRVCross_c_LIBRARY
                NAMES spirv-cross-c libspirv-cross-c spirv-cross-c-shared libspirv-cross-c-shared
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )

            find_library(SPIRVCross_cpp_LIBRARY
                NAMES spirv-cross-cpp libspirv-cross-cpp spirv-cross-cpp-shared libspirv-cross-cpp-shared
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(SPIRVCross_core_LIBRARY
                NAMES spirv-cross-core libspirv-cross-core
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(SPIRVCross_reflect_LIBRARY
                NAMES spirv-cross-reflect libspirv-cross-reflect
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )

            find_library(SPIRVCross_glsl_LIBRARY
                NAMES spirv-cross-glsl libspirv-cross-glsl
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )

            find_library(SPIRVCross_hlsl_LIBRARY
                NAMES spirv-cross-hlsl libspirv-cross-hlsl
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(SPIRVCross_msl_LIBRARY
                NAMES spirv-cross-msl libspirv-cross-msl
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
        endif()
    endif()
endif()

# If Vulkan SDK search failed, try standard search
if (NOT SPIRVCross_FOUND)
    # Look for the header file
    find_path(SPIRVCross_INCLUDE_DIR 
        NAMES spirv_cross/spirv_cross.hpp
        HINTS ${SPIRVCross_ROOT} ENV SPIRVCross_ROOT
        PATH_SUFFIXES include
    )

    # Look for the libraries (separate debug/release searches)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        find_library(SPIRVCross_c_LIBRARY
            NAMES spirv-cross-cd libspirv-cross-cd spirv-cross-c-sharedd libspirv-cross-c-sharedd
            HINTS ${SPIRVCross_ROOT} ENV SPIRVCross_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )
        
        find_library(SPIRVCross_cpp_LIBRARY
            NAMES spirv-cross-cppd libspirv-cross-cppd spirv-cross-cpp-sharedd libspirv-cross-cpp-sharedd
            HINTS ${SPIRVCross_ROOT} ENV SPIRVCross_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(SPIRVCross_core_LIBRARY
            NAMES spirv-cross-cored libspirv-cross-cored
            HINTS ${SPIRVCross_ROOT} ENV SPIRVCross_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(SPIRVCross_reflect_LIBRARY
            NAMES spirv-cross-reflectd libspirv-cross-reflectd
            HINTS ${SPIRVCross_ROOT} ENV SPIRVCross_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(SPIRVCross_glsl_LIBRARY
            NAMES spirv-cross-glsld libspirv-cross-glsld
            HINTS ${SPIRVCross_ROOT} ENV SPIRVCross_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(SPIRVCross_hlsl_LIBRARY
            NAMES spirv-cross-hlsld libspirv-cross-hlsld
            HINTS ${SPIRVCross_ROOT} ENV SPIRVCross_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(SPIRVCross_msl_LIBRARY
            NAMES spirv-cross-msld libspirv-cross-msld
            HINTS ${SPIRVCross_ROOT} ENV SPIRVCross_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )
    else()
        find_library(SPIRVCross_c_LIBRARY
            NAMES spirv-cross-c libspirv-cross-c spirv-cross-c-shared libspirv-cross-c-shared
            HINTS ${SPIRVCross_ROOT} ENV SPIRVCross_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(SPIRVCross_core_LIBRARY
            NAMES spirv-cross-core libspirv-cross-core
            HINTS ${SPIRVCross_ROOT} ENV SPIRVCross_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(SPIRVCross_reflect_LIBRARY
            NAMES spirv-cross-reflect libspirv-cross-reflect
            HINTS ${SPIRVCross_ROOT} ENV SPIRVCross_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(SPIRVCross_glsl_LIBRARY
            NAMES spirv-cross-glsl libspirv-cross-glsl
            HINTS ${SPIRVCross_ROOT} ENV SPIRVCross_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(SPIRVCross_hlsl_LIBRARY
            NAMES spirv-cross-hlsl libspirv-cross-hlsl
            HINTS ${SPIRVCross_ROOT} ENV SPIRVCross_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(SPIRVCross_msl_LIBRARY
            NAMES spirv-cross-msl libspirv-cross-msl
            HINTS ${SPIRVCross_ROOT} ENV SPIRVCross_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )
    endif()
endif()

# If we found the main library, mark as found
if (SPIRVCross_c_LIBRARY OR SPIRVCross_cpp_LIBRARY AND SPIRVCross_INCLUDE_DIR)
    set(SPIRVCross_FOUND TRUE)
endif()

# Set the include directory and libraries
if (SPIRVCross_FOUND)
    # If we already found the libraries through CONFIG mode, don't override them
    if (NOT SPIRVCross_LIBRARIES)
        set(SPIRVCross_INCLUDE_DIRS ${SPIRVCross_INCLUDE_DIR})
        set(SPIRVCross_LIBRARIES 
            ${SPIRVCross_c_LIBRARY}
            ${SPIRVCross_cpp_LIBRARY}
            ${SPIRVCross_core_LIBRARY}
            ${SPIRVCross_reflect_LIBRARY}
            ${SPIRVCross_glsl_LIBRARY}
            ${SPIRVCross_hlsl_LIBRARY}
            ${SPIRVCross_msl_LIBRARY}
        )
    endif()
else()
    # If we didn't find anything, set the variables from standard search
    set(SPIRVCross_INCLUDE_DIRS ${SPIRVCross_INCLUDE_DIR})
    set(SPIRVCross_LIBRARIES 
        ${SPIRVCross_c_LIBRARY}
        ${SPIRVCross_cpp_LIBRARY}
        ${SPIRVCross_core_LIBRARY}
        ${SPIRVCross_reflect_LIBRARY}
        ${SPIRVCross_glsl_LIBRARY}
        ${SPIRVCross_hlsl_LIBRARY}
        ${SPIRVCross_msl_LIBRARY}
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SPIRVCross 
    REQUIRED_VARS SPIRVCross_INCLUDE_DIR SPIRVCross_LIBRARIES
)

mark_as_advanced(
    SPIRVCross_INCLUDE_DIR
    SPIRVCross_LIBRARIES
)