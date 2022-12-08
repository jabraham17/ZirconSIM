#!/usr/bin/env bash

set -e

ELF_TOOLCHAIN=$1
if [ -z $ELF_TOOLCHAIN ]; then
    echo "Need to specify the elf toolchain root"
    exit 1
fi
shift
BUILD=$1
if [ -z $BUILD ]; then
    echo "Need to specify the root build directory"
    exit 1
fi
shift

BUILD=$BUILD/hint

(set -x && \
    make \
    BIN_DIRECTORY=$BUILD \
    TARGET_PREFIX="rv64ima-" \
    CC="clang" \
    CFLAGS="--target=riscv64-unknown-elf -march=rv64ima -mabi=lp64 --gcc-toolchain=$ELF_TOOLCHAIN --sysroot=$ELF_TOOLCHAIN/riscv64-unknown-elf" \
    LDFLAGS="-static" \
    -B \
    all \
)
(set -x && \
    make \
    BIN_DIRECTORY=$BUILD \
    TARGET_PREFIX="native-" \
    CC=clang \
    CFLAGS="--target=x86_64" \
    LDFLAGS="-static" \
    -B \
    all \
)
