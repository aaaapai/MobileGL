cmake_minimum_required(VERSION 3.22.1)

project("MobileGL")

enable_language(C)
enable_language(CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ANDROID_LD "lld")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(MOBILEGL_FORCE_RELEASE_OPT "Enable Release optimization flags in Debug build" ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -pipe -integrated-as -mllvm -polly -mllvm -polly-vectorizer=stripmine -mllvm -polly-invariant-load-hoisting -mllvm -polly-run-inliner -mllvm -polly-run-dce -mllvm -polly-invariant-load-hoisting -flto -mllvm -polly-run-inliner -mllvm -polly-run-dce -fno-emulated-tls -fwhole-program-vtables -fdata-sections -ffunction-sections -fmerge-all-constants -mllvm -polly-detect-keep-going -mllvm -polly-ast-use-context -march=armv8-a+simd -fvisibility=hidden -Wall -Wextra -ferror-limit=0 -Wno-return-type -Wno-unused-parameter -Wno-c++98-compat -Wno-c++98-compat-pedantic -DGLM_FORCE_INTRINSICS -Wno-unicode-whitespace -Wno-format -Wno-unused-function -DUseFastSTL -mno-omit-leaf-frame-pointer -fsigned-char -Wno-psabi")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pipe -integrated-as -mllvm -polly -mllvm -polly-vectorizer=stripmine -mllvm -polly-invariant-load-hoisting -mllvm -polly-run-inliner -mllvm -polly-run-dce -mllvm -polly-invariant-load-hoisting -flto -mllvm -polly-run-inliner -mllvm -polly-run-dce -fno-emulated-tls -fwhole-program-vtables -fdata-sections -ffunction-sections -fmerge-all-constants -mllvm -polly-detect-keep-going -mllvm -polly-ast-use-context -march=armv8-a+simd -fvisibility=hidden -Wall -Wextra -ferror-limit=0 -Wno-return-type -Wno-unused-parameter -Wno-c++98-compat -Wno-c++98-compat-pedantic -DGLM_FORCE_INTRINSICS -stdlib=libc++ -Wno-unicode-whitespace -Wno-format -Wno-unused-function -Wno-vla-cxx-extension -DUseFastSTL -frtti -mno-omit-leaf-frame-pointer -fsigned-char -Wno-psabi")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -flto -O3 -Wl,-plugin-opt=-emulated-tls=0 -Wl,--strip-all -fvisibility=hidden -Wl,-O3 -Wl,--gc-sections -Wl,--as-needed -static-libstdc++")
# -DUseFastSTL -DUseStandard -DUseAnkerl

set(ENABLE_GLSLANG_BINARIES    OFF CACHE BOOL "Enable glslangValidator/spirv-remap" FORCE)
set(ENABLE_SPVREMAPPER         OFF CACHE BOOL "Enable SPVRemapper" FORCE)
set(ENABLE_OPT                 ON  CACHE BOOL "Enable SPIRV-Tools opt usage in glslang" FORCE)
set(BUILD_EXTERNAL             ON  CACHE BOOL "Build external deps in External/" FORCE)
set(ENABLE_GLSLANG_INSTALL     OFF CACHE BOOL "Install glslang targets" FORCE)

add_subdirectory(3rdparty/glslang)

if (MSVC)
    set(CMAKE_CXX_FLAGS "/EHsc ${CMAKE_CXX_FLAGS}")
endif()

# Generate MGGitHash.h
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH_FULL
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(SUBSTRING "${GIT_COMMIT_HASH_FULL}" 0 7 GIT_COMMIT_HASH_SHORT)

configure_file(
    ${CMAKE_SOURCE_DIR}/MobileGL/MG_Util/Miscellany/MGGitHash.h.in
    ${CMAKE_BINARY_DIR}/generated/MGGitHash.h
    @ONLY
)

include_directories(${CMAKE_BINARY_DIR}/generated)

set(SOURCE_FILES
    MobileGL/Init.cpp
)

add_library(${CMAKE_PROJECT_NAME} SHARED
    ${SOURCE_FILES}
)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/MobileGL
)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug" OR MOBILEGL_FORCE_RELEASE_OPT)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT LTOSupported OUTPUT LTOError)
    if (LTOSupported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()

    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT MATCHES "AppleClang")
        add_compile_options(-O3 -ffunction-sections -fdata-sections)
        add_link_options(-Wl,--gc-sections)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        add_compile_options(/O2 /GL)
    else ()
        add_compile_options(-O3)
    endif()
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        C_VISIBILITY_PRESET default
        CXX_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
    )
else()
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()

if (ANDROID)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libspirv-cross-c.a
    ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libglslang.a
    ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libspirv-cross-core.a
    ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libspirv-cross-cpp.a
    ${CMAKE_SOURCE_DIR}/library/android/${ANDROID_ABI}/libspirv-cross-glsl.a
    android
    log
)

# 版本脚本针对特定目标设置
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE 
    "LINKER:--version-script=${CMAKE_SOURCE_DIR}/MobileGL/mgl-version.script"
)

endif()
