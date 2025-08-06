cmake_minimum_required(VERSION 3.14)
project(MobileGLTest)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.4
)
FetchContent_MakeAvailable(benchmark)

set(MGL_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

add_executable(SanityBench
  SanityBench.cpp
)
target_link_libraries(SanityBench 
  PRIVATE benchmark::benchmark
)

enable_testing()

add_test(NAME SanityBench COMMAND SanityBench)