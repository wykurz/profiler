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
    LIBCXX="-stdlib=libc++"
    ./b2 install toolset=${TOOLSET} cxxflags="-std=gnu++14 ${LIBCXX}" linkflags=${LIBCXX} --prefix=${BOOST_DIR} -d1 -j 2
else
    ./b2 install toolset=${TOOLSET} --prefix=${BOOST_DIR} -d1 -j 2
fi
