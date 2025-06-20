#!/bin/bash
set -e

<<EOF
if ! command -v ccache &> /dev/null; then
      echo "安装 ccache..."
      sudo apt-get update && sudo apt-get install -y ccache
fi 
EOF

<<EOF
export CMAKE_C_COMPILER_LAUNCHER=ccache
export CMAKE_CXX_COMPILER_LAUNCHER=ccache

export CCACHE_DIR="$GITHUB_WORKSPACE/ccache"
export CCACHE_BASEDIR="$GITHUB_WORKSPACE"
export CCACHE_COMPRESS=1
export CCACHE_COMPRESSLEVEL=6
EOF

cmake_build () {
  ANDROID_ABI=$1
  ANDROID_TARGET=$2
  mkdir -p build
  cd build
  cmake $GITHUB_WORKSPACE -DCMAKE_INSTALL_PREFIX=$GITHUB_WORKSPACE/build -DANDROID_PLATFORM=26 -DCMAKE_BUILD_TYPE=Release -DCMAKE_AR=$ANDROID_NDK_LATEST_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar -DCMAKE_RANLIB=$ANDROID_NDK_LATEST_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ranlib -DANDROID_ABI=$ANDROID_ABI -DCMAKE_SYSTEM_NAME=Android -DANDROID_TOOLCHAIN=clang -DANDROID_ARM_MODE=arm -DCMAKE_MAKE_PROGRAM=$ANDROID_NDK_LATEST_HOME/prebuilt/linux-x86_64/bin/make -DCMAKE_SYSTEM_NAME=Android -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_LATEST_HOME/build/cmake/android.toolchain.cmake
  cmake --build . --config Release --parallel $(nproc)
  $ANDROID_NDK_LATEST_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip $GITHUB_WORKSPACE/build/libMobileGL.so
}

cmake_build arm64-v8a aarch64-linux-android
