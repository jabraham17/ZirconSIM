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


build_gnu() {
    SOURCE=$1
    BUILD=$2
    INSTALL=$3
    TARGET=$4
    PARALLEL=`nproc`

    mkdir -p $BUILD
    mkdir -p $INSTALL

    NEWPATH=$INSTALL/bin:$PATH
    (set -x && cd $BUILD && PATH=$NEWPATH $SOURCE/configure --prefix=$INSTALL \
        --srcdir=$SOURCE \
        --with-arch=rv64ima \
        --with-abi=lp64 \
        --with-cmodel=medany \
        && PATH=$NEWPATH make $TARGET -j $PARALLEL)

}

SOURCE=$1
if [ -z $SOURCE ]; then
    echo "No Source Direcrty Given"
    echo "Correct Usage: $SCRIPT_NAME <gnu-source>"
    exit 1
fi

build_gnu $SOURCE $SCRIPT_DIR/"build-toolchain/rv64ima" $SCRIPT_DIR/"toolchains/rv64ima"
build_gnu $SOURCE $SCRIPT_DIR/"build-toolchain/rv64ima-musl" $SCRIPT_DIR/"toolchains/rv64ima-musl" musl
build_gnu $SOURCE $SCRIPT_DIR/"build-toolchain/rv64ima-linux" $SCRIPT_DIR/"toolchains/rv64ima-linux" linux




