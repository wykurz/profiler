#!/bin/sh
set -ex
git clone https://github.com/google/benchmark.git
cd benchmark
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-I/usr/include/c++/v1" \
      -DCMAKE_CXX_COMPILER=${CXX} \
      -DCMAKE_C_COMPILER=${CC} \
      -DCMAKE_CXX_FLAGS="${BENCHMARK_FLAGS}" \
      ..
make
sudo make install
