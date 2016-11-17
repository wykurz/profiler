#!/bin/sh
set -ex
if [ -n "$COVERAGE" ]; then
    lcov --directory . --capture --output-file coverage.info
    lcov --remove coverage.info 'tests/*' --output-file coverage.info
    lcov --list coverage.info
    coveralls-lcov coverage.info
fi
