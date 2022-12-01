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

RESULTS_DIR=$SCRIPT_DIR/results
mkdir -p $RESULTS_DIR

ZIRCON=$SCRIPT_DIR/../build/bin/zircon
SPIKE=$RV64IMA_ELF/bin/riscv64-unknown-elf-run

# check for zircon toolchain
if ! [ -f $ZIRCON ]; then
    echo "No Zircon binary found"
    exit 1
fi
if ! [ -f $SPIKE ]; then
    echo "No Spike binary found"
    exit 1
fi


# HINT
mkdir -p $RESULTS_DIR/hint
mkdir -p $RESULTS_DIR/hint/zircon
mkdir -p $RESULTS_DIR/hint/spike

(set -x && cd $RESULTS_DIR/hint/zircon && \
    $ZIRCON $SCRIPT_DIR/hint/bin/DOUBLE DOUBLE > d1_out)
(set -x && cd $RESULTS_DIR/hint/zircon && \
    $ZIRCON $SCRIPT_DIR/hint/bin/INT INT > i1_out)
(set -x && cd $RESULTS_DIR/hint/zircon && \
    $ZIRCON $SCRIPT_DIR/hint/bin/SHORT SHORT > s1_out)
(set -x && cd $RESULTS_DIR/hint/zircon && \
    $ZIRCON $SCRIPT_DIR/hint/bin/FLOAT FLOAT > f1_out)
(set -x && cd $RESULTS_DIR/hint/zircon && \
    $ZIRCON $SCRIPT_DIR/hint/bin/LONGLONG LONGLONG > l1_out)

(set -x && cd $RESULTS_DIR/hint/spike && \
    $SPIKE $SCRIPT_DIR/hint/bin/DOUBLE DOUBLE > d1_out)
(set -x && cd $RESULTS_DIR/hint/spike && \
    $SPIKE $SCRIPT_DIR/hint/bin/INT INT > i1_out)
(set -x && cd $RESULTS_DIR/hint/spike && \
    $SPIKE $SCRIPT_DIR/hint/bin/SHORT SHORT > s1_out)
(set -x && cd $RESULTS_DIR/hint/spike && \
    $SPIKE $SCRIPT_DIR/hint/bin/FLOAT FLOAT > f1_out)
(set -x && cd $RESULTS_DIR/hint/spike && \
    $SPIKE $SCRIPT_DIR/hint/bin/LONGLONG LONGLONG > l1_out)
