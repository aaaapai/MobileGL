cmake_minimum_required(VERSION 3.14)
project(MobileGLTest)

message(STATUS "Generating build files for MobileGL Test...")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MGL_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

# # Add the directory containing our custom Find modules
# list(APPEND CMAKE_MODULE_PATH "${MGL_ROOT}/buildsystem/cmake")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# # Find the required libraries using our custom Find modules
# find_package(glslang REQUIRED)
# find_package(SPIRVCross REQUIRED)
# find_package(SPIRVTools)

add_executable(
  SanityTest
  SanityTest.cpp
)
target_link_libraries(
  SanityTest
  GTest::gtest_main
)

# message(STATUS "glslang_LIBRARIES: ${glslang_LIBRARIES}")
# message(STATUS "SPIRVTools_LIBRARIES: ${SPIRVTools_LIBRARIES}")
# message(STATUS "SPIRVCross_LIBRARIES: ${SPIRVCross_LIBRARIES}")

# # Set the libraries to link against
# set(LINK_LIBRARIES 
#     ${glslang_LIBRARIES}
#     ${SPIRVCross_LIBRARIES}
# )

# if (SPIRVTools_FOUND)
#   list(APPEND LINK_LIBRARIES ${SPIRVTools_LIBRARIES})
# endif()

set(LINK_LIBRARIES 
    MobileGL_s
)

include(GoogleTest)
gtest_discover_tests(SanityTest)

add_subdirectory(Buffer)
add_subdirectory(VertexArray)
add_subdirectory(Program)
