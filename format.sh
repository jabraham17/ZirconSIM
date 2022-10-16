#!/usr/bin/env bash

CF=clang-format
CF=../llvm/llvm-project/build/bin/clang-format

FILES=("src/*.c" "src/*.cpp" "src/*.cc" "src/*.h" "src/*.inc")
set -e

for s in "${FILES[@]}"; do
    (set -x && find . -wholename $s -type f -exec $CF -i {} +)
done
