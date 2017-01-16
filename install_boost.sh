#!/bin/bash
set -ex
wget https://sourceforge.net/projects/boost/files/boost/1.62.0/boost_1_62_0.tar.bz2/download
mv download boost_1_62_0.tar.bz2
tar xf boost_1_62_0.tar.bz2
cd boost_1_62_0/
./bootstrap.sh --with-libraries=test,filesystem
if [ "${LIBCXX}" == "1" ]; then
    FLAGS="-stdlib=libc++"
    sudo ./b2 install toolset=${CXX_COMPILER} cxxflags=${FLAGS} linkflags=${FLAGS} -j 2
else
    sudo ./b2 install toolset=${CXX_COMPILER} -j 2
fi
