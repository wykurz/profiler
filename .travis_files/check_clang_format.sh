#!/bin/sh
FILES=$(find Profiler tests examples -name *.cpp) $(find Profiler tests examples -name *.h)
clang-format-3.8 -style=llvm ${FILES} -output-replacements-xml|grep -c "<replacement " >/dev/null
if [ $? -ne 1 ]; then
    echo "Commit did not match clang-format"
    exit 1;
fi
exit 0
