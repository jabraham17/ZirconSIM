#!/usr/bin/env bash

set -e
realpath() {
  OURPWD=$PWD
  cd "$(dirname "$1")"
  LINK=$(readlink "$(basename "$1")")
  while [ "$LINK" ]; do
    cd "$(dirname "$LINK")"
    LINK=$(readlink "$(basename "$1")")
  done
  REALPATH="$PWD/$(basename "$1")"
  cd "$OURPWD"
  echo "$REALPATH"
}
SCRIPT_NAME=
if [ -z $BASH_SOURCE ]; then
    SCRIPT_NAME=$0
else
    SCRIPT_NAME=${BASH_SOURCE[0]}
fi
SCRIPT_DIR=$(realpath "$(dirname "$SCRIPT_NAME")")
source $SCRIPT_DIR/env_vars.sh

# check for elf toolchain
if ! [ -f "$RV64IMA_ELF/bin/riscv64-unknown-elf-gcc" ]; then
    echo "No ELF toolchain"
    exit 1
fi

(set -x && \
    cd $SCRIPT_DIR/hint && \
    make CC="clang" CFLAGS="--target=riscv64-unknown-elf -march=rv64ima -mabi=lp64 --gcc-toolchain=$RV64IMA_ELF --sysroot=$RV64IMA_ELF/riscv64-unknown-elf" LDFLAGS="-static" -B all \
)
(set -x && \
    cd $SCRIPT_DIR/hint && \
    make BIN_DIRECTORY=native CC=clang CFLAGS="--target=x86_64" LDFLAGS="-static" -B all \
)


