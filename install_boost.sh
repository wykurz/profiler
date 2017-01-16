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
if [ "${LIBCXX}" == "1" ]; then
    FLAGS="-stdlib=libc++"
    sudo PATH=${PATH} ./b2 install toolset=${TOOLSET} cxxflags=${FLAGS} linkflags=${FLAGS} -j 2
else
    sudo PATH=${PATH} ./b2 install toolset=${TOOLSET} -j 2
fi
