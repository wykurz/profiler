#!/bin/sh
set -ex
if [ -n "$COVERAGE" ]; then
    wget http://ftp.de.debian.org/debian/pool/main/l/lcov/lcov_1.11.orig.tar.gz
    tar xf lcov_1.11.orig.tar.gz
    sudo make -C lcov-1.11/ install
    gem install coveralls-lcov
    lcov --directory . --zerocounters
fi
