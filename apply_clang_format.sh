#!/bin/sh
clang-format -style=llvm -i $(find . -name *.cpp) $(find . -name *.h)
