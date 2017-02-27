#!/bin/sh
clang-format -style=llvm $(find . -name *.cpp) $(find . -name *.h) -output-replacements-xml|grep -c "<replacement " >/dev/null
if [ $? -ne 1 ]; then
    echo "Commit did not match clang-format"
    exit 1;
fi
exit 0
