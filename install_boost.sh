#!/bin/bash
set -ex
wget https://sourceforge.net/projects/boost/files/boost/1.62.0/boost_1_62_0.tar.bz2/download
mv download boost_1_62_0.tar.bz2
tar xf boost_1_62_0.tar.bz2
cd boost_1_62_0/
if [[ "${CXX_COMPILER}" == *"clang"* ]]; then
    TOOLSET="clang"
else
    TOOLSET="gcc"
fi
./bootstrap.sh --with-libraries=test,filesystem --with-toolset=${TOOLSET}
./b2 clean
if [ "${LIBCXX}" == "1" ]; then
    FLAGS="-stdlib=libc++"
    ./b2 toolset=${TOOLSET} cxxflags=${FLAGS} linkflags=${FLAGS} -d1 -j 2 stage
    sudo ./b2 install
else
    ./b2 toolset=${TOOLSET} -d1 -j 2 stage
    sudo ./b2 install
fi
