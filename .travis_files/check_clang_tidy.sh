#!/bin/sh
clang-tidy-3.8 -p=build/debug -config='' $(find Profiler tests examples -name *.cpp)
