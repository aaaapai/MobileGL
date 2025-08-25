# FindSPIRVTools.cmake
# --------------------
#
# Find SPIRV-Tools library
#
# This module defines the following variables:
#   SPIRVTools_FOUND - True if SPIRV-Tools library is found
#   SPIRVTools_INCLUDE_DIRS - Include directories for SPIRV-Tools
#   SPIRVTools_LIBRARIES - Libraries for SPIRV-Tools
#
# This module reads hints about search locations from:
#   SPIRVTools_ROOT - Preferred installation prefix

# First try to find SPIRV-Tools using CONFIG mode (for vcpkg, conan, etc.)
if (NOT SPIRVTools_FOUND)
    find_package(SPIRV-Tools CONFIG QUIET)
    if (SPIRVTools_FOUND)
        message(STATUS "Found SPIRV-Tools via CONFIG mode: ${SPIRV-Tools_DIR}")
        set(SPIRVTools_INCLUDE_DIRS ${SPIRV-Tools_INCLUDE_DIRS})
        set(SPIRVTools_LIBRARIES ${SPIRV-Tools_LIBRARIES})
    endif()
endif()

# If CONFIG mode failed, try to find Vulkan SDK
if (NOT SPIRVTools_FOUND)
    find_package(Vulkan QUIET)
    if (Vulkan_FOUND)
        get_filename_component(VULKAN_SDK_DIR ${Vulkan_INCLUDE_DIRS} DIRECTORY)
        message(STATUS "Found Vulkan SDK: ${VULKAN_SDK_DIR}")
        
        # Try to find headers in Vulkan SDK
        find_path(SPIRVTools_INCLUDE_DIR 
            NAMES spirv-tools/libspirv.h
            HINTS ${VULKAN_SDK_DIR}
            PATH_SUFFIXES include
            NO_DEFAULT_PATH
        )
        
        # Try to find libraries in Vulkan SDK (separate debug/release searches)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            find_library(SPIRVTools_LIBRARY
                NAMES SPIRV-Toolsd libSPIRV-Toolsd
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(SPIRVTools_opt_LIBRARY
                NAMES SPIRV-Tools-optd libSPIRV-Tools-optd
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(SPIRVTools_link_LIBRARY
                NAMES SPIRV-Tools-linkd libSPIRV-Tools-linkd
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
        else()
            find_library(SPIRVTools_LIBRARY
                NAMES SPIRV-Tools libSPIRV-Tools
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(SPIRVTools_opt_LIBRARY
                NAMES SPIRV-Tools-opt libSPIRV-Tools-opt
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
            
            find_library(SPIRVTools_link_LIBRARY
                NAMES SPIRV-Tools-link libSPIRV-Tools-link
                HINTS ${VULKAN_SDK_DIR}
                PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
                NO_DEFAULT_PATH
            )
        endif()
    endif()
endif()

# If Vulkan SDK search failed, try standard search
if (NOT SPIRVTools_FOUND)
    # Look for the header file
    find_path(SPIRVTools_INCLUDE_DIR 
        NAMES spirv-tools/libspirv.h
        HINTS ${SPIRVTools_ROOT} ENV SPIRVTools_ROOT
        PATH_SUFFIXES include
    )

    # Look for the libraries (separate debug/release searches)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        find_library(SPIRVTools_LIBRARY
            NAMES SPIRV-Toolsd libSPIRV-Toolsd
            HINTS ${SPIRVTools_ROOT} ENV SPIRVTools_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(SPIRVTools_opt_LIBRARY
            NAMES SPIRV-Tools-optd libSPIRV-Tools-optd
            HINTS ${SPIRVTools_ROOT} ENV SPIRVTools_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(SPIRVTools_link_LIBRARY
            NAMES SPIRV-Tools-linkd libSPIRV-Tools-linkd
            HINTS ${SPIRVTools_ROOT} ENV SPIRVTools_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )
    else()
        find_library(SPIRVTools_LIBRARY
            NAMES SPIRV-Tools libSPIRV-Tools
            HINTS ${SPIRVTools_ROOT} ENV SPIRVTools_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(SPIRVTools_opt_LIBRARY
            NAMES SPIRV-Tools-opt libSPIRV-Tools-opt
            HINTS ${SPIRVTools_ROOT} ENV SPIRVTools_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )

        find_library(SPIRVTools_link_LIBRARY
            NAMES SPIRV-Tools-link libSPIRV-Tools-link
            HINTS ${SPIRVTools_ROOT} ENV SPIRVTools_ROOT
            PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64
        )
    endif()
endif()

# Set the include directory and libraries
set(SPIRVTools_INCLUDE_DIRS ${SPIRVTools_INCLUDE_DIR})
set(SPIRVTools_LIBRARIES 
    ${SPIRVTools_LIBRARY}
    ${SPIRVTools_opt_LIBRARY}
    ${SPIRVTools_link_LIBRARY}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SPIRVTools 
    REQUIRED_VARS SPIRVTools_INCLUDE_DIR SPIRVTools_LIBRARY
)

mark_as_advanced(
    SPIRVTools_INCLUDE_DIR
    SPIRVTools_LIBRARY
    SPIRVTools_opt_LIBRARY
    SPIRVTools_link_LIBRARY
)