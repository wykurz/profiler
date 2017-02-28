#!/bin/sh
H_FILES=$(find Profiler tests examples -name *.h)
CPP_FILES=$(find Profiler tests examples -name *.cpp)
clang-tidy -p=build/debug -config='' ${H_FILES} ${CPP_FILES}
