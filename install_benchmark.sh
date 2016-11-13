#!/bin/sh
set -ex
if [ -d benchmark ]; then
    cd benchmark
    sudo make install
else
    git clone https://github.com/google/benchmark.git
    cd benchmark
    cmake -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_USE_LIBCXX=ON -DCMAKE_CXX_FLAGS="-I/usr/include/c++/v1" -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang
    make
    sudo make install
fi
