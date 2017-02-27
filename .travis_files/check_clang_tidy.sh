#!/bin/sh
truncate --size 0 tidylog && for f in `find . -name *.cpp`; do clang-tidy -p=build/debug -header-filter=Profiler -config='' -fix -fix-errors $f >> tidylog; done
