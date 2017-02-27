#!/bin/sh
H_FILES=$(find Profiler tests examples -name *.h)
CPP_FILES=$(find Profiler tests examples -name *.cpp)
clang-format-3.8 -style=llvm ${H_FILES} ${CPP_FILES} -output-replacements-xml|grep -c "<replacement " >/dev/null
if [ $? -ne 1 ]; then
    echo "Commit did not match clang-format"
    exit 1;
fi
exit 0
