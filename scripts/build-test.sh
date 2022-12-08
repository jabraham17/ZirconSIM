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
source $SCRIPT_DIR/env.sh


TOOLCHAIN=$1
if [ -z $TOOLCHAIN ]; then
    echo "Need to specify a toolchain"
    exit 1
fi
shift
MAIN=$1
if [ -z $MAIN ]; then
    echo "Need at least 1 argument, likely a source file"
    exit 1
fi
OUTNAME=$ZIRCON_TEST_BUILD/$(basename $MAIN | sed 's/\(.*\)\..*/\1/')
mkdir -p $ZIRCON_TEST_BUILD
if [[ $TOOLCHAIN == "native" ]]; then
    PREFIX=
    OUTNAME+="-native.out"
elif [[ $TOOLCHAIN == "elf" ]]; then
    PREFIX=$RV64IMA_ELF/bin/riscv64-unknown-elf-
    OUTNAME+="-elf.out"
elif [[ $TOOLCHAIN == "elf-debug" ]]; then
    PREFIX=$RV64IMA_ELF_DEBUG/bin/riscv64-unknown-elf-
    OUTNAME+="-elf-debug.out"
elif [[ $TOOLCHAIN == "linux" ]]; then
    PREFIX=$RV64IMA_LINUX/bin/riscv64-unknown-linux-gnu-
    OUTNAME+="-linux.out"
elif [[ $TOOLCHAIN == "linux-debug" ]]; then
    PREFIX=$RV64IMA_LINUX_DEBUG/bin/riscv64-unknown-linux-gnu-
    OUTNAME+="-linux-debug.out"
elif [[ $TOOLCHAIN == "musl" ]]; then
    PREFIX=$RV64IMA_MUSL/bin/riscv64-unknown-linux-musl-
    OUTNAME+="-musl.out"
elif [[ $TOOLCHAIN == "musl-debug" ]]; then
    PREFIX=$RV64IMA_MUSL_DEBUG/bin/riscv64-unknown-linux-musl-
    OUTNAME+="-musl-debug.out"
else
    echo "Invalid Toolchain"
    exit 1
fi

(set -x && ${PREFIX}gcc -o $OUTNAME -g -static -Wall -Wextra $@)
