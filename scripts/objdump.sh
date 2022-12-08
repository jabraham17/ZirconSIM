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

OBJDUMP=
if [ -f "$RV64IMA_ELF/bin/riscv64-unknown-elf-objdump" ]; then
    OBJDUMP=$RV64IMA_ELF/bin/riscv64-unknown-elf-objdump
elif [ -f "$RV64IMA_LINUX/bin/riscv64-unknown-linux-objdump" ]; then
    OBJDUMP=$RV64IMA_LINUX/bin/riscv64-unknown-linux-objdump
elif [ -f "$RV64IMA_MUSL/bin/riscv64-unknown-linux-musl-objdump" ]; then
    OBJDUMP=$RV64IMA_MUSl/bin/riscv64-unknown-linux-musl-objdump
else
    echo "No built objdump"
    exit 1
fi

$OBJDUMP -d --disassembler-color=color --no-show-raw-insn -Mno-aliases $@
