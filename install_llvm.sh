#!/bin/sh
set -ex
if [ -d llvm/build ]; then
  cd llvm/build
  sudo make install-cxxabi install-cxx
else
  git clone --depth=1 https://github.com/llvm-mirror/llvm.git llvm
  git clone --depth=1 https://github.com/llvm-mirror/libcxx.git llvm/projects/libcxx
  git clone --depth=1 https://github.com/llvm-mirror/libcxxabi.git llvm/projects/libcxxabi
  mkdir llvm/build
  cd llvm/build
  cmake -DCMAKE_C_COMPILER=${CC} \
        -DCMAKE_CXX_COMPILER=${CXX} \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_BUILD_TYPE=Release \
        ..
  make cxx -j2
  sudo make install-cxxabi install-cxx
fi
