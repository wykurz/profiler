#!/bin/sh
set -ex
if [ -d benchmark/build ]; then
  cd llvm
  sudo make install-cxxabi install-cxx
else
  git clone --depth=1 https://github.com/llvm-mirror/llvm.git llvm-source
  git clone --depth=1 https://github.com/llvm-mirror/libcxx.git llvm-source/projects/libcxx
  git clone --depth=1 https://github.com/llvm-mirror/libcxxabi.git llvm-source/projects/libcxxabi
  mkdir llvm
  cd llvm
  cmake -DCMAKE_C_COMPILER=clang \
        -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_BUILD_TYPE=Release \
        ../llvm-source
  make cxx -j2
  sudo make install-cxxabi install-cxx
fi
