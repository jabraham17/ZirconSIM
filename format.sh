#!/usr/bin/env bash

CF=clang-format
CF=../llvm/llvm-project/build/bin/clang-format

find . -name "*.c" -type f -exec $CF -i {} +
find . -name "*.cpp" -type f -exec $CF -i {} +
find . -name "*.cc" -type f -exec $CF -i {} +
find . -name "*.h" -type f -exec $CF -i {} +
