#!/usr/bin/env bash

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

BG_BUILD=$SCRIPT_DIR/"rv64ima-linux-toolchain-build"
mkdir -p $BG_BUILD
BG_SOURCE=$1
BG_INSTALL="$SCRIPT_DIR/rv64ima-linux-toolchain"
mkdir -p $BG_INSTALL
BG_PARALLEL=`nproc`

set -e
set -x

(cd $BG_BUILD && $BG_SOURCE/configure --prefix=$BG_INSTALL \
  --srcdir=$BG_SOURCE \
  --with-arch=rv64ima \
  --with-abi=lp64 \
  --with-cmodel=medany)
(cd $BG_BUILD && make linux -j $BG_PARALLEL && make install)

