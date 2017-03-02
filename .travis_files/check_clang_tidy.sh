#!/bin/sh
clang-tidy -p=build/debug -config='' $(find Profiler tests examples -name *.cpp)
