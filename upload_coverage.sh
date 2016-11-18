#!/bin/sh
set -ex
if [ -n "$COVERAGE" ]; then
    lcov --gcov-tool $GCOV --directory . --capture --output-file coverage.info
    lcov --gcov-tool $GCOV --remove coverage.info 'tests/*' '/usr/*'--output-file coverage.info
    lcov --gcov-tool $GCOV --list coverage.info
    coveralls-lcov coverage.info
fi
